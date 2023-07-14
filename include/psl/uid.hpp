#pragma once

#include <psl/exceptions.hpp>
#include <psl/types.hpp>

#include <random>

#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <string>

namespace psl {
struct generate_t {};

class uuidv4 final {
  public:
	constexpr uuidv4() noexcept								  = default;
	constexpr ~uuidv4()										  = default;
	constexpr uuidv4(const uuidv4& other) noexcept			  = default;
	constexpr uuidv4(uuidv4&& other) noexcept				  = default;
	constexpr uuidv4& operator=(const uuidv4& other) noexcept = default;
	constexpr uuidv4& operator=(uuidv4&& other) noexcept	  = default;

	explicit uuidv4(generate_t) noexcept;

	inline constexpr bool operator==(const uuidv4& rhs) const noexcept { return m_Data == rhs.m_Data; }
	inline constexpr bool operator!=(const uuidv4& rhs) const noexcept { return m_Data != rhs.m_Data; }
	inline constexpr bool operator<(const uuidv4& rhs) const noexcept { return m_Data < rhs.m_Data; }
	inline constexpr bool operator<=(const uuidv4& rhs) const noexcept { return m_Data <= rhs.m_Data; }
	inline constexpr bool operator>(const uuidv4& rhs) const noexcept { return m_Data > rhs.m_Data; }
	inline constexpr bool operator>=(const uuidv4& rhs) const noexcept { return m_Data >= rhs.m_Data; }

	explicit constexpr operator bool() const noexcept { return *this != uuidv4 {}; }

	static auto generate(std::mt19937_64& rng) {
		std::uniform_int_distribution<uint64_t> distribution {std::numeric_limits<uint64_t>::min(),
															  std::numeric_limits<uint64_t>::max()};

		const __m128i and_mask = _mm_set_epi64x(0xFFFFFFFFFFFF0FFFull, 0x3FFFFFFFFFFFFFFFull);
		const __m128i or_mask  = _mm_set_epi64x(0x0000000000004000ull, 0x8000000000000000ull);
		__m128i n			   = _mm_set_epi64x(distribution(rng), distribution(rng));
		__m128i uuid		   = _mm_or_si128(_mm_and_si128(n, and_mask), or_mask);

		uuidv4 res {};
		_mm_store_si128((__m128i*)res.m_Data, uuid);
		return res;
	}

	std::string to_string() const noexcept {
		std::string str;
		str.resize(36);
		__m128i x = _mm_load_si128((__m128i*)m_Data);
		auto mem  = str.data();

		const __m256i mask		   = _mm256_set1_epi8(0x0F);
		const __m256i add		   = _mm256_set1_epi8(0x06);
		const __m256i alpha_mask   = _mm256_set1_epi8(0x10);
		const __m256i alpha_offset = _mm256_set1_epi8(0x57);

		__m256i a	   = _mm256_castsi128_si256(x);
		__m256i as	   = _mm256_srli_epi64(a, 4);
		__m256i lo	   = _mm256_unpacklo_epi8(as, a);
		__m128i hi	   = _mm256_castsi256_si128(_mm256_unpackhi_epi8(as, a));
		__m256i c	   = _mm256_inserti128_si256(lo, hi, 1);
		__m256i d	   = _mm256_and_si256(c, mask);
		__m256i alpha  = _mm256_slli_epi64(_mm256_and_si256(_mm256_add_epi8(d, add), alpha_mask), 3);
		__m256i offset = _mm256_blendv_epi8(_mm256_slli_epi64(add, 3), alpha_offset, alpha);
		__m256i res	   = _mm256_add_epi8(d, offset);

		// Add dashes between blocks as specified in RFC-4122
		// 8-4-4-4-12
		const __m256i dash_shuffle = _mm256_set_epi32(
		  0x0b0a0908, 0x07060504, 0x80030201, 0x00808080, 0x0d0c800b, 0x0a090880, 0x07060504, 0x03020100);
		const __m256i dash =
		  _mm256_set_epi64x(0x0000000000000000ull, 0x2d000000002d0000ull, 0x00002d000000002d, 0x0000000000000000ull);

		__m256i resd = _mm256_shuffle_epi8(res, dash_shuffle);
		resd		 = _mm256_or_si256(resd, dash);

		_mm256_storeu_si256((__m256i*)mem, (resd));
		*(uint16_t*)(mem + 16) = (_mm256_extract_epi16(res, 7));
		*(uint32_t*)(mem + 32) = (_mm256_extract_epi32(res, 7));

		return str;
	}

  private:
	alignas(16) std::byte m_Data[16] = {};
};

inline constexpr uuidv4 invalid_uuidv4 = {};
}	 // namespace psl