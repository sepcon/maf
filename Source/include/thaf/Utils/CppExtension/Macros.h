#ifndef MACROS_H
#define MACROS_H

#define msvc_expand_va_args(X) X

#define Args_(...) __VA_ARGS__
#define STRIP_PARENTHESES_(X) X
#define mc_strip_parentheses(X) STRIP_PARENTHESES_(Args_ X)
//EXPAND((A, B)) -> STRIP_PARENS(Args_(a,b)) -> 

// Make a FOREACH macro
#define fe_1(WHAT, X)		WHAT(X)
#define fe_2(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_1(WHAT, __VA_ARGS__) )
#define fe_3(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_2(WHAT, __VA_ARGS__) )
#define fe_4(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_3(WHAT, __VA_ARGS__) )
#define fe_5(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_4(WHAT, __VA_ARGS__) )
#define fe_6(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_5(WHAT, __VA_ARGS__) )
#define fe_7(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_6(WHAT, __VA_ARGS__) )
#define fe_8(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_7(WHAT, __VA_ARGS__) )
#define fe_9(WHAT, X, ...)	WHAT(X)	msvc_expand_va_args( fe_8(WHAT, __VA_ARGS__) )
#define fe_10(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_9(WHAT, __VA_ARGS__) )
#define fe_11(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_10(WHAT, __VA_ARGS__))
#define fe_12(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_11(WHAT, __VA_ARGS__))
#define fe_13(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_12(WHAT, __VA_ARGS__))
#define fe_14(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_13(WHAT, __VA_ARGS__))
#define fe_15(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_14(WHAT, __VA_ARGS__))
#define fe_16(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_15(WHAT, __VA_ARGS__))
#define fe_17(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_16(WHAT, __VA_ARGS__))
#define fe_18(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_17(WHAT, __VA_ARGS__))
#define fe_19(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_18(WHAT, __VA_ARGS__))
#define fe_20(WHAT, X, ...) WHAT(X)	msvc_expand_va_args( fe_19(WHAT, __VA_ARGS__))
//... repeat as needed

// Make a FOREACH macro
#define fe_i_1(WHAT, X) WHAT(X, 1)
#define fe_i_2(WHAT, X, ...) WHAT(X, 2)		msvc_expand_va_args( fe_i_1(WHAT, __VA_ARGS__) )
#define fe_i_3(WHAT, X, ...) WHAT(X, 3)		msvc_expand_va_args( fe_i_2(WHAT, __VA_ARGS__) )
#define fe_i_4(WHAT, X, ...) WHAT(X, 4)		msvc_expand_va_args( fe_i_3(WHAT, __VA_ARGS__) )
#define fe_i_5(WHAT, X, ...) WHAT(X, 5)		msvc_expand_va_args( fe_i_4(WHAT, __VA_ARGS__) )
#define fe_i_6(WHAT, X, ...) WHAT(X, 6)		msvc_expand_va_args( fe_i_5(WHAT, __VA_ARGS__) )
#define fe_i_7(WHAT, X, ...) WHAT(X, 7)		msvc_expand_va_args( fe_i_6(WHAT, __VA_ARGS__) )
#define fe_i_8(WHAT, X, ...) WHAT(X, 8)		msvc_expand_va_args( fe_i_7(WHAT, __VA_ARGS__) )
#define fe_i_9(WHAT, X, ...) WHAT(X, 9)		msvc_expand_va_args( fe_i_8(WHAT, __VA_ARGS__) )
#define fe_i_10(WHAT, X, ...) WHAT(X, 10)	msvc_expand_va_args( fe_i_9(WHAT, __VA_ARGS__) )
#define fe_i_11(WHAT, X, ...) WHAT(X, 11)	msvc_expand_va_args( fe_i_10(WHAT, __VA_ARGS__))
#define fe_i_12(WHAT, X, ...) WHAT(X, 12)	msvc_expand_va_args( fe_i_11(WHAT, __VA_ARGS__))
#define fe_i_13(WHAT, X, ...) WHAT(X, 13)	msvc_expand_va_args( fe_i_12(WHAT, __VA_ARGS__))
#define fe_i_14(WHAT, X, ...) WHAT(X, 14)	msvc_expand_va_args( fe_i_13(WHAT, __VA_ARGS__))
#define fe_i_15(WHAT, X, ...) WHAT(X, 15)	msvc_expand_va_args( fe_i_14(WHAT, __VA_ARGS__))
#define fe_i_16(WHAT, X, ...) WHAT(X, 16)	msvc_expand_va_args( fe_i_15(WHAT, __VA_ARGS__))
#define fe_i_17(WHAT, X, ...) WHAT(X, 17)	msvc_expand_va_args( fe_i_16(WHAT, __VA_ARGS__))
#define fe_i_18(WHAT, X, ...) WHAT(X, 18)	msvc_expand_va_args( fe_i_17(WHAT, __VA_ARGS__))
#define fe_i_19(WHAT, X, ...) WHAT(X, 19)	msvc_expand_va_args( fe_i_18(WHAT, __VA_ARGS__))
#define fe_i_20(WHAT, X, ...) WHAT(X, 20)	msvc_expand_va_args( fe_i_19(WHAT, __VA_ARGS__))

//... repeat as needed

#define mc_get_macro(_1,_2,_3,_4,_5,_6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, NAME, ...) NAME

#define mc_args_count(...) msvc_expand_va_args( mc_get_macro( __VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1) )

#define mc_for_each(action,...) \
  msvc_expand_va_args( mc_get_macro(__VA_ARGS__, fe_20, fe_19, fe_18, fe_17, fe_16, fe_15, fe_14, fe_13, fe_12, fe_11, fe_10, fe_9, fe_8, fe_7, fe_6, fe_5, fe_4, fe_3, fe_2, fe_1)(action, __VA_ARGS__) )

#define mc_for_each_with_index(action, ...) \
  msvc_expand_va_args( mc_get_macro(__VA_ARGS__, fe_i_20, fe_i_19, fe_i_18, fe_i_17, fe_i_16, fe_i_15, fe_i_14, fe_i_13, fe_i_12, fe_i_11, fe_i_10, fe_i_9, fe_i_8, fe_i_7, fe_i_6, fe_i_5, fe_i_4, fe_i_3, fe_i_2, fe_i_1)(action, __VA_ARGS__) )
 
#define mc_remove_first_arg_(first, ...) __VA_ARGS__
#define mc_remove_first_arg(...) msvc_expand_va_args( mc_remove_first_arg_(__VA_ARGS__) )

#endif // MACROS_H
