#pragma once
#include <type_traits>
#include <psl/types.hpp>
#include <psl/fwd/allocator.hpp>
#include <psl/allocator_traits.hpp>
#include <psl/config.hpp>
#include <psl/types.hpp>
#include <utility>

namespace psl
{
	/**
	 * \brief Abstract memory resource interface.
	 * \details Used to describe the intended API for implementing a custom memory resource.
	 * The Traits can be used to extend the API of the abstract class.
	 */
	class abstract_memory_resource
	{
	  public:
		abstract_memory_resource(const size_t alignment) noexcept : m_Alignment(alignment) {}
		virtual ~abstract_memory_resource()									   = default;
		abstract_memory_resource(const abstract_memory_resource& rhs) noexcept = default;
		abstract_memory_resource(abstract_memory_resource&& rhs) noexcept	  = default;

		abstract_memory_resource& operator=(const abstract_memory_resource& rhs) noexcept = default;
		abstract_memory_resource& operator=(abstract_memory_resource&& rhs) noexcept = default;

		size_t alignment() const noexcept { return m_Alignment; }

	  private:
		size_t m_Alignment;
	};


	/**
	 * \brief Base class to derive your custom memory resource from.
	 *
	 * \tparam Traits
	 */
	template <typename... Traits>
	class traited_memory_resource : public traits::memory_resource_trait<Traits, traited_memory_resource<Traits...>>...,
									public abstract_memory_resource
	{
	  public:
		traited_memory_resource(const size_t alignment) noexcept : abstract_memory_resource(alignment) {}
		virtual ~traited_memory_resource()									 = default;
		traited_memory_resource(const traited_memory_resource& rhs) noexcept = default;
		traited_memory_resource(traited_memory_resource&& rhs) noexcept		 = default;

		traited_memory_resource& operator=(const traited_memory_resource& rhs) noexcept = default;
		traited_memory_resource& operator=(traited_memory_resource&& rhs) noexcept = default;

		template <typename T>
		static consteval bool has_trait()
		{
			if constexpr(sizeof...(Traits) == 0)
				return false;
			else
				return (std::is_same_v<T, Traits> || ...);
		}
	};

	/**
	 * \brief Traited allocator class, as a replacement for the `std::pmr::allocator`
	 * @details The allocator class internally derives its behaviour and functionality based on the traits.
	 * For example, the `psl::traits::basic_allocation` grants the allocator the ability to do allocations using the
	 * alignment of the given memory resource.
	 * Other traits could be adding the ability to override the allocation, or even override the entire behaviour
	 * (through wrapping).
	 *
	 * \tparam Traits
	 */
	template <typename... Traits>
	class allocator final : public traits::allocator_trait<Traits, allocator<Traits...>>...
	{
	  public:
		using abstract_memory_resource_t = abstract_memory_resource;
		using traited_memory_resource_t  = traited_memory_resource<Traits...>;

		allocator() = default;
		allocator(traited_memory_resource_t* memoryResource) noexcept : m_MemoryResource(memoryResource){};
		~allocator() = default;

		allocator(const allocator& other) noexcept = default;
		allocator(allocator&& other) noexcept	  = default;
		allocator& operator=(const allocator& other) noexcept = default;
		allocator& operator=(allocator&& other) noexcept = default;

		abstract_memory_resource_t* abstract_resource() { return (abstract_memory_resource_t*)m_MemoryResource; }

		traited_memory_resource_t* resource() { return m_MemoryResource; }

	  private:
		traited_memory_resource_t* m_MemoryResource{nullptr};
	};

	template <typename T, typename... Traits, typename... Args>
	[[nodiscard]] alloc_results<T> construct(allocator<Traits...>& allocator, Args&&... args)
	{
		auto res = allocator.template allocate<T>();
		new(res.data) T{std::forward<Args>(args)...};
		return res;
	}

	template <typename T, typename... Traits>
	bool destroy(allocator<Traits...>& allocator, T& object)
	{
		if constexpr(std::is_pointer_v<T>)
		{
			return destroy<T>(allocator, *object);
		}
		else
		{
			object.~T();
			return allocator.template deallocate<T>(&object);
		}
	}

	class new_resource
		: public psl::traited_memory_resource<psl::traits::shareable_t<true>, psl::traits::basic_allocation>
	{
		using base_type = psl::traited_memory_resource<psl::traits::shareable_t<true>, psl::traits::basic_allocation>;

	  public:
		new_resource(size_t alignment) : base_type(alignment) {}

	  private:
		alloc_results<void> do_allocate(size_t size, size_t alignment) override;

		bool do_deallocate(void* ptr, size_t size, size_t alignment) override;
	};
} // namespace psl
