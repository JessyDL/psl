#pragma once
#include <array>
#include <cstdint>
// defines used locally for defining various accessor functions such as .x() or .zwxy(), etc..
// these are cleaned up at the end of the file to avoid global pollution.
#define ACCESSOR_1_FN(name, index)                                                                                     \
	constexpr auto name() const noexcept requires(index < Size) { return get<index>(); }

#define ACCESSOR_2_FN(name, index0, index1)                                                                            \
	constexpr auto name() const noexcept requires(index0 < Size && index1 < Size)                                      \
	{                                                                                                                  \
		return tvec<const_reference, 2>{get<index0>(), get<index1>()};                                                 \
	}


#define ACCESSOR_3_FN(name, index0, index1, index2)                                                                    \
	constexpr auto name() const noexcept requires(index0 < Size && index1 < Size && index2 < Size)                     \
	{                                                                                                                  \
		return tvec<const_reference, 3>{get<index0>(), get<index1>(), get<index2>()};                                  \
	}

#define ACCESSOR_4_FN(name, index0, index1, index2, index3)                                                            \
	constexpr auto name() const noexcept requires(index0 < Size && index1 < Size && index2 < Size && index3 < Size)    \
	{                                                                                                                  \
		return tvec<const_reference, 4>{get<index0>(), get<index1>(), get<index2>(), get<index3>()};                   \
	}

#define ACCESSOR_1() ACCESSOR_1_FN(x, 0)

#define ACCESSOR_2()                                                                                                   \
	ACCESSOR_1_FN(y, 1)                                                                                                \
	ACCESSOR_2_FN(xx, 0, 0)                                                                                            \
	ACCESSOR_2_FN(xy, 0, 1)                                                                                            \
	ACCESSOR_2_FN(yx, 1, 0)                                                                                            \
	ACCESSOR_2_FN(yy, 1, 1)

#define ACCESSOR_3()                                                                                                   \
	ACCESSOR_1_FN(z, 2)                                                                                                \
	ACCESSOR_2_FN(xz, 0, 2)                                                                                            \
	ACCESSOR_2_FN(yz, 1, 2)                                                                                            \
	ACCESSOR_2_FN(zx, 2, 0)                                                                                            \
	ACCESSOR_2_FN(zy, 2, 1)                                                                                            \
	ACCESSOR_2_FN(zz, 2, 2)                                                                                            \
	ACCESSOR_3_FN(xxx, 0, 0, 0)                                                                                        \
	ACCESSOR_3_FN(xxy, 0, 0, 1)                                                                                        \
	ACCESSOR_3_FN(xxz, 0, 0, 2)                                                                                        \
	ACCESSOR_3_FN(xyx, 0, 1, 0)                                                                                        \
	ACCESSOR_3_FN(xyy, 0, 1, 1)                                                                                        \
	ACCESSOR_3_FN(xyz, 0, 1, 2)                                                                                        \
	ACCESSOR_3_FN(xzx, 0, 2, 0)                                                                                        \
	ACCESSOR_3_FN(xzy, 0, 2, 1)                                                                                        \
	ACCESSOR_3_FN(xzz, 0, 2, 2)                                                                                        \
	ACCESSOR_3_FN(yxx, 1, 0, 0)                                                                                        \
	ACCESSOR_3_FN(yxy, 1, 0, 1)                                                                                        \
	ACCESSOR_3_FN(yxz, 1, 0, 2)                                                                                        \
	ACCESSOR_3_FN(yyx, 1, 1, 0)                                                                                        \
	ACCESSOR_3_FN(yyy, 1, 1, 1)                                                                                        \
	ACCESSOR_3_FN(yyz, 1, 1, 2)                                                                                        \
	ACCESSOR_3_FN(yzx, 1, 2, 0)                                                                                        \
	ACCESSOR_3_FN(yzy, 1, 2, 1)                                                                                        \
	ACCESSOR_3_FN(yzz, 1, 2, 2)                                                                                        \
	ACCESSOR_3_FN(zxx, 2, 0, 0)                                                                                        \
	ACCESSOR_3_FN(zxy, 2, 0, 1)                                                                                        \
	ACCESSOR_3_FN(zxz, 2, 0, 2)                                                                                        \
	ACCESSOR_3_FN(zyx, 2, 1, 0)                                                                                        \
	ACCESSOR_3_FN(zyy, 2, 1, 1)                                                                                        \
	ACCESSOR_3_FN(zyz, 2, 1, 2)                                                                                        \
	ACCESSOR_3_FN(zzx, 2, 2, 0)                                                                                        \
	ACCESSOR_3_FN(zzy, 2, 2, 1)                                                                                        \
	ACCESSOR_3_FN(zzz, 2, 2, 2)

