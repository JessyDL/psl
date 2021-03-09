#pragma once
#include <psl/details/fixed_ascii_string.hpp>

namespace psl::serialization
{
	template <typename T>
	class anonymous_property;

	template <fixed_ascii_string Name, typename T>
	class property;

	inline namespace details
	{
		template <typename T>
		struct is_anonymous_property : public std::false_type
		{};

		template <typename T>
		struct is_anonymous_property<anonymous_property<T>> : public std::true_type
		{};

		template <typename T>
		struct is_named_property : public std::false_type
		{};

		// workaround for fixed_ascii_string deduction issues on GCC
		template <typename T>
		requires requires(T t)
		{
			typename T::underlying_t;
		}
		&&std::is_base_of_v<anonymous_property<typename T::underlying_t>, T> struct is_named_property<T>
			: public std::true_type
		{};


		template <typename T>
		struct is_named_property<anonymous_property<T>> : public std::false_type
		{};

		template <typename T>
		struct is_property : public std::conditional_t<is_named_property<T>::value || is_anonymous_property<T>::value,
													   std::true_type, std::false_type>
		{};
	} // namespace details

	template <typename T>
	concept IsProperty = is_property<std::remove_cvref_t<T>>::value;

	template <typename T>
	concept IsNamedProperty = is_named_property<std::remove_cvref_t<T>>::value;

	template <typename T>
	concept IsAnonymousProperty = is_anonymous_property<std::remove_cvref_t<T>>::value;

	template <typename T>
	concept SupportsRangeOperations = requires(T t)
	{
		t.begin();
		t.end();
	}
	&&!std::is_same_v<std::remove_cvref_t<T>, std::string>;

	template <typename T>
	concept SupportsSizeOperation = requires(T t)
	{
		t.size();
	};

	template <typename T>
	class anonymous_property
	{
	  public:
		using underlying_t = T;

		template <typename... Args>
		constexpr anonymous_property(Args&&... args) noexcept : m_Value(std::forward<Args>(args)...)
		{}

		template <typename Y>
		constexpr anonymous_property(const anonymous_property<Y>& other) noexcept : m_Value(other.value())
		{}

		template <typename Y>
		constexpr anonymous_property(anonymous_property<Y>&& other) noexcept : m_Value(std::move(other.value()))
		{}

		template <typename Y>
		constexpr auto operator=(const anonymous_property<Y>& other) noexcept -> anonymous_property&
		{
			if(this != &other)
			{
				m_Value = other.value();
			}
			return *this;
		}

		template <typename Y>
		constexpr auto operator=(anonymous_property<Y>&& other) noexcept -> anonymous_property&
		{
			if(this != &other)
			{
				m_Value = std::move(other.value());
			}
			return *this;
		}

		template <typename Y>
		constexpr auto operator=(Y&& other) noexcept
			-> anonymous_property& requires(std::is_assignable_v<underlying_t, Y>)
		{
			m_Value = std::move(other);
			return *this;
		}

		constexpr
		operator std::conditional_t<std::is_trivially_copyable_v<underlying_t>, underlying_t, const underlying_t&>()
			const noexcept
		{
			return m_Value;
		}

		constexpr auto value() & -> underlying_t& { return m_Value; }
		constexpr auto value() const& -> const underlying_t& { return m_Value; }
		constexpr auto value() && -> underlying_t&& { return m_Value; }
		constexpr auto value() const&& -> const underlying_t&& { return m_Value; }

		constexpr explicit operator underlying_t&() noexcept { return m_Value; }

		template <typename Y>
		constexpr auto operator==(const Y& rhs) const noexcept -> bool
		{
			return m_Value == rhs;
		}
		template <typename Y>
		constexpr auto operator!=(const Y& rhs) const noexcept -> bool
		{
			return m_Value != rhs;
		}
		template <typename Y>
		constexpr auto operator<=(const Y& rhs) const noexcept -> bool
		{
			return m_Value <= rhs;
		}
		template <typename Y>
		constexpr auto operator>=(const Y& rhs) const noexcept -> bool
		{
			return m_Value >= rhs;
		}
		template <typename Y>
		constexpr auto operator<(const Y& rhs) const noexcept -> bool
		{
			return m_Value < rhs;
		}
		template <typename Y>
		constexpr auto operator>(const Y& rhs) const noexcept -> bool
		{
			return m_Value > rhs;
		}

		constexpr auto operator++(int) noexcept requires requires(underlying_t t) { t++; }
		{
			return m_Value++;
		}
		constexpr auto operator++() noexcept requires requires(underlying_t t) { ++t; }
		{
			return ++m_Value;
		}

		constexpr auto operator--(int) noexcept requires requires(underlying_t t) { t--; }
		{
			return m_Value--;
		}
		constexpr auto operator--() noexcept requires requires(underlying_t t) { --t; }
		{
			return --m_Value;
		}

		/// ARITHMETIC

