#pragma once
#include <system_error>
#include <psl/type_concepts.hpp>
#include <psl/exceptions.hpp>
#include <psl/types.hpp>

namespace psl
{
	class bad_exceptional_access : public exception
	{
	  public:
		bad_exceptional_access(const source_location& location = source_location::current())
			: exception("bad exceptional access", location)
		{}
		virtual ~bad_exceptional_access() = default;
	};
	template <typename T, typename Error>
	class expected;

	inline namespace details
	{
		template <typename T>
		struct is_expected : std::false_type
		{};

		template <typename... Args>
		struct is_expected<expected<Args...>> : std::true_type
		{};

		template <typename T>
		inline constexpr bool is_expected_v = is_expected<T>::value;
	} // namespace details

	template <typename T>
	concept IsExpected = is_expected_v<std::remove_cvref_t<T>>;

	/**
	 * \brief contains either a result (the expected), or an error
	 * \details Models an object that can either contain the result of an operation, or an error in case the operation
	 * failed. It cannot be re-assigned, meaning that the value or error is set at construction time.
	 * After this it supports continuation behaviours `then()` and `on_error()`.
	 * \note when storing reference types, it will be stored as a pointer under the hood.
	 * \todo should reference types be stored as `psl::optional` instead?
	 *
	 * \tparam T value type to contain.
	 * \tparam Error error type to throw, by default this is `std::error_code`.
	 * \throw bad_exceptional_access on accessing/consuming the results when no value is present, or accessing the error
	 * when no error is present.
	 */
	template <typename T, typename Error = std::error_code>
	class expected
	{
	  public:
		static_assert(std::is_default_constructible_v<T>, "must be default constructible");
		static_assert(MoveAssignable<T> || CopyAssignable<T>,
					  "contained type should either be move, or copy assignable");
		using value_type = T;
		using error_type = Error;

		constexpr expected() = delete;
		explicit constexpr expected(default_value_t) noexcept(std::is_nothrow_default_constructible_v<T>) requires(
			std::is_default_constructible_v<T>)
			: m_Value(), m_HasValue(true){};

		template <typename Arg, typename... Args>
		constexpr expected(Arg&& arg, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<T, Arg, Args...>) requires(!IsExpected<Arg>)
			: m_Value(std::forward<Arg>(arg), std::forward<Args>(args)...), m_HasValue(true)
		{}

		constexpr expected(const expected& other) noexcept(std::is_nothrow_copy_constructible_v<T>) requires(
			std::is_copy_constructible_v<T>)
			: m_Value(other.m_Value), m_Error(other.m_Error), m_HasValue(other.m_HasValue)
		{}
		constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible_v<T>) requires(
			std::is_move_constructible_v<T>)
			: m_Value(std::move(other.m_Value)), m_Error(other.m_Error), m_HasValue(other.m_HasValue)
		{}

		constexpr expected& operator=(const expected& other) noexcept(std::is_nothrow_copy_assignable_v<T>) requires(
			std::is_copy_assignable_v<T>)
		{
			if(this != &other)
			{
				m_HasValue = other.m_HasValue;
				if(m_HasValue) m_Value = other.m_Value;
				m_Error = other.m_Error;
			}
			return *this;
		}
		constexpr expected& operator=(expected&& other) noexcept(std::is_nothrow_move_assignable_v<T>) requires(
			std::is_move_assignable_v<T>)
		{
			if(this != &other)
			{
				m_HasValue = other.m_HasValue;
				if(m_HasValue) m_Value = std::move(other.m_Value);
				m_Error = other.m_Error;
			}
			return *this;
		}

		constexpr expected(Error& error) noexcept requires(CopyAssignable<Error>) : m_Error(error){};
		constexpr expected(const Error& error) noexcept requires(CopyAssignable<Error>) : m_Error(error){};
		constexpr expected(Error&& error) noexcept requires(MoveAssignable<Error>) : m_Error(std::move(error)){};

		/**
		 * \brief runs the given function when the expected is in an error condition. The first argument will be the
		 * Error object.
		 * \note the error condition could be from up the chain (if done in a continuation context)
		 *
		 * \tparam FN function object
		 * \tparam Args arguments to forward to the function object
		 */
		template <typename FN, typename... Args>
		constexpr expected on_error(FN&& fn, Args&&... args) noexcept(
			std::is_nothrow_invocable_v<FN, Error, Args...>) requires(std::is_invocable_v<FN, Error, Args...>)
		{
			if(!m_HasValue)
			{
				fn(m_Error, std::forward<Args>(args)...);
			}
			return std::move(*this);
		}

		/**
		 * \brief Continuation method, will automatically run the function if not in an error state, and will wrap the
		 * results in an `expected<>` if the result type isn't an `expected<>` already.
		 *
		 * \tparam FN function object
		 * \tparam Args arguments to the function object.
		 */
		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args) -> expected<std::invoke_result_t<FN, T, Args...>, Error> requires(
			!IsExpected<std::invoke_result_t<FN, T, Args...>>)
		{
			if(m_HasValue)
			{
				m_HasValue = false;
				return {fn(std::move(m_Value), std::forward<Args>(args)...)};
			}
			return m_Error;
		}

		template <typename FN, typename... Args>
		constexpr auto then(FN&& fn, Args&&... args) -> std::invoke_result_t<FN, T, Args...> requires(
			IsExpected<std::invoke_result_t<FN, T, Args...>>&&
				std::is_same_v<error_type, typename std::invoke_result_t<FN, T, Args...>::error_type>)
		{
			if(m_HasValue)
			{
				m_HasValue = false;
				return fn(std::move(m_Value), std::forward<Args>(args)...);
			}
			return m_Error;
		}

		/**
		 * \return `T` if not in an error state.
		 * \throw in case of being in an error state.
		 */
		constexpr auto consume() noexcept(NothrowMoveCopyAssignable<T> && !config::exceptions)
			-> decltype(std::move(std::declval<T>()))
		{
			PSL_EXCEPT_IF(!m_HasValue, bad_exceptional_access);
			m_HasValue = false;
			return std::move(m_Value);
		}

		/**
		 * \brief Recovers from an error state if in an error state, otherwise equivalent to a `NOP`
		 *
		 * \tparam Args arguments to construct a valid `T` object with in case of error state.
		 */
		template <typename... Args>
		expected recover(Args&&... args) noexcept(
			NothrowMoveCopyAssignable<T>&& std::is_nothrow_constructible_v<
				T, Args...>) requires(MoveCopyAssignable<T>&& std::is_constructible_v<T, Args...>)
		{
			if(!m_HasValue)
			{
				m_Error	= {};
				m_Value	= std::move(T{std::forward<Args>(args)...});
				m_HasValue = true;
			}
			return std::move(*this);
		}

		constexpr operator bool() const noexcept { return m_HasValue; }

	  private:
		T m_Value;
		Error m_Error;
		bool m_HasValue{false};
	};
} // namespace psl