#define ACCESSOR_4()                                                                                                   \
	ACCESSOR_1_FN(w, 3)                                                                                                \
	ACCESSOR_2_FN(xw, 0, 3)                                                                                            \
	ACCESSOR_2_FN(yw, 1, 3)                                                                                            \
	ACCESSOR_2_FN(zw, 2, 3)                                                                                            \
	ACCESSOR_2_FN(wx, 3, 0)                                                                                            \
	ACCESSOR_2_FN(wy, 3, 1)                                                                                            \
	ACCESSOR_2_FN(wz, 3, 2)                                                                                            \
	ACCESSOR_2_FN(ww, 3, 3)                                                                                            \
	ACCESSOR_3_FN(xxw, 0, 0, 3)                                                                                        \
	ACCESSOR_3_FN(xyw, 0, 1, 3)                                                                                        \
	ACCESSOR_3_FN(xzw, 0, 2, 3)                                                                                        \
	ACCESSOR_3_FN(xwx, 0, 3, 0)                                                                                        \
	ACCESSOR_3_FN(xwy, 0, 3, 1)                                                                                        \
	ACCESSOR_3_FN(xwz, 0, 3, 2)                                                                                        \
	ACCESSOR_3_FN(xww, 0, 3, 3)                                                                                        \
	ACCESSOR_3_FN(yxw, 1, 0, 3)                                                                                        \
	ACCESSOR_3_FN(yyw, 1, 1, 3)                                                                                        \
	ACCESSOR_3_FN(yzw, 1, 2, 3)                                                                                        \
	ACCESSOR_3_FN(ywx, 1, 3, 0)                                                                                        \
	ACCESSOR_3_FN(ywy, 1, 3, 1)                                                                                        \
	ACCESSOR_3_FN(ywz, 1, 3, 2)                                                                                        \
	ACCESSOR_3_FN(yww, 1, 3, 3)                                                                                        \
	ACCESSOR_3_FN(zxw, 2, 0, 3)                                                                                        \
	ACCESSOR_3_FN(zyw, 2, 1, 3)                                                                                        \
	ACCESSOR_3_FN(zzw, 2, 2, 3)                                                                                        \
	ACCESSOR_3_FN(zwx, 2, 3, 0)                                                                                        \
	ACCESSOR_3_FN(zwy, 2, 3, 1)                                                                                        \
	ACCESSOR_3_FN(zwz, 2, 3, 2)                                                                                        \
	ACCESSOR_3_FN(zww, 2, 3, 3)                                                                                        \
	ACCESSOR_3_FN(wxx, 3, 0, 0)                                                                                        \
	ACCESSOR_3_FN(wxy, 3, 0, 1)                                                                                        \
	ACCESSOR_3_FN(wxz, 3, 0, 2)                                                                                        \
	ACCESSOR_3_FN(wxw, 3, 0, 3)                                                                                        \
	ACCESSOR_3_FN(wyx, 3, 1, 0)                                                                                        \
	ACCESSOR_3_FN(wyy, 3, 1, 1)                                                                                        \
	ACCESSOR_3_FN(wyz, 3, 1, 2)                                                                                        \
	ACCESSOR_3_FN(wyw, 3, 1, 3)                                                                                        \
	ACCESSOR_3_FN(wzx, 3, 2, 0)                                                                                        \
	ACCESSOR_3_FN(wzy, 3, 2, 1)                                                                                        \
	ACCESSOR_3_FN(wzz, 3, 2, 2)                                                                                        \
	ACCESSOR_3_FN(wzw, 3, 2, 3)                                                                                        \
	ACCESSOR_3_FN(wwx, 3, 3, 0)                                                                                        \
	ACCESSOR_3_FN(wwy, 3, 3, 1)                                                                                        \
	ACCESSOR_3_FN(wwz, 3, 3, 2)                                                                                        \
	ACCESSOR_3_FN(www, 3, 3, 3)                                                                                        \
	ACCESSOR_4_FN(xxxx, 0, 0, 0, 0)                                                                                    \
	ACCESSOR_4_FN(xxxy, 0, 0, 0, 1)                                                                                    \
	ACCESSOR_4_FN(xxxz, 0, 0, 0, 2)                                                                                    \
	ACCESSOR_4_FN(xxxw, 0, 0, 0, 3)                                                                                    \
	ACCESSOR_4_FN(xxyx, 0, 0, 1, 0)                                                                                    \
	ACCESSOR_4_FN(xxyy, 0, 0, 1, 1)                                                                                    \
	ACCESSOR_4_FN(xxyz, 0, 0, 1, 2)                                                                                    \
	ACCESSOR_4_FN(xxyw, 0, 0, 1, 3)                                                                                    \
	ACCESSOR_4_FN(xxzx, 0, 0, 2, 0)                                                                                    \
	ACCESSOR_4_FN(xxzy, 0, 0, 2, 1)                                                                                    \
	ACCESSOR_4_FN(xxzz, 0, 0, 2, 2)                                                                                    \
	ACCESSOR_4_FN(xxzw, 0, 0, 2, 3)                                                                                    \
	ACCESSOR_4_FN(xxwx, 0, 0, 3, 0)                                                                                    \
	ACCESSOR_4_FN(xxwy, 0, 0, 3, 1)                                                                                    \
	ACCESSOR_4_FN(xxwz, 0, 0, 3, 2)                                                                                    \
	ACCESSOR_4_FN(xxww, 0, 0, 3, 3)                                                                                    \
	ACCESSOR_4_FN(xyxx, 0, 1, 0, 0)                                                                                    \
	ACCESSOR_4_FN(xyxy, 0, 1, 0, 1)                                                                                    \
	ACCESSOR_4_FN(xyxz, 0, 1, 0, 2)                                                                                    \
	ACCESSOR_4_FN(xyxw, 0, 1, 0, 3)                                                                                    \
	ACCESSOR_4_FN(xyyx, 0, 1, 1, 0)                                                                                    \
	ACCESSOR_4_FN(xyyy, 0, 1, 1, 1)                                                                                    \
	ACCESSOR_4_FN(xyyz, 0, 1, 1, 2)                                                                                    \
	ACCESSOR_4_FN(xyyw, 0, 1, 1, 3)                                                                                    \
	ACCESSOR_4_FN(xyzx, 0, 1, 2, 0)                                                                                    \
	ACCESSOR_4_FN(xyzy, 0, 1, 2, 1)                                                                                    \
	ACCESSOR_4_FN(xyzz, 0, 1, 2, 2)                                                                                    \
	ACCESSOR_4_FN(xyzw, 0, 1, 2, 3)                                                                                    \
	ACCESSOR_4_FN(xywx, 0, 1, 3, 0)                                                                                    \
	ACCESSOR_4_FN(xywy, 0, 1, 3, 1)                                                                                    \
	ACCESSOR_4_FN(xywz, 0, 1, 3, 2)                                                                                    \
	ACCESSOR_4_FN(xyww, 0, 1, 3, 3)                                                                                    \
	ACCESSOR_4_FN(xzxx, 0, 2, 0, 0)                                                                                    \
	ACCESSOR_4_FN(xzxy, 0, 2, 0, 1)                                                                                    \
	ACCESSOR_4_FN(xzxz, 0, 2, 0, 2)                                                                                    \
	ACCESSOR_4_FN(xzxw, 0, 2, 0, 3)                                                                                    \
	ACCESSOR_4_FN(xzyx, 0, 2, 1, 0)                                                                                    \
	ACCESSOR_4_FN(xzyy, 0, 2, 1, 1)                                                                                    \
	ACCESSOR_4_FN(xzyz, 0, 2, 1, 2)                                                                                    \
	ACCESSOR_4_FN(xzyw, 0, 2, 1, 3)                                                                                    \
	ACCESSOR_4_FN(xzzx, 0, 2, 2, 0)                                                                                    \
	ACCESSOR_4_FN(xzzy, 0, 2, 2, 1)                                                                                    \
	ACCESSOR_4_FN(xzzz, 0, 2, 2, 2)                                                                                    \
	ACCESSOR_4_FN(xzzw, 0, 2, 2, 3)                                                                                    \
	ACCESSOR_4_FN(xzwx, 0, 2, 3, 0)                                                                                    \
	ACCESSOR_4_FN(xzwy, 0, 2, 3, 1)                                                                                    \
	ACCESSOR_4_FN(xzwz, 0, 2, 3, 2)                                                                                    \
	ACCESSOR_4_FN(xzww, 0, 2, 3, 3)                                                                                    \
	ACCESSOR_4_FN(xwxx, 0, 3, 0, 0)                                                                                    \
	ACCESSOR_4_FN(xwxy, 0, 3, 0, 1)                                                                                    \
	ACCESSOR_4_FN(xwxz, 0, 3, 0, 2)                                                                                    \
	ACCESSOR_4_FN(xwxw, 0, 3, 0, 3)                                                                                    \
	ACCESSOR_4_FN(xwyx, 0, 3, 1, 0)                                                                                    \
	ACCESSOR_4_FN(xwyy, 0, 3, 1, 1)                                                                                    \
	ACCESSOR_4_FN(xwyz, 0, 3, 1, 2)                                                                                    \
	ACCESSOR_4_FN(xwyw, 0, 3, 1, 3)                                                                                    \
	ACCESSOR_4_FN(xwzx, 0, 3, 2, 0)                                                                                    \
	ACCESSOR_4_FN(xwzy, 0, 3, 2, 1)                                                                                    \
	ACCESSOR_4_FN(xwzz, 0, 3, 2, 2)                                                                                    \
	ACCESSOR_4_FN(xwzw, 0, 3, 2, 3)                                                                                    \
	ACCESSOR_4_FN(xwwx, 0, 3, 3, 0)                                                                                    \
	ACCESSOR_4_FN(xwwy, 0, 3, 3, 1)                                                                                    \
	ACCESSOR_4_FN(xwwz, 0, 3, 3, 2)                                                                                    \
	ACCESSOR_4_FN(xwww, 0, 3, 3, 3)                                                                                    \
	ACCESSOR_4_FN(yxxx, 1, 0, 0, 0)                                                                                    \
	ACCESSOR_4_FN(yxxy, 1, 0, 0, 1)                                                                                    \
	ACCESSOR_4_FN(yxxz, 1, 0, 0, 2)                                                                                    \
	ACCESSOR_4_FN(yxxw, 1, 0, 0, 3)                                                                                    \
	ACCESSOR_4_FN(yxyx, 1, 0, 1, 0)                                                                                    \
	ACCESSOR_4_FN(yxyy, 1, 0, 1, 1)                                                                                    \
	ACCESSOR_4_FN(yxyz, 1, 0, 1, 2)                                                                                    \
	ACCESSOR_4_FN(yxyw, 1, 0, 1, 3)                                                                                    \
	ACCESSOR_4_FN(yxzx, 1, 0, 2, 0)                                                                                    \
	ACCESSOR_4_FN(yxzy, 1, 0, 2, 1)                                                                                    \
	ACCESSOR_4_FN(yxzz, 1, 0, 2, 2)                                                                                    \
	ACCESSOR_4_FN(yxzw, 1, 0, 2, 3)                                                                                    \
	ACCESSOR_4_FN(yxwx, 1, 0, 3, 0)                                                                                    \
	ACCESSOR_4_FN(yxwy, 1, 0, 3, 1)                                                                                    \
	ACCESSOR_4_FN(yxwz, 1, 0, 3, 2)                                                                                    \
	ACCESSOR_4_FN(yxww, 1, 0, 3, 3)                                                                                    \
	ACCESSOR_4_FN(yyxx, 1, 1, 0, 0)                                                                                    \
	ACCESSOR_4_FN(yyxy, 1, 1, 0, 1)                                                                                    \
	ACCESSOR_4_FN(yyxz, 1, 1, 0, 2)                                                                                    \
	ACCESSOR_4_FN(yyxw, 1, 1, 0, 3)                                                                                    \
	ACCESSOR_4_FN(yyyx, 1, 1, 1, 0)                                                                                    \
	ACCESSOR_4_FN(yyyy, 1, 1, 1, 1)                                                                                    \
	ACCESSOR_4_FN(yyyz, 1, 1, 1, 2)                                                                                    \
	ACCESSOR_4_FN(yyyw, 1, 1, 1, 3)                                                                                    \
	ACCESSOR_4_FN(yyzx, 1, 1, 2, 0)                                                                                    \
	ACCESSOR_4_FN(yyzy, 1, 1, 2, 1)                                                                                    \
	ACCESSOR_4_FN(yyzz, 1, 1, 2, 2)                                                                                    \
	ACCESSOR_4_FN(yyzw, 1, 1, 2, 3)                                                                                    \
	ACCESSOR_4_FN(yywx, 1, 1, 3, 0)                                                                                    \
	ACCESSOR_4_FN(yywy, 1, 1, 3, 1)                                                                                    \
	ACCESSOR_4_FN(yywz, 1, 1, 3, 2)                                                                                    \
	ACCESSOR_4_FN(yyww, 1, 1, 3, 3)                                                                                    \
	ACCESSOR_4_FN(yzxx, 1, 2, 0, 0)                                                                                    \
	ACCESSOR_4_FN(yzxy, 1, 2, 0, 1)                                                                                    \
	ACCESSOR_4_FN(yzxz, 1, 2, 0, 2)                                                                                    \
	ACCESSOR_4_FN(yzxw, 1, 2, 0, 3)                                                                                    \
	ACCESSOR_4_FN(yzyx, 1, 2, 1, 0)                                                                                    \
	ACCESSOR_4_FN(yzyy, 1, 2, 1, 1)                                                                                    \
	ACCESSOR_4_FN(yzyz, 1, 2, 1, 2)                                                                                    \
	ACCESSOR_4_FN(yzyw, 1, 2, 1, 3)                                                                                    \
	ACCESSOR_4_FN(yzzx, 1, 2, 2, 0)                                                                                    \
	ACCESSOR_4_FN(yzzy, 1, 2, 2, 1)                                                                                    \
	ACCESSOR_4_FN(yzzz, 1, 2, 2, 2)                                                                                    \
	ACCESSOR_4_FN(yzzw, 1, 2, 2, 3)                                                                                    \
	ACCESSOR_4_FN(yzwx, 1, 2, 3, 0)                                                                                    \
	ACCESSOR_4_FN(yzwy, 1, 2, 3, 1)                                                                                    \
	ACCESSOR_4_FN(yzwz, 1, 2, 3, 2)                                                                                    \
	ACCESSOR_4_FN(yzww, 1, 2, 3, 3)                                                                                    \
	ACCESSOR_4_FN(ywxx, 1, 3, 0, 0)                                                                                    \
	ACCESSOR_4_FN(ywxy, 1, 3, 0, 1)                                                                                    \
	ACCESSOR_4_FN(ywxz, 1, 3, 0, 2)                                                                                    \
	ACCESSOR_4_FN(ywxw, 1, 3, 0, 3)                                                                                    \
	ACCESSOR_4_FN(ywyx, 1, 3, 1, 0)                                                                                    \
	ACCESSOR_4_FN(ywyy, 1, 3, 1, 1)                                                                                    \
	ACCESSOR_4_FN(ywyz, 1, 3, 1, 2)                                                                                    \
	ACCESSOR_4_FN(ywyw, 1, 3, 1, 3)                                                                                    \
	ACCESSOR_4_FN(ywzx, 1, 3, 2, 0)                                                                                    \
	ACCESSOR_4_FN(ywzy, 1, 3, 2, 1)                                                                                    \
	ACCESSOR_4_FN(ywzz, 1, 3, 2, 2)                                                                                    \
	ACCESSOR_4_FN(ywzw, 1, 3, 2, 3)                                                                                    \
	ACCESSOR_4_FN(ywwx, 1, 3, 3, 0)                                                                                    \
	ACCESSOR_4_FN(ywwy, 1, 3, 3, 1)                                                                                    \
	ACCESSOR_4_FN(ywwz, 1, 3, 3, 2)                                                                                    \
	ACCESSOR_4_FN(ywww, 1, 3, 3, 3)                                                                                    \
	ACCESSOR_4_FN(zxxx, 2, 0, 0, 0)                                                                                    \
	ACCESSOR_4_FN(zxxy, 2, 0, 0, 1)                                                                                    \
	ACCESSOR_4_FN(zxxz, 2, 0, 0, 2)                                                                                    \
	ACCESSOR_4_FN(zxxw, 2, 0, 0, 3)                                                                                    \
	ACCESSOR_4_FN(zxyx, 2, 0, 1, 0)                                                                                    \
	ACCESSOR_4_FN(zxyy, 2, 0, 1, 1)                                                                                    \
	ACCESSOR_4_FN(zxyz, 2, 0, 1, 2)                                                                                    \
	ACCESSOR_4_FN(zxyw, 2, 0, 1, 3)                                                                                    \
	ACCESSOR_4_FN(zxzx, 2, 0, 2, 0)                                                                                    \
	ACCESSOR_4_FN(zxzy, 2, 0, 2, 1)                                                                                    \
	ACCESSOR_4_FN(zxzz, 2, 0, 2, 2)                                                                                    \
	ACCESSOR_4_FN(zxzw, 2, 0, 2, 3)                                                                                    \
	ACCESSOR_4_FN(zxwx, 2, 0, 3, 0)                                                                                    \
	ACCESSOR_4_FN(zxwy, 2, 0, 3, 1)                                                                                    \
	ACCESSOR_4_FN(zxwz, 2, 0, 3, 2)                                                                                    \
	ACCESSOR_4_FN(zxww, 2, 0, 3, 3)                                                                                    \
	ACCESSOR_4_FN(zyxx, 2, 1, 0, 0)                                                                                    \
	ACCESSOR_4_FN(zyxy, 2, 1, 0, 1)                                                                                    \
	ACCESSOR_4_FN(zyxz, 2, 1, 0, 2)                                                                                    \
	ACCESSOR_4_FN(zyxw, 2, 1, 0, 3)                                                                                    \
	ACCESSOR_4_FN(zyyx, 2, 1, 1, 0)                                                                                    \
	ACCESSOR_4_FN(zyyy, 2, 1, 1, 1)                                                                                    \
	ACCESSOR_4_FN(zyyz, 2, 1, 1, 2)                                                                                    \
	ACCESSOR_4_FN(zyyw, 2, 1, 1, 3)                                                                                    \
	ACCESSOR_4_FN(zyzx, 2, 1, 2, 0)                                                                                    \
	ACCESSOR_4_FN(zyzy, 2, 1, 2, 1)                                                                                    \
	ACCESSOR_4_FN(zyzz, 2, 1, 2, 2)                                                                                    \
	ACCESSOR_4_FN(zyzw, 2, 1, 2, 3)                                                                                    \
	ACCESSOR_4_FN(zywx, 2, 1, 3, 0)                                                                                    \
	ACCESSOR_4_FN(zywy, 2, 1, 3, 1)                                                                                    \
	ACCESSOR_4_FN(zywz, 2, 1, 3, 2)                                                                                    \
	ACCESSOR_4_FN(zyww, 2, 1, 3, 3)                                                                                    \
	ACCESSOR_4_FN(zzxx, 2, 2, 0, 0)                                                                                    \
	ACCESSOR_4_FN(zzxy, 2, 2, 0, 1)                                                                                    \
	ACCESSOR_4_FN(zzxz, 2, 2, 0, 2)                                                                                    \
	ACCESSOR_4_FN(zzxw, 2, 2, 0, 3)                                                                                    \
	ACCESSOR_4_FN(zzyx, 2, 2, 1, 0)                                                                                    \
	ACCESSOR_4_FN(zzyy, 2, 2, 1, 1)                                                                                    \
	ACCESSOR_4_FN(zzyz, 2, 2, 1, 2)                                                                                    \
	ACCESSOR_4_FN(zzyw, 2, 2, 1, 3)                                                                                    \
	ACCESSOR_4_FN(zzzx, 2, 2, 2, 0)                                                                                    \
	ACCESSOR_4_FN(zzzy, 2, 2, 2, 1)                                                                                    \
	ACCESSOR_4_FN(zzzz, 2, 2, 2, 2)                                                                                    \
	ACCESSOR_4_FN(zzzw, 2, 2, 2, 3)                                                                                    \
	ACCESSOR_4_FN(zzwx, 2, 2, 3, 0)                                                                                    \
	ACCESSOR_4_FN(zzwy, 2, 2, 3, 1)                                                                                    \
	ACCESSOR_4_FN(zzwz, 2, 2, 3, 2)                                                                                    \
	ACCESSOR_4_FN(zzww, 2, 2, 3, 3)                                                                                    \
	ACCESSOR_4_FN(zwxx, 2, 3, 0, 0)                                                                                    \
	ACCESSOR_4_FN(zwxy, 2, 3, 0, 1)                                                                                    \
	ACCESSOR_4_FN(zwxz, 2, 3, 0, 2)                                                                                    \
	ACCESSOR_4_FN(zwxw, 2, 3, 0, 3)                                                                                    \
	ACCESSOR_4_FN(zwyx, 2, 3, 1, 0)                                                                                    \
	ACCESSOR_4_FN(zwyy, 2, 3, 1, 1)                                                                                    \
	ACCESSOR_4_FN(zwyz, 2, 3, 1, 2)                                                                                    \
	ACCESSOR_4_FN(zwyw, 2, 3, 1, 3)                                                                                    \
	ACCESSOR_4_FN(zwzx, 2, 3, 2, 0)                                                                                    \
	ACCESSOR_4_FN(zwzy, 2, 3, 2, 1)                                                                                    \
	ACCESSOR_4_FN(zwzz, 2, 3, 2, 2)                                                                                    \
	ACCESSOR_4_FN(zwzw, 2, 3, 2, 3)                                                                                    \
	ACCESSOR_4_FN(zwwx, 2, 3, 3, 0)                                                                                    \
	ACCESSOR_4_FN(zwwy, 2, 3, 3, 1)                                                                                    \
	ACCESSOR_4_FN(zwwz, 2, 3, 3, 2)                                                                                    \
	ACCESSOR_4_FN(zwww, 2, 3, 3, 3)                                                                                    \
	ACCESSOR_4_FN(wxxx, 3, 0, 0, 0)                                                                                    \
	ACCESSOR_4_FN(wxxy, 3, 0, 0, 1)                                                                                    \
	ACCESSOR_4_FN(wxxz, 3, 0, 0, 2)                                                                                    \
	ACCESSOR_4_FN(wxxw, 3, 0, 0, 3)                                                                                    \
	ACCESSOR_4_FN(wxyx, 3, 0, 1, 0)                                                                                    \
	ACCESSOR_4_FN(wxyy, 3, 0, 1, 1)                                                                                    \
	ACCESSOR_4_FN(wxyz, 3, 0, 1, 2)                                                                                    \
	ACCESSOR_4_FN(wxyw, 3, 0, 1, 3)                                                                                    \
	ACCESSOR_4_FN(wxzx, 3, 0, 2, 0)                                                                                    \
	ACCESSOR_4_FN(wxzy, 3, 0, 2, 1)                                                                                    \
	ACCESSOR_4_FN(wxzz, 3, 0, 2, 2)                                                                                    \
	ACCESSOR_4_FN(wxzw, 3, 0, 2, 3)                                                                                    \
	ACCESSOR_4_FN(wxwx, 3, 0, 3, 0)                                                                                    \
	ACCESSOR_4_FN(wxwy, 3, 0, 3, 1)                                                                                    \
	ACCESSOR_4_FN(wxwz, 3, 0, 3, 2)                                                                                    \
	ACCESSOR_4_FN(wxww, 3, 0, 3, 3)                                                                                    \
	ACCESSOR_4_FN(wyxx, 3, 1, 0, 0)                                                                                    \
	ACCESSOR_4_FN(wyxy, 3, 1, 0, 1)                                                                                    \
	ACCESSOR_4_FN(wyxz, 3, 1, 0, 2)                                                                                    \
	ACCESSOR_4_FN(wyxw, 3, 1, 0, 3)                                                                                    \
	ACCESSOR_4_FN(wyyx, 3, 1, 1, 0)                                                                                    \
	ACCESSOR_4_FN(wyyy, 3, 1, 1, 1)                                                                                    \
	ACCESSOR_4_FN(wyyz, 3, 1, 1, 2)                                                                                    \
	ACCESSOR_4_FN(wyyw, 3, 1, 1, 3)                                                                                    \
	ACCESSOR_4_FN(wyzx, 3, 1, 2, 0)                                                                                    \
	ACCESSOR_4_FN(wyzy, 3, 1, 2, 1)                                                                                    \
	ACCESSOR_4_FN(wyzz, 3, 1, 2, 2)                                                                                    \
	ACCESSOR_4_FN(wyzw, 3, 1, 2, 3)                                                                                    \
	ACCESSOR_4_FN(wywx, 3, 1, 3, 0)                                                                                    \
	ACCESSOR_4_FN(wywy, 3, 1, 3, 1)                                                                                    \
	ACCESSOR_4_FN(wywz, 3, 1, 3, 2)                                                                                    \
	ACCESSOR_4_FN(wyww, 3, 1, 3, 3)                                                                                    \
	ACCESSOR_4_FN(wzxx, 3, 2, 0, 0)                                                                                    \
	ACCESSOR_4_FN(wzxy, 3, 2, 0, 1)                                                                                    \
	ACCESSOR_4_FN(wzxz, 3, 2, 0, 2)                                                                                    \
	ACCESSOR_4_FN(wzxw, 3, 2, 0, 3)                                                                                    \
	ACCESSOR_4_FN(wzyx, 3, 2, 1, 0)                                                                                    \
	ACCESSOR_4_FN(wzyy, 3, 2, 1, 1)                                                                                    \
	ACCESSOR_4_FN(wzyz, 3, 2, 1, 2)                                                                                    \
	ACCESSOR_4_FN(wzyw, 3, 2, 1, 3)                                                                                    \
	ACCESSOR_4_FN(wzzx, 3, 2, 2, 0)                                                                                    \
	ACCESSOR_4_FN(wzzy, 3, 2, 2, 1)                                                                                    \
	ACCESSOR_4_FN(wzzz, 3, 2, 2, 2)                                                                                    \
	ACCESSOR_4_FN(wzzw, 3, 2, 2, 3)                                                                                    \
	ACCESSOR_4_FN(wzwx, 3, 2, 3, 0)                                                                                    \
	ACCESSOR_4_FN(wzwy, 3, 2, 3, 1)                                                                                    \
	ACCESSOR_4_FN(wzwz, 3, 2, 3, 2)                                                                                    \
	ACCESSOR_4_FN(wzww, 3, 2, 3, 3)                                                                                    \
	ACCESSOR_4_FN(wwxx, 3, 3, 0, 0)                                                                                    \
	ACCESSOR_4_FN(wwxy, 3, 3, 0, 1)                                                                                    \
	ACCESSOR_4_FN(wwxz, 3, 3, 0, 2)                                                                                    \
	ACCESSOR_4_FN(wwxw, 3, 3, 0, 3)                                                                                    \
	ACCESSOR_4_FN(wwyx, 3, 3, 1, 0)                                                                                    \
	ACCESSOR_4_FN(wwyy, 3, 3, 1, 1)                                                                                    \
	ACCESSOR_4_FN(wwyz, 3, 3, 1, 2)                                                                                    \
	ACCESSOR_4_FN(wwyw, 3, 3, 1, 3)                                                                                    \
	ACCESSOR_4_FN(wwzx, 3, 3, 2, 0)                                                                                    \
	ACCESSOR_4_FN(wwzy, 3, 3, 2, 1)                                                                                    \
	ACCESSOR_4_FN(wwzz, 3, 3, 2, 2)                                                                                    \
	ACCESSOR_4_FN(wwzw, 3, 3, 2, 3)                                                                                    \
	ACCESSOR_4_FN(wwwx, 3, 3, 3, 0)                                                                                    \
	ACCESSOR_4_FN(wwwy, 3, 3, 3, 1)                                                                                    \
	ACCESSOR_4_FN(wwwz, 3, 3, 3, 2)                                                                                    \
	ACCESSOR_4_FN(wwww, 3, 3, 3, 3)

