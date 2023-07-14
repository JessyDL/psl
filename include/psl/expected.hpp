#pragma once
#include <psl/details/optional_value_storage.hpp>
#include <psl/exceptions.hpp>
#include <psl/type_concepts.hpp>
#include <psl/types.hpp>
#include <system_error>

namespace psl {
template <typename T, typename Error>
class expected;

inline namespace details {
	template <typename T>
	struct is_expected : std::false_type {};

	template <typename... Args>
	struct is_expected<expected<Args...>> : std::true_type {};

	template <typename T>
	inline constexpr bool is_expected_v = is_expected<T>::value;
}	 // namespace details

template <typename T>
concept IsExpected = is_expected_v<std::remove_cvref_t<T>>;

/**
 * \brief contains either a result (the expected), or an error
 * \details Models an object that can either contain the result of an operation, or an error in case the operation
 * failed. It cannot be re-assigned, meaning that the value or error is set at construction time.
 * After this it supports continuation behaviours `then()` and `on_error()`.
 * \todo should references and pointers be supported in this type, or fall under a distinct type such as
 * expected_ref?
 *
 * \tparam T value type to contain.
 * \tparam Error error type to throw, by default this is `std::error_code`.
 * \throw bad_exceptional_access on accessing/consuming the results when no value is present, or accessing the error
 * when no error is present.
 */
template <typename T, typename Error = std::error_code>
class expected {
  public:
	using bad_exceptional_error_access =
	  bad_access<expected, "bad exceptional access, tried to access an error, but expected contains a value instead.">;
	using bad_exceptional_value_access =
	  bad_access<expected, "bad exceptional access, tried to access a value, but expected contains an error instead.">;

	static_assert(std::is_same_v<T, std::remove_pointer_t<std::remove_cvref_t<T>>>,
				  "no support for reference or pointer types");
	static_assert(MoveAssignable<T> || CopyAssignable<T>, "contained type should either be move, or copy assignable");
	using value_type = T;
	using error_type = Error;

	constexpr expected() = delete;
	explicit constexpr expected(value_init_t) noexcept(std::is_nothrow_constructible_v<T>)
		requires(std::is_constructible_v<T>)
		: m_ValueStorage({}) {}
	explicit constexpr expected(default_init_t) noexcept(std::is_nothrow_default_constructible_v<T>)
		requires(std::is_default_constructible_v<T>)
		: m_ValueStorage() {}
	explicit constexpr expected(zero_init_t) noexcept(std::is_nothrow_constructible_v<T, zero_init_t>)
		requires(std::is_constructible_v<T, zero_init_t>)
		: m_ValueStorage(zero_init) {}

	template <typename... Args>
		requires(disable_perfect_forward_illegal_type_v<expected, Args...>)
	constexpr expected(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
		: m_ValueStorage(std::forward<Args>(args)...) {}

	constexpr expected(const expected& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
		requires(std::is_copy_constructible_v<T>)
		: m_ValueStorage(other.m_ValueStorage), m_Error(other.m_Error) {}
	constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
		requires(std::is_move_constructible_v<T>)
		: m_ValueStorage(std::move(other.m_ValueStorage)), m_Error(other.m_Error) {}

	constexpr expected& operator=(const expected& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
		requires(std::is_copy_assignable_v<T>)
	{
		if(this != &other) [[likely]] {
			m_ValueStorage = other.m_ValueStorage;
			m_Error		   = other.m_Error;
		}
		return *this;
	}
	constexpr expected& operator=(expected&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
		requires(std::is_move_assignable_v<T>)
	{
		if(this != &other) [[likely]] {
			m_ValueStorage = std::move(other.m_ValueStorage);
			m_Error		   = other.m_Error;
		}
		return *this;
	}

	constexpr ~expected() = default;

	constexpr expected(Error& error) noexcept
		requires(CopyAssignable<Error>)
		: m_Error(error) {};
	constexpr expected(const Error& error) noexcept
		requires(CopyAssignable<Error>)
		: m_Error(error) {};
	constexpr expected(Error&& error) noexcept
		requires(MoveAssignable<Error>)
		: m_Error(std::move(error)) {};

	/**
	 * \brief runs the given function when the expected is in an error condition. The first argument will be the
	 * Error object.
	 * \note the error condition could be from up the chain (if done in a continuation context)
	 *
	 * \tparam FN function object
	 * \tparam Args arguments to forward to the function object
	 */
	template <typename FN, typename... Args>
	constexpr expected& on_error(FN&& fn, Args&&... args) noexcept(std::is_nothrow_invocable_v<FN, Error, Args...>)
		requires(std::is_invocable_v<FN, Error, Args...>)
	{
		if(!m_ValueStorage) [[unlikely]] {
			fn(m_Error, std::forward<Args>(args)...);
		}
		return *this;
	}

	/**
	 * \brief Continuation method, will automatically run the function if not in an error state, and will wrap the
	 * results in an `expected<>` if the result type isn't an `expected<>` already.
	 *
	 * \tparam FN function object
	 * \tparam Args arguments to the function object.
	 */
	template <typename FN, typename... Args>
	constexpr auto then(FN&& fn, Args&&... args) -> expected<std::invoke_result_t<FN, T, Args...>, Error>
		requires(!IsExpected<std::invoke_result_t<FN, T, Args...>>)
	{
		if(m_ValueStorage) [[likely]] {
			return {fn(m_ValueStorage.take(), std::forward<Args>(args)...)};
		}
		return m_Error;
	}

	template <typename FN, typename... Args>
	constexpr auto then(FN&& fn, Args&&... args) -> std::invoke_result_t<FN, T, Args...>
		requires(IsExpected<std::invoke_result_t<FN, T, Args...>> &&
				 std::is_same_v<error_type, typename std::invoke_result_t<FN, T, Args...>::error_type>)
	{
		if(m_ValueStorage) [[likely]] {
			return fn(m_ValueStorage.take(), std::forward<Args>(args)...);
		}
		return m_Error;
	}

	/**
	 * \return `T` if not in an error state.
	 * \throw in case of being in an error state.
	 */
	constexpr auto consume() noexcept(NothrowMoveCopyAssignable<T> && !config::exceptions) {
		PSL_EXCEPT_IF(!m_ValueStorage.has_value(), bad_exceptional_value_access);
		return m_ValueStorage.take();
	}

	/**
	 * \return `Error` if in an error state.
	 * \throw in case of not being in an error state.
	 */
	constexpr auto consume_error() noexcept(!config::exceptions) -> error_type {
		PSL_EXCEPT_IF(m_ValueStorage.has_value(), bad_exceptional_error_access);
		return m_Error;
	}

	/**
	 * \brief Recovers from an error state if in an error state, otherwise equivalent to a `NOP`
	 *
	 * \tparam Args arguments to construct a valid `T` object with in case of error state.
	 */
	template <typename... Args>
	constexpr expected&
	recover(Args&&... args) noexcept(NothrowMoveCopyAssignable<T>&& std::is_nothrow_constructible_v<T, Args...>)
		requires(MoveCopyAssignable<T> && std::is_constructible_v<T, Args...>)
	{
		if(!m_ValueStorage) [[unlikely]] {
			m_Error = {};
			m_ValueStorage.set(std::forward<Args>(args)...);
		}
		return *this;
	}

	constexpr bool has_value() const noexcept { return m_ValueStorage.has_value(); }
	explicit constexpr operator bool() const noexcept { return m_ValueStorage.has_value(); }

  private:
	optional_value_storage<T> m_ValueStorage {nullopt};
	Error m_Error;
};
}	 // namespace psl