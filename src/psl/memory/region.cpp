#include "psl/memory/region.h"
#include <stdlib.h>

using namespace psl::memory;
using namespace psl::config;

region::region(size_t size, size_t alignment) noexcept(!exceptions)
	: region(specialize::region_default_alloc_t<default_setting_t>::type{}, size, alignment)
{}

region::region(paged_alloc_t, size_t size, size_t alignment) noexcept(!exceptions)
	: m_Alignment(alignment), m_Range(0, align_to(size, alignment)), m_Behaviour(behaviour::paged)
{}

region::region(malloc_t, size_t size, size_t alignment) noexcept(!exceptions)
	: m_Alignment(alignment), m_Range(0, align_to(size, alignment)), m_Data(malloc(m_Range.size())),
	  m_Behaviour(behaviour::malloc)
{
	if constexpr(exceptions)
	{
		if(m_Data == nullptr) throw std::runtime_error("could not allocate the requested size");
	}
	else if constexpr(exceptions_as_asserts || asserts)
	{
		assert(m_Data != nullptr || "was unable to allocate the requested size");
	}
	m_Range.shift(static_cast<size_t>((std::uintptr_t)(m_Data)));
	if constexpr(asserts)
	{
		assert(m_Range.size() == align_to(size, alignment) || "was unable to allocate the requested size");
	}
}

region::~region()
{
	switch(m_Behaviour)
	{
	case behaviour::malloc:
	{
		free(m_Data);
	}
	break;
	case behaviour::paged:
	{}
	break;
	default:
		break;
	}
}