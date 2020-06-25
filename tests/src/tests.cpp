
#include "gtest/gtest.h"

#include "psl/uid.h"
#include "psl/range.h"
#include "psl/allocator.h"
#include "psl/array.h"

#include <iostream>
#include "psl/thread.h"
#include "psl/profiler.h"
#include "psl/sinks/stream.h"
#include "psl/serialization/format.h"
#include "psl/serialization/stream_writer.h"
#include "psl/threading/workgroup.h"

#include "psl/meta/library.h"

#include "psl/flat_unordered_map.h"
#include "psl/ct_unordered_map.h"

#include "psl/ecs/details/staged_sparse_component_array.h"

class unit
{
	friend psl::serialization_accessor;

  public:
	POLYMORPHIC_ID(unit)
	virtual ~unit() = default;

	// protected:
	auto properties() noexcept { return psl::make_property_pack(m_ID); }
	// PROPERTY_PACK(m_ID);
	psl::property<int, u8"id"> m_ID{0};
};
POLYMORPHIC_INFO(unit)

class player final : public unit
{
	friend psl::serialization_accessor;

  public:
	POLYMORPHIC_ID(player)

	// private:
	PROPERTY_PACK(unit::properties(), m_Gold);
	psl::property<int, u8"gold"> m_Gold{0};
};
POLYMORPHIC_INFO(player)

class enemy final : public unit
{
	friend psl::serialization_accessor;

  public:
	POLYMORPHIC_ID(enemy)

	// private:
	PROPERTY_PACK(unit::properties(), m_Loot);
	psl::property<int, u8"loot"> m_Loot{0};
};
POLYMORPHIC_INFO(enemy)

constexpr size_t mul(auto x, auto y) { return x * y; }

void start() { std::cout << "Starting..." << std::endl; }
void update(size_t frame) { std::cout << "Update" << frame << std::endl; }
void destroy() { std::cout << "Goodbye :(" << std::endl; }

#include "psl/details/sbo_storage.h"
namespace psl
{
	namespace config
	{
		template <typename T = default_setting_t>
		using ustring_allocator = psl::default_allocator_t;

		template <typename T = default_setting_t>
		// sizeof(size_t) * 2 == the non-aliased members of ustring
		constexpr size_t ustring_sbo = (32 - (sizeof(size_t) * 2) - sizeof(ustring_allocator<T>)) / sizeof(char);
	} // namespace config

	template <size_t SBO		 = psl::config::ustring_sbo<config::default_setting_t>,
			  typename Allocator = psl::config::ustring_allocator<config::default_setting_t>>
	class ustring
	{
	  public:
		using char_type		 = char;
		using value_type	 = char_type;
		using size_type		 = size_t;
		using pointer		 = char_type*;
		using cpointer		 = char_type const*;
		using allocator_type = Allocator;

		template <size_t N>
		constexpr ustring(char const (&data)[N])
		{
			if constexpr(N <= SBO)
			{
				if(std::is_constant_evaluated())
				{
					for(size_t i = 0; i != N; ++i) m_Storage.local[i] = data[i];
				}
				else
				{
					memcpy(m_Storage.local.data(), &data, N * sizeof(char_type));
				}
				m_Size = N - 1;
			}
			else
			{
				auto res = m_Allocator.template allocate_n<char_type>(N);
				if(res)
				{
					m_Storage.ext = res.data;
					m_Capacity	= (pointer)ralign_to((size_type)res.tail, sizeof(value_type)) - m_Storage.ext;
					m_Size		  = N - 1;
					memcpy(m_Storage.ext, &data, N * sizeof(char_type));
				}
				else
				{
					PSL_EXCEPT(std::runtime_error, "could not allocate");
				}
			}
		}

		constexpr ~ustring()
		{
			if(!is_stored_inlined())
			{
				m_Allocator.deallocate(m_Storage.ext);
			}
		}

		constexpr size_type size() const noexcept { return m_Size; }
		constexpr size_type capacity() const noexcept { return m_Capacity; }
		constexpr bool is_stored_inlined() const noexcept { return m_Capacity == SBO; }

		constexpr cpointer data() const noexcept { return (is_stored_inlined()) ? m_Storage.local : m_Storage.ext; }
		constexpr pointer data() noexcept { return (is_stored_inlined()) ? m_Storage.local.data() : m_Storage.ext; }

		constexpr operator std::string_view() const noexcept { return std::string_view{data(), size()}; }

	  private:
		details::sbo_storage<char_type, SBO> m_Storage;
		size_t m_Size{0};
		size_t m_Capacity{SBO};
		allocator_type m_Allocator{psl::default_allocator};
	};
} // namespace psl

struct foo
{
	~foo() { printf("my destructor has side effects!\n"); }
};

#include "psl/vec.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	using namespace psl;

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}