#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <psl/details/fixed_ascii_string.hpp>
#include <psl/exceptions.hpp>

namespace psl
{
	using i8  = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using ui8  = std::uint8_t;
	using ui16 = std::uint16_t;
	using ui32 = std::uint32_t;
	using ui64 = std::uint64_t;

	using uint = unsigned int;

	using byte = std::byte;

	inline constexpr std::size_t dynamic_extent = -1;

	namespace _priv
	{
		template <typename T>
		struct id_token
		{
			enum class identifier
			{
				token
			};
		};
	} // namespace _priv

	/**
	 * \brief value initalization tag. Used by constainers and wrappers to force specific construction behaviour of
	 * contained types.
	 *
	 */
	struct value_init_t : public _priv::id_token<value_init_t>
	{
		explicit constexpr value_init_t(identifier) {}
	};
	/**
	 * \copydoc value_init_t
	 */
	inline constexpr value_init_t value_init{value_init_t::identifier::token};

	/**
	 * \brief default initalization tag. Used by constainers and wrappers to force specific construction behaviour of
	 * contained types.
	 *
	 */
	struct default_init_t : public _priv::id_token<default_init_t>
	{
		explicit constexpr default_init_t(identifier) {}
	};
	/**
	 * \copydoc default_init_t
	 */
	inline constexpr default_init_t default_init{default_init_t::identifier::token};

	/**
	 * \brief zero initalization tag. Used by constainers and wrappers to force specific construction behaviour of
	 * contained types.
	 *
	 */
	struct zero_init_t : public _priv::id_token<zero_init_t>
	{
		explicit constexpr zero_init_t(identifier) {}
	};
	/**
	 * \copydoc zero_init_t
	 */
	inline constexpr zero_init_t zero_init{zero_init_t::identifier::token};

	/**
	 * \brief "no-operation" initalization tag. Used by constainers and wrappers to force specific construction
	 * behaviour of contained types.
	 * \warning this signifies "no initialization at all", not "initialize with nothing".
	 * This means an object will not even be constructed.
	 * As example, an `array<>` type will allocate memory for objects, but not construct them at all with this tag. You
	 * will have to in-place construct them after if you wish to have access to non-trivial types (or don't like
	 * undefined behaviour).
	 *
	 */
	struct nop_init_t : public _priv::id_token<nop_init_t>
	{
		explicit constexpr nop_init_t(identifier) {}
	};
	/**
	 * \copydoc nop_init_t
	 */
	inline constexpr nop_init_t nop_init{nop_init_t::identifier::token};

	template <typename T>
	struct type_value
	{
		using type = T;
	};

	/**
	 * \brief Wraps a NTTP into a type so you can pass it around, or use it in conditional_t
	 *
	 * \tparam T
	 */
	template <typename T, T V>
	struct wrapped_vtype
	{
		static inline constexpr T value = V;
	};

	namespace _priv
	{
		struct no_type_t
		{
			using type = no_type_t;
		};

		template <size_t N, typename... Ts>
		struct get_nth
		{
			using type = no_type_t;
		};

		template <size_t N, typename T, typename... Ts>
		struct get_nth<N, T, Ts...> : public std::conditional_t<N == 0, type_value<T>, get_nth<N - 1, Ts...>>
		{};

		template <size_t N, typename... Ts>
		using get_nth_t = typename get_nth<N, Ts...>::type;

		template <typename N, typename... Ts>
		struct is_nth : std::false_type
		{
			using type = no_type_t;
		};
		template <typename>
		struct always_false : std::false_type
		{};

		template <typename T, fixed_ascii_string ErrorMessage = "">
		struct error_out : std::false_type
		{
			static_assert(always_false<T>::value);
		};

		template <typename T, typename>
		constexpr bool is_same_specialization_v{error_out<
			T, "ErrorMessage: does not support the combination, likely it has more than 8 template arguments.">::value};

