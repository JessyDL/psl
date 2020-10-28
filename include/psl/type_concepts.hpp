#pragma once

namespace psl
{
	template <typename T>
	concept IsIterator = requires
	{
		T::iterator_category;
	};

	template <typename T>
	concept IsIntegral = std::is_integral_v<T>;

	template <typename T>
	concept IsSignedIntegral = IsIntegral<T>&& std::is_signed_v<T>;

	template <typename T>
	concept IsUnsignedIntegral = IsIntegral<T>&& std::is_unsigned_v<T>;

	template <typename T>
	concept IsArithmetic = std::is_arithmetic_v<T>;

	template <typename T>
	concept IsRange = requires(T t)
	{
		t.begin();
		t.end();
		{
			t.size()
		}
		->IsIntegral;
	};

	template <typename T>
	concept MoveAssignable = std::is_move_assignable_v<T>;
	template <typename T>
	concept CopyAssignable = std::is_copy_assignable_v<T>;
	template <typename T>
	concept MoveCopyAssignable = MoveAssignable<T> || CopyAssignable<T>;
	template <typename T>
	concept NothrowMoveAssignable = std::is_nothrow_move_assignable_v<T>;
	template <typename T>
	concept NothrowCopyAssignable = std::is_nothrow_copy_assignable_v<T>;
	template <typename T>
	concept NothrowMoveCopyAssignable = NothrowMoveAssignable<T> || NothrowCopyAssignable<T>;

	template <typename T>
	concept MoveConstructible = std::is_move_constructible_v<T>;
	template <typename T>
	concept CopyConstructible = std::is_copy_constructible_v<T>;
	template <typename T>
	concept MoveCopyConstructible = MoveConstructible<T> || CopyConstructible<T>;
	template <typename T>
	concept NothrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;
	template <typename T>
	concept NothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;
	template <typename T>
	concept NothrowMoveCopyConstructible = NothrowMoveConstructible<T> || NothrowCopyConstructible<T>;
} // namespace psl