namespace psl
{
	template <typename T, size_t Size>
	class tvec;

	inline namespace details
	{
		template <typename T>
		struct is_tvec : std::false_type
		{};

		template <typename T, size_t N>
		struct is_tvec<tvec<T, N>> : std::true_type
		{};

		template <typename T>
		struct is_tvec_like : public is_tvec<T>
		{};

		template <typename T, size_t N>
		struct is_tvec_like<std::array<T, N>> : std::true_type
		{};

		template <typename T>
		concept IsTvec = is_tvec<std::remove_cvref_t<T>>::value;
		template <typename T>
		concept IsTvecLike = is_tvec_like<std::remove_cvref_t<T>>::value;

		// these contain predefined values such as 'forward', 'up', etc..
		// to help users in avoiding duplications.
		template <typename T, size_t Size>
		struct tvec_predefined
		{
			static constexpr bool is_accessor = std::is_reference_v<T>;
			using value_type =
				std::conditional_t<std::is_reference_v<T>, std::remove_cvref_t<T>*, std::remove_cvref_t<T>>;
			using reference		  = std::remove_cvref_t<T>&;
			using const_reference = const std::remove_cvref_t<T>&;
			std::array<value_type, Size> m_Data;
		};

		template <typename T>
		struct tvec_predefined<T, 1>
		{
			static constexpr size_t Size	  = 1;
			static constexpr bool is_accessor = std::is_reference_v<T>;
			using reference					  = std::remove_cvref_t<T>&;
			using const_reference			  = const std::remove_cvref_t<T>&;
			static constexpr tvec<T, 1> zero{T{0}};
			static constexpr tvec<T, 1> one{T{1}};
			template <size_t Index>
			constexpr reference get() noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}

			template <size_t Index>
			constexpr const_reference get() const noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}
			ACCESSOR_1();

