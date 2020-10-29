#pragma once
#include <psl/exceptions.hpp>
#include <psl/type_concepts.hpp>
#include <psl/details/optional_value_storage.hpp>

namespace psl
{
	template <typename T>
	class optional;

	inline namespace details
	{
		template <typename T>
		struct is_optional : std::false_type
		{};

		template <typename Arg>
		struct is_optional<optional<Arg>> : std::true_type
		{};

		template <typename T>
		inline constexpr bool is_optional_v = is_optional<T>::value;
	} // namespace details

	template <typename T>
	concept IsOptional = is_optional_v<T>;

	inline namespace details
	{
		template <typename FN, typename Opt, typename... Args>
		struct is_opt_invocable
			: std::conditional_t<std::is_invocable_v<FN, Opt, Args...> || std::is_invocable_v<FN, Opt&, Args...>,
								 std::true_type, std::false_type>
		{};

		template <typename FN, typename Opt, typename... Args>
		inline constexpr auto is_opt_invocable_v = is_opt_invocable<FN, Opt, Args...>::value;

		template <typename FN, typename Opt, typename... Args>
		struct is_opt_invocable_r
			: std::conditional_t<std::is_invocable_v<FN, Opt, Args...>, std::invoke_result<FN, Opt, Args...>,
								 std::invoke_result<FN, Opt&, Args...>>
		{};


		template <typename FN, typename Opt, typename... Args>
		using is_opt_invocable_r_t = typename is_opt_invocable_r<FN, Opt, Args...>::type;
	} // namespace details

	/**
	 * \brief wrapper type to optionally hold an object with continuation (`then`) functionality.
	 * \details Unlike `psl::expected`, when optionals are passed in continuation contexts, them being missing is not an
	 * error state, but rather a possible expected state. This means that optionals when the continuation doesn't expect
	 * an optional, it will have to depend on the fallback value (if provided), or the default value (if possible).
	 * Unlike `psl::expected`, optionals can be re-assigned after construction.
	 *
	 * \tparam T type to contain.
	 */
	template <typename T>
	class optional
	{
	  public:
		using bad_optional_access = bad_access<optional>;

		static_assert(std::is_same_v<T, std::remove_pointer_t<std::remove_cvref_t<T>>>,
					  "no support for reference or pointer types");
		static_assert(MoveCopyAssignable<T>, "contained type has to be either movable, or copyable");

		constexpr optional() noexcept = default;
		explicit constexpr optional(nullopt_t) noexcept : m_ValueStorage(nullopt) {}

		explicit constexpr optional(value_init_t) noexcept(std::is_nothrow_constructible_v<T>) requires(
			std::is_constructible_v<T>)
			: m_ValueStorage({})
		{}
		explicit constexpr optional(default_init_t) noexcept(std::is_nothrow_default_constructible_v<T>) requires(
			std::is_default_constructible_v<T>)
			: m_ValueStorage()
		{}
		explicit constexpr optional(zero_init_t) noexcept(std::is_nothrow_constructible_v<T, zero_init_t>) requires(
			std::is_constructible_v<T, zero_init_t>)
			: m_ValueStorage(zero_init)
		{}

		template <typename... Args>
		requires(disable_perfect_forward_illegal_type_v<optional, Args...>) constexpr optional(Args&&... args) noexcept(
			std::is_nothrow_constructible_v<T, Args...>)
			: m_ValueStorage(std::forward<Args>(args)...)
		{}

		constexpr optional(const optional& other) noexcept(std::is_nothrow_copy_constructible_v<T>) requires(
			std::is_copy_constructible_v<T>)
			: m_ValueStorage(other.m_ValueStorage)
		{}
		constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>) requires(
			std::is_move_constructible_v<T>)
			: m_ValueStorage(std::move(other.m_ValueStorage))
		{}

		constexpr optional& operator=(const optional& other) noexcept(std::is_nothrow_copy_assignable_v<T>) requires(
			std::is_copy_assignable_v<T>)
		{
			if(this != &other) [[likely]]
				{
					m_ValueStorage = other.m_ValueStorage;
				}
			return *this;
		}
		constexpr optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>) requires(
			std::is_move_assignable_v<T>)
		{
			if(this != &other) [[likely]]
				{
					m_ValueStorage = std::move(other.m_ValueStorage);
				}
			return *this;
		}

		constexpr ~optional() = default;

		/**
		 * \brief Calls the given functional object and returns an optional.
		 * \details This version will be called when the first argument is an optional and the function does not return
		 * an optional. It will wrap the result into an optional.
		 *
		 */
		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args) -> optional<is_opt_invocable_r_t<FN, optional, Args...>> requires(
			!IsOptional<is_opt_invocable_r_t<FN, optional, Args...>>)
		{
			return {fn(*this, std::forward<Args>(args)...)};
		}

		/**
		 * \brief Calls the given functional object and returns an optional.
		 * \details This version will be called when the first argument is an optional and the function does return
		 * an optional.
		 *
		 */
		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args) -> is_opt_invocable_r_t<FN, optional, Args...> requires(
			IsOptional<is_opt_invocable_r_t<FN, optional, Args...>>)
		{
			return fn(*this, std::forward<Args>(args)...);
		}

		/**
		 * \brief Calls the given functional object and returns an optional.
		 * \details This version will be called when the first argument is not an optional and the function does not
		 * return an optional. It will wrap the result into an optional, and will default initialize (if possible) in
		 * case the current optional does not have a value.
		 *
		 */
		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args) -> optional<is_opt_invocable_r_t<FN, T, Args...>> requires(
			!IsOptional<is_opt_invocable_r_t<FN, T, Args...>> && !is_opt_invocable_v<FN, optional, Args...> &&
			std::is_default_constructible_v<T>)
		{
			if(m_ValueStorage) return {fn(m_ValueStorage.get(), std::forward<Args>(args)...)};
			return fn(T{}, std::forward<Args>(args)...);
		}

		/**
		 * \brief Calls the given functional object and returns an optional.
		 * \details This version will be called when the first argument is not an optional and the function does return
		 * an optional. It will default initialize (if possible) in case the current optional does not have a value.
		 *
		 */
		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args)
			-> is_opt_invocable_r_t<FN, T, Args...> requires(IsOptional<is_opt_invocable_r_t<FN, T, Args...>> &&
															 !is_opt_invocable_v<FN, optional, Args...> &&
															 std::is_default_constructible_v<T>)
		{
			if(m_ValueStorage) return fn(m_ValueStorage.get(), std::forward<Args>(args)...);

			return fn(T{}, std::forward<Args>(args)...);
		}

		constexpr T& value() noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(!m_ValueStorage.has_value(), bad_optional_access);
			return m_ValueStorage.get();
		}
		constexpr const T& value() const noexcept(!config::exceptions)
		{
			PSL_EXCEPT_IF(!m_ValueStorage.has_value(), bad_optional_access);
			return m_ValueStorage.get();
		}

		explicit constexpr operator bool() const noexcept { return m_ValueStorage.has_value(); }
		constexpr bool has_value() const noexcept { return m_ValueStorage.has_value(); }
		constexpr void reset() noexcept { return m_ValueStorage.reset(); }

	  private:
		optional_value_storage<T> m_ValueStorage{nullopt};
	};
} // namespace psl