		/**
		 * \brief region contains the implementation of is_same_specialization_v's many permutations. Currently it
		 * supports up to 8 template parameters (will error out otherwise)
		 *
		 */
#pragma region is_same_specialization_v
#define CONCATE_(X, Y) X##Y
#define CONCATE(X, Y) CONCATE_(X, Y)

#define NUM_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, TOTAL, ...) TOTAL
#define NUM_ARGS(...) NUM_ARGS_(__VA_ARGS__, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define VA_MACRO(MACRO, ...) CONCATE(MACRO, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define GEN_SPECIALIZATION_impl(...) VA_MACRO(GEN_SPECIALIZATION_impl_, __VA_ARGS__)
#define GEN_SPECIALIZATION_impl_1(i0)                                                                                  \
	template <template <i0> typename T, i0 A0, i0 B0>                                                                  \
	constexpr bool is_same_specialization_v<T<A0>, T<B0>>{true};
#define GEN_SPECIALIZATION_impl_2(i0, i1)                                                                              \
	template <template <i0, i1> typename T, i0 A0, i1 A1, i0 B0, i1 B1>                                                \
	constexpr bool is_same_specialization_v<T<A0, A1>, T<B0, B1>>{true};
#define GEN_SPECIALIZATION_impl_3(i0, i1, i2)                                                                          \
	template <template <i0, i1, i2> typename T, i0 A0, i1 A1, i2 A2, i0 B0, i1 B1, i2 B2>                              \
	constexpr bool is_same_specialization_v<T<A0, A1, A2>, T<B0, B1, B2>>{true};
#define GEN_SPECIALIZATION_impl_4(i0, i1, i2, i3)                                                                      \
	template <template <i0, i1, i2, i3> typename T, i0 A0, i1 A1, i2 A2, i3 A3, i0 B0, i1 B1, i2 B2, i3 B3>            \
	constexpr bool is_same_specialization_v<T<A0, A1, A2, A3>, T<B0, B1, B2, B3>>{true};
#define GEN_SPECIALIZATION_impl_5(i0, i1, i2, i3, i4)                                                                  \
	template <template <i0, i1, i2, i3, i4> typename T, i0 A0, i1 A1, i2 A2, i3 A3, i4 A4, i0 B0, i1 B1, i2 B2, i3 B3, \
			  i4 B4>                                                                                                   \
	constexpr bool is_same_specialization_v<T<A0, A1, A2, A3, A4>, T<B0, B1, B2, B3, B4>>{true};
#define GEN_SPECIALIZATION_impl_6(i0, i1, i2, i3, i4, i5)                                                              \
	template <template <i0, i1, i2, i3, i4, i5> typename T, i0 A0, i1 A1, i2 A2, i3 A3, i4 A4, i5 A5, i0 B0, i1 B1,    \
			  i2 B2, i3 B3, i4 B4, i5 B5>                                                                              \
	constexpr bool is_same_specialization_v<T<A0, A1, A2, A3, A4, A5>, T<B0, B1, B2, B3, B4, B5>>{true};
#define GEN_SPECIALIZATION_impl_7(i0, i1, i2, i3, i4, i5, i6)                                                          \
	template <template <i0, i1, i2, i3, i4, i5, i6> typename T, i0 A0, i1 A1, i2 A2, i3 A3, i4 A4, i5 A5, i6 A6,       \
			  i0 B0, i1 B1, i2 B2, i3 B3, i4 B4, i5 B5, i6 B6>                                                         \
	constexpr bool is_same_specialization_v<T<A0, A1, A2, A3, A4, A5, A6>, T<B0, B1, B2, B3, B4, B5, B6>>{true};
#define GEN_SPECIALIZATION_impl_8(i0, i1, i2, i3, i4, i5, i6, i7)                                                      \
	template <template <i0, i1, i2, i3, i4, i5, i6, i7> typename T, i0 A0, i1 A1, i2 A2, i3 A3, i4 A4, i5 A5, i6 A6,   \
			  i7 A7, i0 B0, i1 B1, i2 B2, i3 B3, i4 B4, i5 B5, i6 B6, i7 B7>                                           \
	constexpr bool is_same_specialization_v<T<A0, A1, A2, A3, A4, A5, A6, A7>, T<B0, B1, B2, B3, B4, B5, B6, B7>>{     \
		true};


#define W7() W7_impl(auto) W7_impl(typename)
#define W7_impl(...) W6_impl(__VA_ARGS__, auto) W6_impl(__VA_ARGS__, typename)
#define W6() W6_impl(auto) W6_impl(typename)
#define W6_impl(...) W5_impl(__VA_ARGS__, auto) W5_impl(__VA_ARGS__, typename)
#define W5() W5_impl(auto) W5_impl(typename)
#define W5_impl(...) W4_impl(__VA_ARGS__, auto) W4_impl(__VA_ARGS__, typename)
#define W4() W4_impl(auto) W4_impl(typename)
#define W4_impl(...) W3_impl(__VA_ARGS__, auto) W3_impl(__VA_ARGS__, typename)
#define W3() W3_impl(auto) W3_impl(typename)
#define W3_impl(...) W2_impl(__VA_ARGS__, auto) W2_impl(__VA_ARGS__, typename)
#define W2() W2_impl(auto) W2_impl(typename)
#define W2_impl(...) W1_impl(__VA_ARGS__, auto) W1_impl(__VA_ARGS__, typename)
#define W1() W1_impl(auto) W1_impl(typename)
#define W1_impl(...) GEN_SPECIALIZATION_impl(__VA_ARGS__, auto) GEN_SPECIALIZATION_impl(__VA_ARGS__, typename)
#define W0() GEN_SPECIALIZATION_impl(auto) GEN_SPECIALIZATION_impl(typename)

