#pragma once
#include <cstdint>
#include <cstddef>

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
} // namespace psl