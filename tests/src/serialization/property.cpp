#include "psl/serialization/property.h"
#include "psl/serialization/versioning.h"
#include "psl/serialization/serialization.h"
#include "psl/meta/data.h"
#include "gtest/gtest.h"
#include <vector>

TEST(property, generic)
{
	psl::property<char, u8"myname"> c{'1'};
	psl::property<int> i{0};

	ASSERT_NE(c, i);

	psl::property<std::vector<int>> vec{std::initializer_list<int>{1, 2, 3, 4, 5, 6}};

	{
		auto i = 0;
		for(auto element : vec) ASSERT_EQ(++i, element);
		for(i = 0; i < (int)vec->size(); ++i) ASSERT_EQ(i + 1, vec[i]);
		++vec[0];
		ASSERT_EQ(vec[0], 2);
	}

	psl::property<int, u8"heyaa"> i2 = i;

	i2 += 5;
	ASSERT_NE(i, i2);
	i = std::move(i2);


	i = i + i + 5 + i;
	ASSERT_EQ(i, 20);
}

struct foo;

namespace psl::versioning
{
	template <>
	struct packet<foo, 0>
	{
		auto properties() noexcept { return make_property_pack(health, damage_multiplier); }

		psl::property<int, u8"health"> health{100};
		psl::property<int, u8"damage_multiplier"> damage_multiplier{100};
	};

	template <>
	struct packet<foo, 1>
	{
		auto properties() noexcept { return make_property_pack(health, is_hero); }

		psl::property<int, u8"health"> health{145678};
		psl::property<std::optional<bool>, u8"hero"> is_hero{};
	};
} // namespace psl::versioning

struct foo : public psl::versioning::packet<foo>
{};

struct bar
{};

#include "psl/serialization/stream_writer.h"
#include <iostream>

TEST(versioning, generic)
{
	static_assert(psl::versioning::IsVersioned<foo>);
	static_assert(psl::versioning::HasVersion<foo, 1>);
	static_assert(psl::versioning::latest_version_for<foo>() == 1);

	static_assert(!psl::versioning::IsVersioned<bar>);
	static_assert(psl::versioning::latest_version_for_or<bar>() == 0);


	foo _foo{};
	psl::stream_writer writer{std::cout};
	psl::serialize(_foo, writer);
	static_assert(psl::IsSerializable<foo>);

	// psl::static_array<uint8_t, 16> res{};
	// psl::from_hex_unformatted(u8"83EC45B2", (uint8_t*)res.data(), 2);
	// psl::from_hex_unformatted(u8"3AFF", (uint8_t*)res.data() + 4, 1);
	// psl::from_hex_unformatted(u8"AAD8", (uint8_t*)res.data() + 6, 1);
	// psl::from_hex_unformatted(u8"78F7", (uint8_t*)res.data() + 8, 1);
	// psl::from_hex_unformatted(u8"F4D566EA4E50", (uint8_t*)res.data() + 10, 3);
	// // psl::from_hex<std::span<uint8_t, 2>>(u8"0x3AFF", res.data() + 4);
	// // psl::from_hex<std::span<uint8_t, 2>>(u8"0xAAD8", res.data() + 6);
	// // psl::from_hex<std::span<uint8_t, 2>>(u8"0x78F7", res.data() + 8);
	// // psl::from_hex<std::span<uint8_t, 6>>(u8"0xF4D566EA4E50", {res.data() + 10});

	psl::uid uid{psl::generate_t{}};
	// std::cout << psl::to_pstring(psl::to_hex_unformatted(std::span<std::byte>{(std::byte*)res.data(), res.size()}))
	// 		  << std::endl;
	std::cout << psl::to_pstring(uid.to_string()) << std::endl;

	// psl::meta::data data{psl::uid::generate(), {{u8"tagme baby", u8"tag twoooo"}}};
	// std::cout << psl::to_pstring(data.uid().to_string());
	// psl::serialize(data, writer);
}