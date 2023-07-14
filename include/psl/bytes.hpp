#pragma once
#include <psl/strong_type_wrapper.hpp>

namespace psl {
/**
 * @brief Strong wrapper that signifies bytes as a numeric object.
 *
 */
using bytes_t = strong_type_wrapper_t<size_t>;
}	 // namespace psl