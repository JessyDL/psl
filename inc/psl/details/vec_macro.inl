// note use vec_macro_cleanup.inl to cleanup these defines when done with them.

#define ACCESSOR_1_FN(name, index)                                                                                     \
	constexpr auto name() const noexcept requires(index < N)                                                           \
	{                                                                                                                  \
		if constexpr(index < N) return get<index>();                                                                   \
	}                                                                                                                  \
	constexpr auto name() noexcept requires(index < N)                                                                 \
	{                                                                                                                  \
		if constexpr(index < N) return get<index>();                                                                   \
	}

#define ACCESSOR_2_FN(name, index0, index1)                                                                            \
	constexpr auto name() const noexcept requires(index0 < N && index1 < N)                                            \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N)                                                                         \
			return tvec<const_reference, 2>{*this, std::index_sequence<index0, index1>{}};                             \
	}                                                                                                                  \
	constexpr auto name() noexcept requires(index0 < N && index1 < N && (index0 != index1))                            \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N)                                                                         \
			return tvec<reference, 2>{*this, std::index_sequence<index0, index1>{}};                                   \
	}


#define ACCESSOR_3_FN(name, index0, index1, index2)                                                                    \
	constexpr auto name() const noexcept requires(index0 < N && index1 < N && index2 < N)                              \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N && index2 < N)                                                           \
			return tvec<const_reference, 3>{*this, std::index_sequence<index0, index1, index2>{}};                     \
	}                                                                                                                  \
	constexpr auto name() noexcept requires(index0 < N && index1 < N && index2 < N &&                                  \
											(index0 != index1 && index1 != index2))                                    \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N && index2 < N)                                                           \
			return tvec<reference, 3>{*this, std::index_sequence<index0, index1, index2>{}};                           \
	}

#define ACCESSOR_4_FN(name, index0, index1, index2, index3)                                                            \
	constexpr auto name() const noexcept requires(index0 < N && index1 < N && index2 < N && index3 < N)                \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N && index2 < N && index3 < N)                                             \
			return tvec<const_reference, 4>{*this, std::index_sequence<index0, index1, index2, index3>{}};             \
	}                                                                                                                  \
	constexpr auto name() noexcept requires(index0 < N && index1 < N && index2 < N && index3 < N &&                    \
											(index0 != index1 && index1 != index2 && index2 != index3))                \
	{                                                                                                                  \
		if constexpr(index0 < N && index1 < N && index2 < N && index3 < N)                                             \
			return tvec<reference, 4>{*this, std::index_sequence<index0, index1, index2, index3>{}};                   \
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
