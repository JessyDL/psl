#pragma once
#include <array>
#include <cstdint>
#include <psl/config.h>
// defines used locally for defining various accessor functions such as .x() or .zwxy(), etc..
// these are cleaned up at the end of the file to avoid global pollution.
#include <psl/details/vec_macro.inl>

namespace psl
{

	template <typename T, size_t N>
	class tvec;

	inline namespace details
	{
		template <typename T>
		struct IMPL_is_tvec : std::false_type
		{};
		template <typename T, size_t N>
		struct IMPL_is_tvec<tvec<T, N>> : std::true_type
		{};


		template <typename T>
		struct is_tvec : IMPL_is_tvec<std::remove_cvref_t<T>>
		{};

		template <typename T>
		struct IMPL_is_tvec_like : public is_tvec<T>
		{};

		template <typename T, size_t N>
		struct IMPL_is_tvec_like<std::array<T, N>> : std::true_type
		{};

		template <typename T>
		struct is_tvec_like : public IMPL_is_tvec_like<std::remove_cvref_t<T>>
		{};

		template <typename T>
		concept IsTvec = is_tvec<T>::value;
		template <typename T>
		concept IsTvecLike = is_tvec_like<T>::value;

		template <typename T>
		struct is_tvec_accessor : std::false_type
		{};

		template <typename T, size_t N>
		struct is_tvec_accessor<tvec<T, N>>
			: std::conditional_t<std::is_reference_v<T>, std::true_type, std::false_type>
		{};

		template <typename T>
		static constexpr auto is_tvec_accessor_v = is_tvec_accessor<T>::value;

		template <typename T>
		struct get_size
		{};

		template <typename T, size_t N>
		struct get_size<tvec<T, N>>
		{
			static constexpr size_t size = N;
		};
		template <typename T, size_t N>
		struct get_size<std::array<T, N>>
		{
			static constexpr size_t size = N;
		};
		// helper functions to transform or do operations on TvecLikes

		template <typename Fn, size_t... Is>
		constexpr bool all_of_impl(Fn&& fn, IsTvecLike auto const& v0, IsTvecLike auto const& v1,
								   std::index_sequence<Is...>) noexcept
		{
			return (fn(v0.template get<Is>(), v1.template get<Is>()) && ...);
		}

		template <typename Fn>
		constexpr bool all_of(Fn&& fn, IsTvecLike auto const& v0, IsTvecLike auto const& v1) noexcept
		{
			return all_of_impl(std::forward<Fn>(fn), v0, v1,
							   std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}


		template <typename Fn, size_t... Is>
		constexpr auto for_each_impl(Fn&& fn, IsTvecLike auto const& v0, IsTvecLike auto const& v1,
									 std::index_sequence<Is...>) noexcept
		{
			return tvec{fn(v0[Is], v1[Is])...};
		}

		template <typename Fn, size_t... Is>
		constexpr auto for_each_impl(Fn&& fn, IsTvecLike auto const& v0, std::index_sequence<Is...>) noexcept
		{
			return tvec{fn(v0[Is])...};
		}

		template <typename Fn>
		constexpr auto for_each(Fn&& fn, IsTvecLike auto const& v0, IsTvecLike auto const& v1) noexcept
		{
			return for_each_impl(std::forward<Fn>(fn), v0, v1,
								 std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}

		template <typename Fn>
		constexpr auto for_each(Fn&& fn, IsTvecLike auto const& v0) noexcept
		{
			return for_each_impl(std::forward<Fn>(fn), v0,
								 std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}

		template <typename Fn, size_t... Is>
		constexpr auto& for_each_inplace_impl(Fn&& fn, IsTvecLike auto& v0, IsTvecLike auto const& v1,
											  std::index_sequence<Is...>) noexcept
		{
			(fn(v0[Is], v1[Is]), ...);
			return v0;
		}

		template <typename Fn>
		constexpr auto& for_each_inplace(Fn&& fn, IsTvecLike auto& v0, IsTvecLike auto const& v1) noexcept
		{
			return for_each_inplace_impl(std::forward<Fn>(fn), v0, v1,
										 std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}


		template <typename Fn, size_t... Is>
		constexpr auto& for_each_inplace_impl(Fn&& fn, IsTvecLike auto& v0, std::index_sequence<Is...>) noexcept
		{
			(fn(v0[Is]), ...);
			return v0;
		}

		template <typename Fn>
		constexpr auto& for_each_inplace(Fn&& fn, IsTvecLike auto& v0) noexcept
		{
			return for_each_inplace_impl(std::forward<Fn>(fn), v0,
										 std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}

		template <typename Fn1, size_t... Is>
		constexpr auto accumulate_impl(Fn1&& combinator, IsTvecLike auto const& v0, std::index_sequence<Is...>) noexcept
		{
			auto result{v0[0]};
			(void(result = combinator(result, v0[Is + 1])), ...);
			return result;
		}


		template <typename Fn1>
		constexpr auto accumulate(Fn1&& combinator, IsTvecLike auto const& v0) noexcept
		{
			return accumulate_impl(std::forward<Fn1>(combinator), v0,
								   std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size - 1>{});
		}

		template <typename Fn1, typename Fn2, size_t... Is>
		constexpr auto accumulate_transform_impl(Fn1&& combinator, Fn2&& tranformer, IsTvecLike auto const& v0,
												 std::index_sequence<Is...>) noexcept
		{
			auto result{tranformer(v0[0])};
			(void(result = combinator(result, tranformer(v0[Is + 1]))), ...);
			return result;
		}


		template <typename Fn1, typename Fn2, size_t... Is>
		constexpr auto accumulate_transform_impl(Fn1&& combinator, Fn2&& tranformer, IsTvecLike auto const& v0,
												 IsTvecLike auto const& v1, std::index_sequence<Is...>) noexcept
		{
			auto result{tranformer(v0[0], v1[0])};
			(void(result = combinator(result, tranformer(v0[Is + 1], v1[Is + 1]))), ...);
			return result;
		}

		template <typename Fn1, typename Fn2>
		constexpr auto accumulate_transform(Fn1&& combinator, Fn2&& transform, IsTvecLike auto const& v0,
											IsTvecLike auto const& v1) noexcept
		{
			return accumulate_transform_impl(
				std::forward<Fn1>(combinator), std::forward<Fn2>(transform), v0, v1,
				std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size - 1>{});
		}
		template <typename Fn1, typename Fn2>
		constexpr auto accumulate_transform(Fn1&& combinator, Fn2&& transform, IsTvecLike auto const& v0) noexcept
		{
			return accumulate_transform_impl(
				std::forward<Fn1>(combinator), std::forward<Fn2>(transform), v0,
				std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size - 1>{});
		}

		template <typename Fn, size_t... Is>
		constexpr auto all_of_impl(IsTvecLike auto const& v0, IsTvecLike auto const& v1, Fn&& fn,
								   std::index_sequence<Is...>)
		{
			bool result{true};
			(void(result = !result || fn(v0[Is], v1[Is])), ...);
			return result;
		}

		template <typename Fn>
		constexpr auto all_of(IsTvecLike auto const& v0, IsTvecLike auto const& v1, Fn&& fn)
		{
			return all_of_impl(v0, v1, std::forward<Fn>(fn),
							   std::make_index_sequence<get_size<std::remove_cvref_t<decltype(v0)>>::size>{});
		}

		template <typename T, size_t N>
		class tvec_predefined
		{};
		template <typename T>
		class tvec_predefined<T, 1>
		{
		  public:
			static constexpr tvec<T, 1> zero{T{0}};
			static constexpr tvec<T, 1> one{T{1}};
		};
		template <typename T>
		class tvec_predefined<T, 2>
		{
		  public:
			static constexpr tvec<T, 2> zero{T{0}};
			static constexpr tvec<T, 2> one{T{1}};
			static constexpr tvec<T, 2> up{T{0}, T{1}};
			static constexpr tvec<T, 2> down{T{0}, -T{1}};
			static constexpr tvec<T, 2> right{T{1}, T{0}};
			static constexpr tvec<T, 2> left{-T{1}, T{0}};
		};
		template <typename T>
		class tvec_predefined<T, 3>
		{
		  public:
			static constexpr tvec<T, 3> zero{T{0}};
			static constexpr tvec<T, 3> one{T{1}};
			static constexpr tvec<T, 3> forward{T{0}, T{0}, T{1}};
			static constexpr tvec<T, 3> back{T{0}, T{0}, -T{1}};
			static constexpr tvec<T, 3> up{T{0}, T{1}, T{0}};
			static constexpr tvec<T, 3> down{T{0}, -T{1}, T{0}};
			static constexpr tvec<T, 3> right{T{1}, T{0}, T{0}};
			static constexpr tvec<T, 3> left{-T{1}, T{0}, T{0}};
		};
		template <typename T>
		class tvec_predefined<T, 4>
		{
		  public:
			static constexpr tvec<T, 4> zero{T{0}};
			static constexpr tvec<T, 4> one{T{1}};
			static constexpr tvec<T, 4> forward{T{0}, T{0}, T{1}, T{0}};
			static constexpr tvec<T, 4> back{T{0}, T{0}, -T{1}, T{0}};
			static constexpr tvec<T, 4> up{T{0}, T{1}, T{0}, T{0}};
			static constexpr tvec<T, 4> down{T{0}, -T{1}, T{0}, T{0}};
			static constexpr tvec<T, 4> right{T{1}, T{0}, T{0}, T{0}};
			static constexpr tvec<T, 4> left{-T{1}, T{0}, T{0}, T{0}};
			static constexpr tvec<T, 4> point{T{0}, T{0}, T{0}, T{1}};
		};
	} // namespace details

	/**
	 * \brief Standard layout wrapper over std::array<T,N> that encapsulates and facilitates math operations
	 *
	 * \tparam T
	 * \tparam N
	 */
	template <typename T, size_t N>
	class tvec : public tvec_predefined<std::remove_cvref_t<T>, N>
	{
	  public:
		static constexpr auto DIMENSIONS = N;
		using base_type					 = std::remove_const_t<T>;
		using value_type				 = T;
		using reference					 = T&;
		using const_reference			 = T const&;
		using pointer					 = T*;
		using const_pointer				 = T const*;
		using size_type					 = size_t;

		using type				  = tvec<base_type, N>;
		using const_type		  = tvec<const base_type, N>;
		using accessor_type		  = tvec<base_type&, N>;
		using const_accessor_type = tvec<base_type const&, N>;
		using container_type	  = std::array<base_type, N>;

		static_assert(std::is_same_v<base_type, std::remove_cvref_t<T>>,
					  "type cannot be anything other than a base type, i.e. no 'float *', but 'float'");
		// ------------------------------------------------------------------------------------------------------------
		// constructors
		// ------------------------------------------------------------------------------------------------------------

		template <typename... Ys>
		requires(sizeof...(Ys) ==
				 N) constexpr tvec(Ys&&... values) noexcept(std::is_nothrow_constructible_v<container_type, Ys...>)
			: m_Data({std::forward<Ys>(values)...})
		{}

		template <typename Y>
		requires(std::is_constructible_v<T, Y> && !IsTvecLike<Y>) constexpr tvec(Y&& value) noexcept
		{
			for_each_inplace([&value](auto& lhs) { lhs = value; }, m_Data);
		}

		template <typename Y>
		constexpr tvec(IsTvecLike auto const& vec) noexcept
		{
			for_each_inplace([](auto& lhs, auto rhs) { lhs = rhs; }, m_Data, vec);
		}

		constexpr tvec& operator=(IsTvecLike auto const& other)
		{
			if(&other != this)
			{
				for_each_inplace([](auto& element, auto value) { element = value; }, m_Data, other);
			}
			return *this;
		}

		// ------------------------------------------------------------------------------------------------------------
		// conversion
		// ------------------------------------------------------------------------------------------------------------
		operator container_type&() noexcept { return m_Data; }
		operator const container_type&() const noexcept { return m_Data; }

	  private:
		template <typename Y, size_t... Is0, size_t... Is1>
		tvec<Y, sizeof...(Is0) + sizeof...(Is1)> resize_impl(std::index_sequence<Is0...>,
															 std::index_sequence<Is1...>) const noexcept
		{
			return tvec<Y, sizeof...(Is0) + sizeof...(Is1)>(this->m_Data[Is0]..., T{Is1 * 0}...);
		}

	  public:
		template <size_t NewSize, typename Y = base_type>
		[[nodiscard]] tvec<Y, NewSize> resize() const noexcept requires(N > NewSize)
		{
			return resize_impl<Y>(std::make_index_sequence<NewSize>{}, std::make_index_sequence<0>{});
		}

		template <size_t NewSize, typename Y = base_type>
		[[nodiscard]] tvec<Y, NewSize> resize() const noexcept requires(N <= NewSize)
		{
			return resize_impl<Y>(std::make_index_sequence<N>{}, std::make_index_sequence<NewSize - N>{});
		}

		// ------------------------------------------------------------------------------------------------------------
		// element accessors
		// ------------------------------------------------------------------------------------------------------------

		constexpr reference operator[](size_type index) noexcept { return m_Data[index]; }
		constexpr const_reference operator[](size_type index) const noexcept { return m_Data[index]; }
		template <size_t I>
		constexpr reference get() noexcept requires(I < N)
		{
			return std::get<I>(m_Data);
		}
		template <size_t I>
		constexpr const_reference get() const noexcept requires(I < N)
		{
			return std::get<I>(m_Data);
		}

		ACCESSOR_1()
		ACCESSOR_2()
		ACCESSOR_3()
		ACCESSOR_4()

		/**
		 * \brief Returns the dimensions of the vector type.
		 *
		 * \return constexpr size_type
		 */
		constexpr size_type size() const noexcept { return N; }

	  private:
		container_type m_Data;
	};

	template <typename T, typename... Ts>
	requires(std::conjunction_v<std::is_same<T, Ts>...>) tvec(T&&, Ts&&...)->tvec<T, sizeof...(Ts) + 1>;


	/**
	 * \brief Accessor variation of tvec.
	 *
	 * \tparam T
	 * \tparam N
	 * \warning Should never be created manually by the user. This type is created from existing tvec's only.
	 * \note Certain operations are not permitted, one of which is assignment from other accessors. You will have to
	 * construct temporaries to circumvent this. The reason for this is that if an accessor does self-assignment, the
	 * results can become counter-intuitive fast, which is undesireable in math heavy contexts.
	 */
	template <typename T, size_t N>
	class tvec<T&, N>
	{
	  public:
		static constexpr auto DIMENSIONS = N;
		using base_type					 = std::remove_const_t<T>;
		using value_type				 = T;
		using reference					 = base_type&;
		using const_reference			 = base_type const&;
		using pointer					 = base_type*;
		using const_pointer				 = base_type const*;
		using size_type					 = size_t;

		using type				  = tvec<base_type, N>;
		using const_type		  = tvec<const base_type, N>;
		using accessor_type		  = tvec<base_type&, N>;
		using const_accessor_type = tvec<base_type const&, N>;
		using container_type	  = std::array<base_type*, N>;

		template <typename Y, size_t Nx>
		friend class tvec;
		// ------------------------------------------------------------------------------------------------------------
		// constructors
		// ------------------------------------------------------------------------------------------------------------
		constexpr tvec()				  = delete;
		constexpr tvec(const tvec& other) = delete;
		constexpr tvec(tvec&& other)	  = delete;
		constexpr tvec& operator=(const tvec& other) = delete;
		constexpr tvec& operator=(tvec&& other) = delete;

	  private:
		template <size_t... Is>
		constexpr tvec(const auto& vec, std::index_sequence<Is...>) noexcept
			: m_Data({const_cast<base_type*>(&vec[Is])...})
		{}

	  public:
		constexpr tvec& operator=(const auto& other) noexcept requires(!std::is_const_v<T>)
		{
			if((void*)this != (void*)&other)
			{
				for_each_inplace([](auto lhs, auto rhs) { *lhs = rhs; }, m_Data, other);
			}
			return *this;
		}

		constexpr operator type() const noexcept
		{
			return for_each([](auto value) { return *value; }, m_Data);
		}


		// ------------------------------------------------------------------------------------------------------------
		// element accessors
		// ------------------------------------------------------------------------------------------------------------

		constexpr reference operator[](size_type index) noexcept { return *m_Data[index]; }
		constexpr const_reference operator[](size_type index) const noexcept { return *m_Data[index]; }
		template <size_t I>
		constexpr reference get() noexcept requires(I < N)
		{
			return *std::get<I>(m_Data);
		}
		template <size_t I>
		constexpr const_reference get() const noexcept requires(I < N)
		{
			return *std::get<I>(m_Data);
		}

	  private:
		container_type m_Data;
	};
	// ----------------------------------------------------------------------------------------------------------------
	// logical operators
	// ----------------------------------------------------------------------------------------------------------------
	constexpr bool operator==(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		return all_of([](auto lhs, auto rhs) { return lhs == rhs; }, lhs, rhs);
	}
	constexpr bool operator!=(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		return all_of([](auto lhs, auto rhs) { return lhs != rhs; }, lhs, rhs);
	}
	// ----------------------------------------------------------------------------------------------------------------
	// arithmic operators
	// ----------------------------------------------------------------------------------------------------------------

	constexpr auto& operator+=(IsTvecLike auto& lhs, IsTvecLike auto const& rhs) noexcept
	{
		for_each_inplace([](auto& lhs, const auto& rhs) { lhs += rhs; }, lhs, rhs);
		return lhs;
	}
	constexpr auto operator+(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		typename std::remove_cvref_t<decltype(lhs)>::type copy = lhs;
		return copy += rhs;
	}

	constexpr auto& operator-=(IsTvecLike auto& lhs, IsTvecLike auto const& rhs) noexcept
	{
		for_each_inplace([](auto& lhs, const auto& rhs) { lhs -= rhs; }, lhs, rhs);
		return lhs;
	}
	constexpr auto operator-(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		typename std::remove_cvref_t<decltype(lhs)>::type copy = lhs;
		return copy -= rhs;
	}

	constexpr auto& operator*=(IsTvecLike auto& lhs, IsTvecLike auto const& rhs) noexcept
	{
		for_each_inplace([](auto& lhs, const auto& rhs) { lhs *= rhs; }, lhs, rhs);
		return lhs;
	}
	constexpr auto operator*(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		typename std::remove_cvref_t<decltype(lhs)>::type copy = lhs;
		return copy *= rhs;
	}

	constexpr auto& operator/=(IsTvecLike auto& lhs, IsTvecLike auto const& rhs) noexcept
	{
		for_each_inplace([](auto& lhs, const auto& rhs) { lhs /= rhs; }, lhs, rhs);
		return lhs;
	}
	constexpr auto operator/(IsTvecLike auto const& lhs, IsTvecLike auto const& rhs) noexcept
	{
		typename std::remove_cvref_t<decltype(lhs)>::type copy = lhs;
		return copy /= rhs;
	}

	constexpr auto operator-(IsTvecLike auto const& lhs) noexcept
	{
		typename std::remove_cvref_t<decltype(lhs)>::type copy = lhs;
		for_each_inplace([](auto& element) { element = -element; }, copy);
		return copy;
	}
} // namespace psl


namespace psl
{
	namespace config
	{
		template <typename T = default_setting_t>
		using tvec_default_type = float;
	}
	// predefined variations
	template <size_t N>
	using vec  = tvec<typename config::tvec_default_type<config::default_setting_t>, N>;
	using vec1 = vec<1>;
	using vec2 = vec<2>;
	using vec3 = vec<3>;
	using vec4 = vec<4>;

	template <size_t N>
	using fvec  = tvec<float, N>;
	using fvec1 = fvec<1>;
	using fvec2 = fvec<2>;
	using fvec3 = fvec<3>;
	using fvec4 = fvec<4>;

	template <size_t N>
	using dvec  = tvec<double, N>;
	using dvec1 = dvec<1>;
	using dvec2 = dvec<2>;
	using dvec3 = dvec<3>;
	using dvec4 = dvec<4>;

	template <size_t N>
	using ivec  = tvec<int, N>;
	using ivec1 = ivec<1>;
	using ivec2 = ivec<2>;
	using ivec3 = ivec<3>;
	using ivec4 = ivec<4>;

	template <size_t N>
	using i8vec  = tvec<int8_t, N>;
	using i8vec1 = i8vec<1>;
	using i8vec2 = i8vec<2>;
	using i8vec3 = i8vec<3>;
	using i8vec4 = i8vec<4>;

	template <size_t N>
	using i16vec  = tvec<int16_t, N>;
	using i16vec1 = i16vec<1>;
	using i16vec2 = i16vec<2>;
	using i16vec3 = i16vec<3>;
	using i16vec4 = i16vec<4>;

	template <size_t N>
	using i32vec  = tvec<int32_t, N>;
	using i32vec1 = i32vec<1>;
	using i32vec2 = i32vec<2>;
	using i32vec3 = i32vec<3>;
	using i32vec4 = i32vec<4>;

	template <size_t N>
	using i64vec  = tvec<int64_t, N>;
	using i64vec1 = i64vec<1>;
	using i64vec2 = i64vec<2>;
	using i64vec3 = i64vec<3>;
	using i64vec4 = i64vec<4>;

	template <size_t N>
	using uvec  = tvec<unsigned int, N>;
	using uvec1 = uvec<1>;
	using uvec2 = uvec<2>;
	using uvec3 = uvec<3>;
	using uvec4 = uvec<4>;

	template <size_t N>
	using ui8vec  = tvec<uint8_t, N>;
	using ui8vec1 = ui8vec<1>;
	using ui8vec2 = ui8vec<2>;
	using ui8vec3 = ui8vec<3>;
	using ui8vec4 = ui8vec<4>;

	template <size_t N>
	using ui16vec  = tvec<uint16_t, N>;
	using ui16vec1 = ui16vec<1>;
	using ui16vec2 = ui16vec<2>;
	using ui16vec3 = ui16vec<3>;
	using ui16vec4 = ui16vec<4>;

	template <size_t N>
	using ui32vec  = tvec<uint32_t, N>;
	using ui32vec1 = ui32vec<1>;
	using ui32vec2 = ui32vec<2>;
	using ui32vec3 = ui32vec<3>;
	using ui32vec4 = ui32vec<4>;

	template <size_t N>
	using ui64vec  = tvec<uint64_t, N>;
	using ui64vec1 = ui64vec<1>;
	using ui64vec2 = ui64vec<2>;
	using ui64vec3 = ui64vec<3>;
	using ui64vec4 = ui64vec<4>;

	template <size_t N>
	using szvec  = tvec<size_t, N>;
	using szvec1 = szvec<1>;
	using szvec2 = szvec<2>;
	using szvec3 = szvec<3>;
	using szvec4 = szvec<4>;

	// explicit instantiations
	// floating point types
	extern template class tvec<float, 1>;
	extern template class tvec<float, 2>;
	extern template class tvec<float, 3>;
	extern template class tvec<float, 4>;

	extern template class tvec<double, 1>;
	extern template class tvec<double, 2>;
	extern template class tvec<double, 3>;
	extern template class tvec<double, 4>;

	// signed ints
	extern template class tvec<int8_t, 1>;
	extern template class tvec<int8_t, 2>;
	extern template class tvec<int8_t, 3>;
	extern template class tvec<int8_t, 4>;

	extern template class tvec<int16_t, 1>;
	extern template class tvec<int16_t, 2>;
	extern template class tvec<int16_t, 3>;
	extern template class tvec<int16_t, 4>;

	extern template class tvec<int32_t, 1>;
	extern template class tvec<int32_t, 2>;
	extern template class tvec<int32_t, 3>;
	extern template class tvec<int32_t, 4>;

	extern template class tvec<int64_t, 1>;
	extern template class tvec<int64_t, 2>;
	extern template class tvec<int64_t, 3>;
	extern template class tvec<int64_t, 4>;

	// unsigned signed ints
	extern template class tvec<uint8_t, 1>;
	extern template class tvec<uint8_t, 2>;
	extern template class tvec<uint8_t, 3>;
	extern template class tvec<uint8_t, 4>;

	extern template class tvec<uint16_t, 1>;
	extern template class tvec<uint16_t, 2>;
	extern template class tvec<uint16_t, 3>;
	extern template class tvec<uint16_t, 4>;

	extern template class tvec<uint32_t, 1>;
	extern template class tvec<uint32_t, 2>;
	extern template class tvec<uint32_t, 3>;
	extern template class tvec<uint32_t, 4>;

	extern template class tvec<uint64_t, 1>;
	extern template class tvec<uint64_t, 2>;
	extern template class tvec<uint64_t, 3>;
	extern template class tvec<uint64_t, 4>;
} // namespace psl

// cleanup

#include <psl/details/vec_macro_cleanup.inl>

// mathematics

#if !defined(PSL_VEC_ONLY)
#include <cmath>
#include <psl/concepts.h>

namespace psl
{
	inline namespace details
	{
		struct plus
		{
			constexpr auto operator()(const auto& lhs, const auto& rhs) const noexcept { return lhs + rhs; }
		};
	} // namespace details

	template <typename FnCombinator, typename FnTransformer, typename T, size_t N>
	[[nodiscard]] constexpr T compound(const tvec<T, N>& value, FnCombinator&& combinator,
									   FnTransformer&& transformer) noexcept
	{
		return accumulate_transform(std::forward<FnCombinator>(combinator), std::forward<FnTransformer>(transformer),
									value);
	}

	template <typename FnCombinator, typename T, size_t N>
	[[nodiscard]] constexpr T compound(const tvec<T, N>& value, FnCombinator&& combinator) noexcept
	{
		return accumulate(std::forward<FnCombinator>(combinator), value);
	}

	template <typename T, size_t N>
	[[nodiscard]] constexpr T compound(const tvec<T, N>& value) noexcept
	{
		return accumulate(plus{}, value);
	}

	template <typename T, size_t N>
	[[nodiscard]] constexpr T square_magnitude(const tvec<T, N>& value) noexcept
	{
		return accumulate_transform(plus{}, [](auto value) noexcept { return value * value; }, value);
	}

	template <typename T, size_t N>
	[[nodiscard]] constexpr auto magnitude(const tvec<T, N>& value) noexcept
	{
		using std::sqrt;
		return sqrt(square_magnitude(value));
	}


	template <typename T, size_t N>
	[[nodiscard]] constexpr T dot(const tvec<T, N>& v0, const tvec<T, N>& v1) noexcept
	{
		return accumulate_transform(plus{}, [](auto e0, auto e1) noexcept { return (e0 * e1); }, v0, v1);
	}

	template <typename T>
	[[nodiscard]] constexpr tvec<T, 3> cross(const tvec<T, 3>& v0, const tvec<T, 3>& v1) noexcept
	{
		return {(v0.y() * v1.z()) - (v0.z() * v1.y()), (v0.z() * v1.x()) - (v0.x() * v1.z()),
				(v0.x() * v1.y()) - (v0.y() * v1.x())};
	}

	template <typename T>
	[[nodiscard]] constexpr T cross(const tvec<T, 2>& v0, const tvec<T, 2>& v1) noexcept
	{
		return (v0.x() * v1.y()) - (v0.y() * v1.x());
	}

	template <IsTvec... Ts>
	constexpr auto& max_inplace(IsTvec auto& v0, IsTvec auto const& v1, const Ts&... values) noexcept
	{
		using std::max;
		if constexpr(sizeof...(Ts) == 0)
			return for_each_inplace([](auto& e0, auto e1) { e0 = max(e0, e1); }, v0, v1);
		else
			return psl::max_inplace(for_each_inplace([](auto& e0, auto e1) { e0 = max(e0, e1); }, v0, v1), values...);
	}

	template <IsTvec... Ts>
	[[nodiscard]] constexpr auto max(IsTvec auto const& v0, IsTvec auto const& v1, const Ts&... values) noexcept
	{
		using std::max;
		if constexpr(sizeof...(Ts) == 0)
			return for_each([](auto e0, auto e1) { return max(e0, e1); }, v0, v1);
		else
			return psl::max(for_each([](auto e0, auto e1) { return max(e0, e1); }, v0, v1), values...);
	}

	template <IsTvec... Ts>
	constexpr auto& min_inplace(IsTvec auto& v0, IsTvec auto const& v1, const Ts&... values) noexcept
	{
		using std::min;
		if constexpr(sizeof...(Ts) == 0)
			return for_each_inplace([](auto& e0, auto e1) { e0 = min(e0, e1); }, v0, v1);
		else
			return psl::min_inplace(for_each_inplace([](auto& e0, auto e1) { e0 = min(e0, e1); }, v0, v1), values...);
	}

	template <IsTvec... Ts>
	[[nodiscard]] constexpr auto min(IsTvec auto const& v0, IsTvec auto const& v1, const Ts&... values) noexcept
	{
		using std::min;
		if constexpr(sizeof...(Ts) == 0)
			return for_each([](auto e0, auto e1) { return min(e0, e1); }, v0, v1);
		else
			return psl::min(for_each([](auto e0, auto e1) { return min(e0, e1); }, v0, v1), values...);
	}

	/**
	 * \brief Constructs a new tvec that applies abs() on every component.
	 *
	 * \param v0 tvec to modify
	 * \return constexpr auto
	 */
	[[nodiscard]] constexpr auto abs(IsTvec auto const& v0) noexcept
	{
		using std::abs;
		return for_each([](auto e0) { return abs(e0); }, v0);
	}
	/**
	 * \brief Inplace modifies every component of the vector to the abs() value.
	 *
	 * \param v0
	 * \return constexpr auto&
	 */
	constexpr auto& abs_inplace(IsTvec auto& v0) noexcept
	{
		using std::abs;
		return for_each_inplace([](auto& e0) { return abs(e0); }, v0);
	}

	/**
	 * \brief Constructs a new vector that is scaled by the given amount.
	 *
	 * \param vec tvec to scale
	 * \param value scale amount
	 * \return constexpr tvec
	 */
	[[nodiscard]] constexpr auto scale(IsTvecLike auto const& vec, IsArithmetic auto value) noexcept
	{
		return for_each([&value](auto element) { return element * value; }, vec);
	}

	/**
	 * \brief Scales the given vector by the value inplace.
	 *
	 * \param vec tvec to scale
	 * \param value scale amount
	 * \return constexpr tvec&
	 */
	constexpr auto& scale_inplace(IsTvecLike auto& vec, IsArithmetic auto value) noexcept
	{
		return for_each_inplace([&value](auto& element) { element *= value; }, vec);
	}
} // namespace psl

#endif