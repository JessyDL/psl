#pragma once


class move_only_object
{
  public:
	move_only_object() = default;
	move_only_object(int value) : m_Value(value){};
	move_only_object(const move_only_object&) = delete;
	move_only_object& operator=(const move_only_object&) = delete;

	move_only_object(move_only_object&&) noexcept = default;
	move_only_object& operator=(move_only_object&&) noexcept = default;

	operator int() const noexcept { return m_Value; }

	move_only_object& operator++()
	{
		++m_Value;
		return *this;
	}
	move_only_object operator++(int) = delete;

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }

	int m_Value{0};
};

class copy_only_object
{
	struct disable_move
	{
		disable_move()				 = default;
		disable_move(disable_move&&) = delete;
		disable_move& operator=(disable_move&&) = delete;
	};

  public:
	copy_only_object() noexcept = default;
	copy_only_object(int value) noexcept : m_Value(value) {}
	copy_only_object(const copy_only_object& other) : m_Value(other.m_Value){};
	copy_only_object& operator=(const copy_only_object& other) noexcept
	{
		if(this != &other) m_Value = other.m_Value;
		return *this;
	};

	constexpr operator int() const noexcept { return m_Value; }

	copy_only_object& operator++()
	{
		++m_Value;
		return *this;
	}
	copy_only_object operator++(int)
	{
		auto copy{*this};
		return ++copy;
	};

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }

	int m_Value{0};
	[[no_unique_address]] disable_move beh{};
};

class requires_init
{
  public:
	requires_init() = delete;
	requires_init(int value) noexcept : m_Value(value) {}
	requires_init(const requires_init&) = default;
	requires_init(requires_init&&)		= default;
	requires_init& operator=(const requires_init&) = default;
	requires_init& operator=(requires_init&&) = default;

	operator int() const noexcept { return m_Value; }

	requires_init& operator++()
	{
		++m_Value;
		return *this;
	}
	requires_init operator++(int)
	{
		auto copy{*this};
		return ++copy;
	};

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }
	int m_Value{0};
};
template <bool has_move>
class complex_destruct;

template <typename T>
struct is_complex_destruct : std::false_type
{};
template <bool T>
struct is_complex_destruct<complex_destruct<T>> : std::true_type
{};

template <typename T>
inline constexpr auto is_complex_destruct_v = is_complex_destruct<T>::value;


template <bool has_move = false>
class complex_destruct
{
  public:
	complex_destruct() = delete;
	complex_destruct(int value) noexcept : m_Value(value), m_References(new int(1)) {}
	complex_destruct(const complex_destruct& other) : m_Value(other.m_Value), m_References(other.m_References)
	{
		if(m_References) ++(*m_References);
	}
	complex_destruct& operator=(const complex_destruct& other)
	{
		if(this != &other)
		{
			m_Value		 = other.m_Value;
			m_References = other.m_References;
			if(m_References) ++(*m_References);
		}
		return *this;
	}
	complex_destruct(complex_destruct&& other) requires(has_move)
		: m_Value(other.m_Value), m_References(other.m_References)
	{
		other.m_References = nullptr;
	}
	complex_destruct& operator=(complex_destruct&& other) requires(has_move)
	{
		if(this != &other)
		{
			m_Value			   = other.m_Value;
			m_References	   = other.m_References;
			other.m_References = nullptr;
		}
		return *this;
	}
	~complex_destruct()
	{
		if(m_References)
		{
			--(*m_References);
			if(*m_References == 0)
			{
				delete(m_References);
			}
		}
	}

	operator int() const noexcept { return m_Value; }

	complex_destruct& operator++()
	{
		++m_Value;
		return *this;
	}
	complex_destruct operator++(int)
	{
		auto copy{*this};
		return ++copy;
	};

	int references() const noexcept { return *m_References; }

	bool operator==(int rhs) const noexcept { return m_Value == rhs; }
	int m_Value{};
	int* m_References{nullptr};
};