		W0()
		W1()
		W2()
		W3()
		W4()
		W5()
		W6()
		W7()

#undef GEN_SPECIALIZATION_impl
#undef GEN_SPECIALIZATION_impl_1
#undef GEN_SPECIALIZATION_impl_2
#undef GEN_SPECIALIZATION_impl_3
#undef GEN_SPECIALIZATION_impl_4
#undef GEN_SPECIALIZATION_impl_5
#undef GEN_SPECIALIZATION_impl_6
#undef GEN_SPECIALIZATION_impl_7
#undef GEN_SPECIALIZATION_impl_8
#undef W0
#undef W1
#undef W2
#undef W3
#undef W4
#undef W5
#undef W6
#undef W7
#undef W0_impl
#undef W1_impl
#undef W2_impl
#undef W3_impl
#undef W4_impl
#undef W5_impl
#undef W6_impl
#undef W7_impl

#undef CONCATE_
#undef CONCATE

#undef NUM_ARGS_
#undef NUM_ARGS
#undef VA_MACRO
#pragma endregion is_same_specialization_v

		template <size_t N, typename T, typename Y, typename... Ys>
		struct index_of : std::conditional_t<
							  std::is_same_v<T, Y>,
							  std::conditional_t<!std::disjunction_v<std::is_same<T, Ys>...>, wrapped_vtype<size_t, N>,
												 error_out<T,
														   "ErrorMessage: does not support duplicate types in the "
														   "pack, T is repeated more than once.">>,
							  index_of<N + 1, T, Ys...>>
		{};
	} // namespace _priv

	template <typename T, typename Y>
	constexpr bool is_same_specialization_v{
		_priv::is_same_specialization_v<std::remove_cvref_t<T>, std::remove_cvref_t<Y>>};

	template <typename... Ts>
	struct type_pack
	{};

	template <size_t N, typename T>
	struct type_pack_element
	{};

	template <size_t N, typename... Ts>
	struct type_pack_element<N, type_pack<Ts...>> : _priv::get_nth<N, Ts...>
	{};

	template <size_t N, typename T>
	using type_pack_element_t = typename type_pack_element<N, T>::type;

	template <typename T, typename Y>
	struct type_pack_index_of
	{};

	template <typename T, typename... Ys>
	struct type_pack_index_of<T, type_pack<Ys...>> : _priv::index_of<0, T, Ys...>
	{};

	template <typename T, typename Y>
	using type_pack_index_of_v = typename type_pack_index_of<T, Y>::value;

	struct monostate
	{};
} // namespace psl