			using value_type =
				std::conditional_t<std::is_reference_v<T>, std::remove_cvref_t<T>*, std::remove_cvref_t<T>>;
			std::array<value_type, 1> m_Data;
		};
		template <typename T>
		struct tvec_predefined<T, 2>
		{
			static constexpr size_t Size	  = 2;
			static constexpr bool is_accessor = std::is_reference_v<T>;
			using reference					  = std::remove_cvref_t<T>&;
			using const_reference			  = const std::remove_cvref_t<T>&;
			static constexpr tvec<T, 2> zero{T{0}};
			static constexpr tvec<T, 2> one{T{1}};
			static constexpr tvec<T, 2> up{T{0}, T{1}};
			static constexpr tvec<T, 2> down{T{0}, -T{1}};
			static constexpr tvec<T, 2> right{T{1}, T{0}};
			static constexpr tvec<T, 2> left{-T{1}, T{0}};
			template <size_t Index>
			constexpr reference get() noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}

			template <size_t Index>
			constexpr const_reference get() const noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}
			ACCESSOR_1();
			ACCESSOR_2();
			using value_type =
				std::conditional_t<std::is_reference_v<T>, std::remove_cvref_t<T>*, std::remove_cvref_t<T>>;
			std::array<value_type, 2> m_Data;
		};
		template <typename T>
		struct tvec_predefined<T, 3>
		{
			static constexpr size_t Size	  = 3;
			static constexpr bool is_accessor = std::is_reference_v<T>;
			using reference					  = std::remove_cvref_t<T>&;
			using const_reference			  = const std::remove_cvref_t<T>&;
			static constexpr tvec<T, 3> zero{T{0}};
			static constexpr tvec<T, 3> one{T{1}};
			static constexpr tvec<T, 3> forward{T{0}, T{0}, T{1}};
			static constexpr tvec<T, 3> back{T{0}, T{0}, -T{1}};
			static constexpr tvec<T, 3> up{T{0}, T{1}, T{0}};
			static constexpr tvec<T, 3> down{T{0}, -T{1}, T{0}};
			static constexpr tvec<T, 3> right{T{1}, T{0}, T{0}};
			static constexpr tvec<T, 3> left{-T{1}, T{0}, T{0}};
			template <size_t Index>
			constexpr reference get() noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}

			template <size_t Index>
			constexpr const_reference get() const noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}
			ACCESSOR_1();
			ACCESSOR_2();
			ACCESSOR_3();

			using value_type =
				std::conditional_t<std::is_reference_v<T>, std::remove_cvref_t<T>*, std::remove_cvref_t<T>>;
			std::array<value_type, 3> m_Data;
		};
		template <typename T>
		struct tvec_predefined<T, 4>
		{
			static constexpr size_t Size	  = 4;
			static constexpr bool is_accessor = std::is_reference_v<T>;
			using reference					  = std::remove_cvref_t<T>&;
			using const_reference			  = const std::remove_cvref_t<T>&;
			static constexpr tvec<T, 4> zero{T{0}};
			static constexpr tvec<T, 4> one{T{1}};
			static constexpr tvec<T, 4> forward{T{0}, T{0}, T{1}, T{0}};
			static constexpr tvec<T, 4> back{T{0}, T{0}, -T{1}, T{0}};
			static constexpr tvec<T, 4> up{T{0}, T{1}, T{0}, T{0}};
			static constexpr tvec<T, 4> down{T{0}, -T{1}, T{0}, T{0}};
			static constexpr tvec<T, 4> right{T{1}, T{0}, T{0}, T{0}};
			static constexpr tvec<T, 4> left{-T{1}, T{0}, T{0}, T{0}};
			static constexpr tvec<T, 4> point{T{0}, T{0}, T{0}, T{1}};
			template <size_t Index>
			constexpr reference get() noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}

			template <size_t Index>
			constexpr const_reference get() const noexcept requires(Index < Size)
			{
				if constexpr(is_accessor)
					return *this->m_Data[Index];
				else
					return this->m_Data[Index];
			}

			ACCESSOR_1();
			ACCESSOR_2();
			ACCESSOR_3();
			ACCESSOR_4();
			using value_type =
				std::conditional_t<std::is_reference_v<T>, std::remove_cvref_t<T>*, std::remove_cvref_t<T>>;
			std::array<value_type, 4> m_Data;
		};

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
	} // namespace details

	template <typename T, size_t Size>
	class tvec : public tvec_predefined<T, Size>
	{
	  public:
		using base_type					   = tvec_predefined<T, Size>;
		static constexpr size_t DIMENSIONS = Size;
		static constexpr bool is_accessor  = std::is_reference_v<T>;
		using type						   = std::remove_cvref_t<T>;
		using value_type				   = base_type::value_type;
		using reference					   = base_type::reference;
		using const_reference			   = base_type::const_reference;
		using accessor_type				   = tvec<const_reference, Size>;

		constexpr tvec(const accessor_type& value) noexcept requires(!std::is_reference_v<T>)
		{
			for_each_inplace([](auto& lhs, auto rhs) { lhs = rhs; }, this->m_Data, value);
		}

		// generic aggregatte constructor
		template <typename... Ts>
		constexpr tvec(Ts&&... values) noexcept requires(
			!is_accessor && std::conjunction_v<std::is_same<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>>...> &&
			(sizeof...(Ts) == Size))
			: base_type({std::forward<Ts>(values)...})
		{}

		// fill constructor
		template <typename Y>
		constexpr tvec(Y&& value) noexcept requires(!is_accessor &&
													std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Y>> &&
													Size > 1)
		{
			this->m_Data.fill(value);
		}

		template <typename... Ts>
		constexpr tvec(Ts&... values) noexcept requires(
			is_accessor&& std::conjunction_v<std::is_same<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>>...>)
			: base_type({const_cast<value_type>(&values)...})
		{}

		constexpr reference operator[](size_t index) noexcept requires(!is_accessor) { return this->m_Data[index]; }
		constexpr reference operator[](size_t index) noexcept requires(is_accessor) { return *this->m_Data[index]; }
		constexpr const_reference operator[](size_t index) const noexcept requires(!is_accessor)
		{
			return this->m_Data[index];
		}
		constexpr const_reference operator[](size_t index) const noexcept requires(is_accessor)
		{
			return *this->m_Data[index];
		}


		constexpr tvec(const tvec& other) noexcept = default;
		constexpr tvec(tvec&& other) noexcept	  = default;
		constexpr tvec& operator=(const tvec& other) noexcept = default;
		constexpr tvec& operator=(tvec&& other) noexcept = default;
		constexpr bool operator==(const tvec& other) const noexcept { return this->m_Data == other.m_Data; };
		constexpr bool operator!=(const tvec& other) const noexcept { return this->m_Data != other.m_Data; };
		constexpr bool operator<=>(const tvec& other) const noexcept { return this->m_Data <=> other.m_Data; }

		// constexpr T* data() noexcept { m_Data.data(); }

		template <typename Y>
		constexpr tvec& operator+=(const tvec<Y, Size>& other) noexcept requires(!is_accessor)
		{
			for_each_inplace([](auto& lhs, const auto& rhs) noexcept { lhs += rhs; }, this->m_Data, other);
			return *this;
		}

		template <typename Y>
		constexpr tvec operator+(const tvec<Y, Size>& other) const noexcept requires(!is_accessor)
		{
			auto copy = *this;
			copy += other;
			return copy;
		}

		template <typename Y>
		constexpr auto operator+(const tvec<Y, Size>& other) const noexcept requires(is_accessor)
		{
			tvec<type, Size> copy{*this};
			return copy += other;
		}

		template <typename Y>
		constexpr tvec& operator-=(const tvec<Y, Size>& other) noexcept requires(!is_accessor)
		{
			for_each_inplace([](auto& lhs, const auto& rhs) noexcept { lhs -= rhs; }, this->m_Data, other);
			return *this;
		}

		template <typename Y>
		constexpr tvec operator-(const tvec<Y, Size>& other) const noexcept requires(!is_accessor)
		{
			auto copy = *this;
			copy -= other;
			return copy;
		}

		template <typename Y>
		constexpr auto operator-(const tvec<Y, Size>& other) const noexcept requires(is_accessor)
		{
			tvec<type, Size> copy{*this};
			return copy -= other;
		}

		template <typename Y>
		constexpr tvec& operator*=(const tvec<Y, Size>& other) noexcept requires(!is_accessor)
		{
			for_each_inplace([](auto& lhs, const auto& rhs) noexcept { lhs *= rhs; }, this->m_Data, other);
			return *this;
		}

		template <typename Y>
		constexpr tvec operator*(const tvec<Y, Size>& other) const noexcept requires(!is_accessor)
		{
			auto copy = *this;
			copy *= other;
			return copy;
		}

		template <typename Y>
		constexpr auto operator*(const tvec<Y, Size>& other) const noexcept requires(is_accessor)
		{
			tvec<type, Size> copy{*this};
			return copy *= other;
		}

		template <typename Y>
		constexpr tvec& operator/=(const tvec<Y, Size>& other) noexcept requires(!is_accessor)
		{
			for_each_inplace([](auto& lhs, const auto& rhs) noexcept { lhs /= rhs; }, this->m_Data, other);
			return *this;
		}

		template <typename Y>
		constexpr tvec operator/(const tvec<Y, Size>& other) const noexcept requires(!is_accessor)
		{
			auto copy = *this;
			copy /= other;
			return copy;
		}

		template <typename Y>
		constexpr auto operator/(const tvec<Y, Size>& other) const noexcept requires(is_accessor)
		{
			tvec<type, Size> copy{*this};
			return copy /= other;
		}

		constexpr tvec operator-() const noexcept requires(!is_accessor)
		{
			auto copy = *this;
			for_each_inplace([](auto& lhs) noexcept { lhs = -lhs; }, copy);
			return copy;
		}


		template <typename Y>
		constexpr auto operator-() const noexcept requires(is_accessor)
		{
			tvec<type, Size> copy{*this};
			for_each_inplace([](auto& lhs) noexcept { lhs = -lhs; }, copy);
			return copy;
		}

		constexpr tvec& rescale(T value) noexcept requires(!is_accessor)
		{
			for_each_inplace([&value](auto& element) noexcept { element *= value; }, this->m_Data);
			return *this;
		}

		[[nodiscard]] constexpr auto scale(type value) const noexcept
		{
			if constexpr(is_accessor)
			{
				auto copy = tvec<type, Size>{*this};
				copy.rescale(value);
				return copy;
			}
			else
			{
				auto copy = *this;
				copy.rescale(value);
				return copy;
			}
		}

	  private:
		template <size_t... Is0, size_t... Is1>
		tvec<T, sizeof...(Is0) + sizeof...(Is1)> resize_impl(std::index_sequence<Is0...>,
															 std::index_sequence<Is1...>) const noexcept
		{
			return tvec<T, sizeof...(Is0) + sizeof...(Is1)>(this->m_Data[Is0]..., T{Is1 * 0}...);
		}

	  public:
		template <size_t NewSize>
		[[nodiscard]] tvec<T, NewSize> resize() const noexcept requires(Size > NewSize)
		{
			return resize_impl(std::make_index_sequence<NewSize>{}, std::make_index_sequence<0>{});
		}

		template <size_t NewSize>
		[[nodiscard]] tvec<T, NewSize> resize() const noexcept requires(Size <= NewSize)
		{
			return resize_impl(std::make_index_sequence<Size>{}, std::make_index_sequence<NewSize - Size>{});
		}

		static constexpr size_t size() noexcept { return Size; }
	};

	template <typename T, typename... Ts>
	requires(std::conjunction_v<std::is_same<T, Ts>...>) tvec(T&&, Ts&&...)->tvec<T, sizeof...(Ts) + 1>;

	template <typename T, size_t Size>
	requires(std::is_reference_v<T>) tvec(tvec<T, Size>)->tvec<std::remove_reference_t<T>, Size>;

	// predefined variations
	template <size_t Size>
	using vec  = tvec<float, Size>;
	using vec1 = vec<1>;
	using vec2 = vec<2>;
	using vec3 = vec<3>;
	using vec4 = vec<4>;

	template <size_t Size>
	using dvec  = tvec<double, Size>;
	using dvec1 = dvec<1>;
	using dvec2 = dvec<2>;
	using dvec3 = dvec<3>;
	using dvec4 = dvec<4>;

	template <size_t Size>
	using ivec  = tvec<int, Size>;
	using ivec1 = ivec<1>;
	using ivec2 = ivec<2>;
	using ivec3 = ivec<3>;
	using ivec4 = ivec<4>;

	template <size_t Size>
	using i32vec  = tvec<int32_t, Size>;
	using i32vec1 = i32vec<1>;
	using i32vec2 = i32vec<2>;
	using i32vec3 = i32vec<3>;
	using i32vec4 = i32vec<4>;

	template <size_t Size>
	using i64vec  = tvec<int64_t, Size>;
	using i64vec1 = i64vec<1>;
	using i64vec2 = i64vec<2>;
	using i64vec3 = i64vec<3>;
	using i64vec4 = i64vec<4>;

	template <size_t Size>
	using uvec  = tvec<unsigned int, Size>;
	using uvec1 = uvec<1>;
	using uvec2 = uvec<2>;
	using uvec3 = uvec<3>;
	using uvec4 = uvec<4>;

	template <size_t Size>
	using ui32vec  = tvec<uint32_t, Size>;
	using ui32vec1 = ui32vec<1>;
	using ui32vec2 = ui32vec<2>;
	using ui32vec3 = ui32vec<3>;
	using ui32vec4 = ui32vec<4>;

	template <size_t Size>
	using ui64vec  = tvec<uint64_t, Size>;
	using ui64vec1 = ui64vec<1>;
	using ui64vec2 = ui64vec<2>;
	using ui64vec3 = ui64vec<3>;
	using ui64vec4 = ui64vec<4>;

	template <size_t Size>
	using szvec  = tvec<size_t, Size>;
	using szvec1 = szvec<1>;
	using szvec2 = szvec<2>;
	using szvec3 = szvec<3>;
	using szvec4 = szvec<4>;

} // namespace psl

// cleanup

#undef ACCESSOR_1
#undef ACCESSOR_2
#undef ACCESSOR_3
#undef ACCESSOR_4

#undef ACCESSOR_1_FN
#undef ACCESSOR_2_FN
#undef ACCESSOR_3_FN
#undef ACCESSOR_4_FN

#include <cmath>

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
	[[nodiscard]] constexpr auto max(IsTvec auto const& v0, IsTvec auto const& v1, const Ts&... values) noexcept
	{
		using std::max;
		if constexpr(sizeof...(Ts) == 0)
			return for_each([](auto e0, auto e1) { return max(e0, e1); }, v0, v1);
		else
			return psl::max(for_each([](auto e0, auto e1) { return max(e0, e1); }, v0, v1), values...);
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

	[[nodiscard]] constexpr auto abs(IsTvec auto const& v0) noexcept
	{
		using std::abs;
		return for_each([](auto e0) { return abs(e0); }, v0);
	}
} // namespace psl