		constexpr auto operator+() const noexcept requires requires(underlying_t t) { +t; }
		{
			return +m_Value;
		}
		constexpr auto operator-() const noexcept requires requires(underlying_t t) { -t; }
		{
			return -m_Value;
		}
		constexpr auto operator+(const auto& rhs) const noexcept { return m_Value + rhs; }
		constexpr auto operator-(const auto& rhs) const noexcept { return m_Value - rhs; }
		constexpr auto operator/(const auto& rhs) const noexcept { return m_Value / rhs; }
		constexpr auto operator*(const auto& rhs) const noexcept { return m_Value * rhs; }
		constexpr auto operator%(const auto& rhs) const noexcept { return m_Value % rhs; }
		constexpr auto operator&(const auto& rhs) const noexcept { return m_Value & rhs; }
		constexpr auto operator|(const auto& rhs) const noexcept { return m_Value | rhs; }
		constexpr auto operator~() const noexcept requires requires(underlying_t t) { ~t; }
		{
			return ~m_Value;
		}
		constexpr auto operator^(const auto& rhs) const noexcept { return m_Value ^ rhs; }
		constexpr auto operator<<(const auto& rhs) const noexcept { return m_Value << rhs; }
		constexpr auto operator>>(const auto& rhs) const noexcept { return m_Value >> rhs; }

		/// ASSIGNMENTS

		constexpr auto& operator+=(const auto& rhs) noexcept
		{
			m_Value += rhs;
			return *this;
		}
		constexpr auto& operator-=(const auto& rhs) noexcept
		{
			m_Value -= rhs;
			return *this;
		}
		constexpr auto& operator/=(const auto& rhs) noexcept
		{
			m_Value /= rhs;
			return *this;
		}
		constexpr auto& operator*=(const auto& rhs) noexcept
		{
			m_Value *= rhs;
			return *this;
		}
		constexpr auto& operator%=(const auto& rhs) noexcept
		{
			m_Value %= rhs;
			return *this;
		}
		constexpr auto& operator&=(const auto& rhs) noexcept
		{
			m_Value &= rhs;
			return *this;
		}
		constexpr auto& operator|=(const auto& rhs) noexcept
		{
			m_Value |= rhs;
			return *this;
		}
		constexpr auto& operator^=(const auto& rhs) noexcept
		{
			m_Value ^= rhs;
			return *this;
		}
		constexpr auto& operator<<=(const auto& rhs) noexcept
		{
			m_Value <<= rhs;
			return *this;
		}
		constexpr auto& operator>>=(const auto& rhs) noexcept
		{
			m_Value >>= rhs;
			return *this;
		}

		/// LOGICAL

		constexpr auto operator!() const noexcept -> bool requires requires(underlying_t t)
		{
			{
				!t
			}
			->std::same_as<bool>;
		}
		{
			return !m_Value;
		}

		constexpr auto operator&&(const auto& rhs) const noexcept -> bool { return m_Value && rhs; }
		constexpr auto operator||(const auto& rhs) const noexcept -> bool { return m_Value || rhs; }

		constexpr auto operator-> () noexcept -> underlying_t* { return &m_Value; }
		constexpr auto operator&() noexcept -> underlying_t* { return &m_Value; }
		constexpr auto operator*() const noexcept -> const underlying_t& { return m_Value; }
		constexpr auto operator*() noexcept -> underlying_t& { return m_Value; }

		template <typename Y>
		constexpr auto operator[](Y&& index) noexcept -> decltype(std::declval<underlying_t>()[std::declval<Y>()])
		{
			return m_Value[index];
		}

		constexpr auto begin() noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::begin;
			return begin(m_Value);
		}

		constexpr auto begin() const noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::begin;
			return begin(m_Value);
		}

		constexpr auto cbegin() const noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::cbegin;
			return cbegin(m_Value);
		}

		constexpr auto end() noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::end;
			return end(m_Value);
		}

		constexpr auto end() const noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::end;
			return end(m_Value);
		}

		constexpr auto cend() const noexcept requires(SupportsRangeOperations<underlying_t>)
		{
			using std::cend;
			return cend(m_Value);
		}

		constexpr auto size() const noexcept requires(SupportsSizeOperation<underlying_t>) { return m_Value.size(); }

	  protected:
		underlying_t m_Value;
	};

	template <typename T>
	anonymous_property(T)->anonymous_property<T>;

	template <fixed_ascii_string Name, typename T>
	class property final : public anonymous_property<T>
	{
		using base_t = anonymous_property<T>;

	  public:
		using underlying_t = anonymous_property<T>::underlying_t;

		using base_t::operator==;
		using base_t::operator!=;
		using base_t::operator<=;
		using base_t::operator>=;
		using base_t::operator<;
		using base_t::operator>;
		using anonymous_property<T>::anonymous_property;

		constexpr property() = default;
		constexpr property(const base_t& other) noexcept : base_t(other) {}
		constexpr property(base_t&& other) noexcept : base_t(std::move(other)) {}

		constexpr auto& operator=(IsProperty auto& other) noexcept
		{
			base_t::operator=(other.value());
			return *this;
		}

		constexpr auto& operator=(IsProperty auto&& other) noexcept
		{
			base_t::operator=(std::move(other.value()));
			return *this;
		}

		template <typename Y>
		constexpr auto& operator=(Y&& other) noexcept
		{
			base_t::operator=(std::forward<Y>(other));
			return *this;
		}

		static constexpr std::string_view name() noexcept { return Name; }
	};
} // namespace psl::serialization
