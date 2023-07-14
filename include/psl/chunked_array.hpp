#pragma once

#include <psl/array.hpp>
#include <psl/bytes.hpp>
#include <psl/types.hpp>

namespace psl {
template <typename T>
class chunked_array_iterator {};


using chunk_element_count = details::strong_type_wrapper_t<size_t>;
using chunk_bytesize	  = details::strong_type_wrapper_t<size_t>;

inline namespace details {
	template <typename CountainedType, typename T>
	constexpr size_t size_to_element_count(T const& value) {
		if constexpr(std::is_same_v<T, chunk_element_count>) {
			return *value;
		} else {
			return (*value) / sizeof(CountainedType);
		}
	}
}	 // namespace details

template <typename T,
		  size_t Extent			   = dynamic_extent,
		  auto ChunkSize		   = chunk_element_count {1024},
		  IsArraySettings Settings = settings::array<>>
class chunked_array {
	constexpr auto chunk_size_v = size_to_element_count(*ChunkSize);
	using chunk_array_t			= psl::array<size_t, Extent, Settings>;
	using data_array_t			= psl::array<psl::array<T, chunk_size_v, Settings>, Extent, Settings>;

	chunk_array_t m_ChunkArray {};
	data_array_t m_Data {};
};
}	 // namespace psl