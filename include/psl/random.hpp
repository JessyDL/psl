#pragma one

#include <psl/config.hpp>
#include <psl/exceptions.hpp>

namespace psl {
class xorshift128plus {
  public:
	struct state_t {
		uint64_t first {0xFFFFFFFFFFFFFFFF};
		uint64_t second {0xFFFFFFFFFFFFFFFF};
	};

	[[nodiscard]] constexpr auto operator()() noexcept -> uint64_t {
		uint64_t t		 = m_State.first;
		uint64_t const s = m_State.second;
		m_State.first	 = s;
		t ^= t << 23;
		t ^= t >> 17;
		t ^= s ^ (s >> 26);
		m_State.second = t;
		return t + s;
	}

	constexpr void reseed(state_t state) noexcept { m_State = state; }

	constexpr void reseed(uint64_t first, uint64_t second = 0xFFFFFFFFFFFFFFFF) noexcept {
		m_State = state_t {first, second};
	}

  private:
	state_t m_State {};
};

inline static auto get_default_rng_generator() noexcept -> ::psl::config::default_rng_generator_t {
	static ::psl::config::default_rng_generator_t generator {};
	return generator;
}

template <typename T, typename Rt = T>
constexpr auto random_range(auto& generator,
							T min = std::numeric_limits<T>::min(),
							T max = std::numeric_limits<T>::max()) noexcept -> Rt {
	PSL_EXCEPT_IF(min >= max, std::logic_error, "random_range max needs to be larger than min");
	return (generator() % (max - min)) + min;
}

template <typename T, typename Rt = T>
inline auto random_range(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) noexcept -> Rt {
	PSL_EXCEPT_IF(min >= max, std::logic_error, "random_range max needs to be larger than min");
	return (Rt)((get_default_rng_generator()() % ((uint64_t)max - (uint64_t)min)) + (uint64_t)min);
}
}	 // namespace psl
