#include "gtest/gtest.h"
#include "psl/vec.h"

using namespace psl;

// template <typename T, size_t N>
// class VecTest : public ::testing::Test
// {
//   public:
// 	T data;
// };

// using MyTypes = ::testing::Types<ivec1, ivec2, ivec3, ivec4>;

static_assert(std::is_standard_layout_v<ivec3>, "type should remain standard layout compatible");
static_assert(std::is_standard_layout_v<vec3>, "type should remain standard layout compatible");
static_assert(std::is_standard_layout_v<dvec3>, "type should remain standard layout compatible");

TEST(ivec3, addition)
{
	ivec3 v0{0, 1, 2};
	ivec3 v1{-800, 432, 6565};
	ivec3 v2{5430, -1, 9};
	ASSERT_EQ(v0 + v1, v1 + v0);
	ASSERT_EQ(v0 + (v1 + v2), (v0 + v1) + v2);
}

TEST(ivec3, subtraction)
{
	ivec3 v0{0, 1, 2};
	ivec3 v1{-800, 432, 6565};
	ASSERT_EQ(v0 - v1, ivec3(800, -431, -6563));
}

TEST(ivec3, multiplication)
{
	ivec3 v0{0, 1, 2};
	ivec3 v1{-800, 432, 6565};
	ASSERT_EQ(v0 * v1, ivec3(0, 432, 13130));
}

TEST(ivec3, division)
{
	ivec3 v0{100, 4, 9};
	ivec3 v1{20, 2, 1};
	ASSERT_EQ(v0 / v1, ivec3(5, 2, 9));
}

TEST(ivec3, magnitude)
{
	ivec3 v0{1, 2, 2};
	ASSERT_EQ(magnitude(v0), 3);
}

TEST(ivec3, square_magnitude)
{
	ivec3 v0{1, 2, 2};
	ASSERT_EQ(square_magnitude(v0), 9);
}

TEST(ivec3, dot)
{
	ivec3 v0{0, 0, 1};
	ivec3 v1{0, 1, 0};
	ASSERT_EQ(dot(v0, v1), 0);
	ASSERT_EQ(dot(v0, v0), 1);
	ASSERT_EQ(dot(v0, -v0), -1);
}

TEST(ivec3, cross)
{
	ivec3 v0{0, 0, 1};
	ivec3 v1{0, 1, 0};
	ivec3 v2{1, 0, 0};
	ASSERT_EQ(cross(v0, v1), -v2);
	ASSERT_EQ(cross(v1, v0), v2);
}

TEST(ivec3, min)
{
	ivec3 v0{0, 0, 1};
	ivec3 v1{0, 1, 0};
	ivec3 v2{1, 0, 0};
	ivec3 v3{ivec3::one};
	ASSERT_EQ(min(v0, v1, v2, v3), ivec3::zero);
}

TEST(ivec3, max)
{
	ivec3 v0{0, 0, 1};
	ivec3 v1{0, 1, 0};
	ivec3 v2{1, 0, 0};
	ivec3 v3{ivec3::zero};
	ASSERT_EQ(max(v0, v1, v2, v3), ivec3::one);
}

TEST(ivec3, abs)
{
	ivec3 v0{-9, 80, -12};
	ivec3 v1{9, 80, 12};
	ASSERT_EQ(abs(v0), v1);
}

TEST(ivec3, compound)
{
	ivec3 v0{2, 2, 2};
	ASSERT_EQ(compound(v0), 6);
}

TEST(ivec3, compound_fn)
{
	ivec3 v0{2, 2, 2};
	ASSERT_EQ(compound(v0, std::multiplies<int>{}), 8);
}


TEST(ivec3, rescale)
{
	ivec3 v0{2, 2, 2};
	ASSERT_EQ(scale_inplace(v0, 3), ivec3(6, 6, 6));
}

TEST(ivec3, accessors)
{
	ivec3 v0{0, 1, 2};
	ASSERT_EQ(v0.xy(), v0.xy());
	ASSERT_EQ(v0.xy(), ivec2(0, 1));
	ASSERT_EQ(v0.zyx(), ivec3(2, 1, 0));
	ASSERT_EQ(v0.zzz() + v0.zzz(), scale(v0.zzz(), 2));
	ASSERT_EQ(v0.z(), 2);
	v0.zy()[0] = 4;
	ASSERT_EQ(v0.z(), 4);
	v0.zy() = ivec2(0, 1);
	v0.zy() = ivec2(v0.yz());
	ASSERT_EQ(v0.zy(), ivec2(1, 0));
}

TEST(ivec3, resize)
{
	ivec3 v0{0, 1, 2};
	ASSERT_EQ(v0.resize<2>().size(), 2);
	ASSERT_EQ(v0.resize<2>(), ivec2(0, 1));
	ASSERT_EQ(v0.resize<4>().size(), 4);
	ASSERT_EQ(v0.resize<4>(), ivec4(0, 1, 2, 0));
}