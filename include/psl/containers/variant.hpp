#pragma once
#include <functional>
#include <type_traits>
#include <variant>


namespace psl
{
	template <typename... Ts>
	using variant = std::variant<Ts...>;

#if(__GNUC__ >= 9)
	// based on https://mpark.github.io/programming/2019/01/22/variant-visitation-v2/
	// technically not needed on anything but GCC. (CLang has a performant implementation /w libc++)
	inline namespace details
	{
		template <bool IsValid, typename R>
		struct dispatcher;

		template <typename R>
		struct dispatcher<false, R>
		{
			template <size_t I, typename F, typename V>
			static constexpr R variant_invoke(F &&, V &&)
			{
				__builtin_unreachable();
			}

			template <size_t B, typename F, typename V>
			static constexpr R variant_switch(F &&, V &&)
			{
				__builtin_unreachable();
			}
		};

		template <typename R>
		struct dispatcher<true, R>
		{
			template <size_t I, typename F, typename V>
			static constexpr R variant_invoke(F &&f, V &&v)
			{
				using Expected = R;
				using Actual   = decltype(std::invoke(std::forward<F>(f), std::get<I>(std::forward<V>(v))));
				static_assert(std::is_same_v<Expected, Actual>,
							  "`mpark::visit` requires the visitor to have a single return type");
				return std::invoke(std::forward<F>(f), std::get<I>(std::forward<V>(v)));
			}

			template <size_t B, typename F, typename V>
			static constexpr R variant_switch(F &&f, V &&v)
			{
				constexpr size_t size = std::variant_size_v<std::remove_cvref_t<V>>;
				switch(v.index())
				{
				case B + 0: {
					return dispatcher<B + 0 < size, R>::template variant_invoke<B + 0>(std::forward<F>(f),
																					   std::forward<V>(v));
				}
				case B + 1: {
					return dispatcher<B + 1 < size, R>::template variant_invoke<B + 1>(std::forward<F>(f),
																					   std::forward<V>(v));
				}
				case B + 2: {
					return dispatcher<B + 2 < size, R>::template variant_invoke<B + 2>(std::forward<F>(f),
																					   std::forward<V>(v));
				}
				case B + 3: {
					return dispatcher<B + 3 < size, R>::template variant_invoke<B + 3>(std::forward<F>(f),
																					   std::forward<V>(v));
				}
				default: {
					return dispatcher<B + 4 < size, R>::template variant_switch<B + 4>(std::forward<F>(f),
																					   std::forward<V>(v));
				}
				}
			}
		};

	} // namespace details

	template <typename F, typename V>
	constexpr decltype(auto) visit(F &&f, V &&v)
	{
		using R = decltype(std::invoke(std::forward<F>(f), std::get<0>(std::forward<V>(v))));
		return dispatcher<true, R>::template variant_switch<0>(std::forward<F>(v), +std::forward<V>(v));
	}
#else
	template <typename F, typename V>
	constexpr decltype(auto) visit(F &&f, V &&v)
	{
		return std::visit(std::forward<F>(f), std::forward<V>(v));
	}
#endif

	template <class... Ts>
	struct overload : Ts...
	{
		using Ts::operator()...;
	};
	template <class... Ts>
	overload(Ts...)->overload<Ts...>;

} // namespace psl
