#pragma once

#define mc_maf_msvc_expand_va_args(X) X

#define Args_(...) __VA_ARGS__
#define STRIP_PARENTHESES_(X) X
#define mc_strip_parentheses(X) STRIP_PARENTHESES_(Args_ X)
// EXPAND((A, B)) -> STRIP_PARENS(Args_(a,b)) ->

// Make a FOREACH macro
#define mc_maf_foreach_1(WHAT, X) WHAT(X)
#define mc_maf_foreach_2(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_1(WHAT, __VA_ARGS__))
#define mc_maf_foreach_3(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_2(WHAT, __VA_ARGS__))
#define mc_maf_foreach_4(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_3(WHAT, __VA_ARGS__))
#define mc_maf_foreach_5(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_4(WHAT, __VA_ARGS__))
#define mc_maf_foreach_6(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_5(WHAT, __VA_ARGS__))
#define mc_maf_foreach_7(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_6(WHAT, __VA_ARGS__))
#define mc_maf_foreach_8(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_7(WHAT, __VA_ARGS__))
#define mc_maf_foreach_9(WHAT, X, ...)                                         \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_8(WHAT, __VA_ARGS__))
#define mc_maf_foreach_10(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_9(WHAT, __VA_ARGS__))
#define mc_maf_foreach_11(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_10(WHAT, __VA_ARGS__))
#define mc_maf_foreach_12(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_11(WHAT, __VA_ARGS__))
#define mc_maf_foreach_13(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_12(WHAT, __VA_ARGS__))
#define mc_maf_foreach_14(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_13(WHAT, __VA_ARGS__))
#define mc_maf_foreach_15(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_14(WHAT, __VA_ARGS__))
#define mc_maf_foreach_16(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_15(WHAT, __VA_ARGS__))
#define mc_maf_foreach_17(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_16(WHAT, __VA_ARGS__))
#define mc_maf_foreach_18(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_17(WHAT, __VA_ARGS__))
#define mc_maf_foreach_19(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_18(WHAT, __VA_ARGS__))
#define mc_maf_foreach_20(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_19(WHAT, __VA_ARGS__))
#define mc_maf_foreach_21(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_20(WHAT, __VA_ARGS__))
#define mc_maf_foreach_22(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_21(WHAT, __VA_ARGS__))
#define mc_maf_foreach_23(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_22(WHAT, __VA_ARGS__))
#define mc_maf_foreach_24(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_23(WHAT, __VA_ARGS__))
#define mc_maf_foreach_25(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_24(WHAT, __VA_ARGS__))
#define mc_maf_foreach_26(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_25(WHAT, __VA_ARGS__))
#define mc_maf_foreach_27(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_26(WHAT, __VA_ARGS__))
#define mc_maf_foreach_28(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_27(WHAT, __VA_ARGS__))
#define mc_maf_foreach_29(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_28(WHAT, __VA_ARGS__))
#define mc_maf_foreach_30(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_29(WHAT, __VA_ARGS__))
#define mc_maf_foreach_31(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_30(WHAT, __VA_ARGS__))
#define mc_maf_foreach_32(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_31(WHAT, __VA_ARGS__))
#define mc_maf_foreach_33(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_32(WHAT, __VA_ARGS__))
#define mc_maf_foreach_34(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_33(WHAT, __VA_ARGS__))
#define mc_maf_foreach_35(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_34(WHAT, __VA_ARGS__))
#define mc_maf_foreach_36(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_35(WHAT, __VA_ARGS__))
#define mc_maf_foreach_37(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_36(WHAT, __VA_ARGS__))
#define mc_maf_foreach_38(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_37(WHAT, __VA_ARGS__))
#define mc_maf_foreach_39(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_38(WHAT, __VA_ARGS__))
#define mc_maf_foreach_40(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_39(WHAT, __VA_ARGS__))
#define mc_maf_foreach_41(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_40(WHAT, __VA_ARGS__))
#define mc_maf_foreach_42(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_41(WHAT, __VA_ARGS__))
#define mc_maf_foreach_43(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_42(WHAT, __VA_ARGS__))
#define mc_maf_foreach_44(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_43(WHAT, __VA_ARGS__))
#define mc_maf_foreach_45(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_44(WHAT, __VA_ARGS__))
#define mc_maf_foreach_46(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_45(WHAT, __VA_ARGS__))
#define mc_maf_foreach_47(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_46(WHAT, __VA_ARGS__))
#define mc_maf_foreach_48(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_47(WHAT, __VA_ARGS__))
#define mc_maf_foreach_49(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_48(WHAT, __VA_ARGS__))
#define mc_maf_foreach_50(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_49(WHAT, __VA_ARGS__))
#define mc_maf_foreach_51(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_50(WHAT, __VA_ARGS__))
#define mc_maf_foreach_52(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_51(WHAT, __VA_ARGS__))
#define mc_maf_foreach_53(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_52(WHAT, __VA_ARGS__))
#define mc_maf_foreach_54(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_53(WHAT, __VA_ARGS__))
#define mc_maf_foreach_55(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_54(WHAT, __VA_ARGS__))
#define mc_maf_foreach_56(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_55(WHAT, __VA_ARGS__))
#define mc_maf_foreach_57(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_56(WHAT, __VA_ARGS__))
#define mc_maf_foreach_58(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_57(WHAT, __VA_ARGS__))
#define mc_maf_foreach_59(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_58(WHAT, __VA_ARGS__))
#define mc_maf_foreach_60(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_59(WHAT, __VA_ARGS__))
#define mc_maf_foreach_61(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_60(WHAT, __VA_ARGS__))
#define mc_maf_foreach_62(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_61(WHAT, __VA_ARGS__))
#define mc_maf_foreach_63(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_62(WHAT, __VA_ARGS__))
#define mc_maf_foreach_64(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_63(WHAT, __VA_ARGS__))
#define mc_maf_foreach_65(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_64(WHAT, __VA_ARGS__))
#define mc_maf_foreach_66(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_65(WHAT, __VA_ARGS__))
#define mc_maf_foreach_67(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_66(WHAT, __VA_ARGS__))
#define mc_maf_foreach_68(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_67(WHAT, __VA_ARGS__))
#define mc_maf_foreach_69(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_68(WHAT, __VA_ARGS__))
#define mc_maf_foreach_70(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_69(WHAT, __VA_ARGS__))
#define mc_maf_foreach_71(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_70(WHAT, __VA_ARGS__))
#define mc_maf_foreach_72(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_71(WHAT, __VA_ARGS__))
#define mc_maf_foreach_73(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_72(WHAT, __VA_ARGS__))
#define mc_maf_foreach_74(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_73(WHAT, __VA_ARGS__))
#define mc_maf_foreach_75(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_74(WHAT, __VA_ARGS__))
#define mc_maf_foreach_76(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_75(WHAT, __VA_ARGS__))
#define mc_maf_foreach_77(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_76(WHAT, __VA_ARGS__))
#define mc_maf_foreach_78(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_77(WHAT, __VA_ARGS__))
#define mc_maf_foreach_79(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_78(WHAT, __VA_ARGS__))
#define mc_maf_foreach_80(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_79(WHAT, __VA_ARGS__))
#define mc_maf_foreach_81(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_80(WHAT, __VA_ARGS__))
#define mc_maf_foreach_82(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_81(WHAT, __VA_ARGS__))
#define mc_maf_foreach_83(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_82(WHAT, __VA_ARGS__))
#define mc_maf_foreach_84(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_83(WHAT, __VA_ARGS__))
#define mc_maf_foreach_85(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_84(WHAT, __VA_ARGS__))
#define mc_maf_foreach_86(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_85(WHAT, __VA_ARGS__))
#define mc_maf_foreach_87(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_86(WHAT, __VA_ARGS__))
#define mc_maf_foreach_88(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_87(WHAT, __VA_ARGS__))
#define mc_maf_foreach_89(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_88(WHAT, __VA_ARGS__))
#define mc_maf_foreach_90(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_89(WHAT, __VA_ARGS__))
#define mc_maf_foreach_91(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_90(WHAT, __VA_ARGS__))
#define mc_maf_foreach_92(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_91(WHAT, __VA_ARGS__))
#define mc_maf_foreach_93(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_92(WHAT, __VA_ARGS__))
#define mc_maf_foreach_94(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_93(WHAT, __VA_ARGS__))
#define mc_maf_foreach_95(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_94(WHAT, __VA_ARGS__))
#define mc_maf_foreach_96(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_95(WHAT, __VA_ARGS__))
#define mc_maf_foreach_97(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_96(WHAT, __VA_ARGS__))
#define mc_maf_foreach_98(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_97(WHAT, __VA_ARGS__))
#define mc_maf_foreach_99(WHAT, X, ...)                                        \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_98(WHAT, __VA_ARGS__))
#define mc_maf_foreach_100(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_99(WHAT, __VA_ARGS__))
#define mc_maf_foreach_101(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_100(WHAT, __VA_ARGS__))
#define mc_maf_foreach_102(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_101(WHAT, __VA_ARGS__))
#define mc_maf_foreach_103(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_102(WHAT, __VA_ARGS__))
#define mc_maf_foreach_104(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_103(WHAT, __VA_ARGS__))
#define mc_maf_foreach_105(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_104(WHAT, __VA_ARGS__))
#define mc_maf_foreach_106(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_105(WHAT, __VA_ARGS__))
#define mc_maf_foreach_107(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_106(WHAT, __VA_ARGS__))
#define mc_maf_foreach_108(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_107(WHAT, __VA_ARGS__))
#define mc_maf_foreach_109(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_108(WHAT, __VA_ARGS__))
#define mc_maf_foreach_110(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_109(WHAT, __VA_ARGS__))
#define mc_maf_foreach_111(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_110(WHAT, __VA_ARGS__))
#define mc_maf_foreach_112(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_111(WHAT, __VA_ARGS__))
#define mc_maf_foreach_113(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_112(WHAT, __VA_ARGS__))
#define mc_maf_foreach_114(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_113(WHAT, __VA_ARGS__))
#define mc_maf_foreach_115(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_114(WHAT, __VA_ARGS__))
#define mc_maf_foreach_116(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_115(WHAT, __VA_ARGS__))
#define mc_maf_foreach_117(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_116(WHAT, __VA_ARGS__))
#define mc_maf_foreach_118(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_117(WHAT, __VA_ARGS__))
#define mc_maf_foreach_119(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_118(WHAT, __VA_ARGS__))
#define mc_maf_foreach_120(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_119(WHAT, __VA_ARGS__))
#define mc_maf_foreach_121(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_120(WHAT, __VA_ARGS__))
#define mc_maf_foreach_122(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_121(WHAT, __VA_ARGS__))
#define mc_maf_foreach_123(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_122(WHAT, __VA_ARGS__))
#define mc_maf_foreach_124(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_123(WHAT, __VA_ARGS__))
#define mc_maf_foreach_125(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_124(WHAT, __VA_ARGS__))
#define mc_maf_foreach_126(WHAT, X, ...)                                       \
  WHAT(X) mc_maf_msvc_expand_va_args(mc_maf_foreach_125(WHAT, __VA_ARGS__))

// Make a FOREACH macro
#define mc_maf_foreach_i_1(WHAT, X) WHAT(X, 1)
#define mc_maf_foreach_i_2(WHAT, X, ...)                                       \
  WHAT(X, 2) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_1(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_3(WHAT, X, ...)                                       \
  WHAT(X, 3) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_2(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_4(WHAT, X, ...)                                       \
  WHAT(X, 4) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_3(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_5(WHAT, X, ...)                                       \
  WHAT(X, 5) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_4(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_6(WHAT, X, ...)                                       \
  WHAT(X, 6) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_5(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_7(WHAT, X, ...)                                       \
  WHAT(X, 7) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_6(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_8(WHAT, X, ...)                                       \
  WHAT(X, 8) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_7(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_9(WHAT, X, ...)                                       \
  WHAT(X, 9) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_8(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_10(WHAT, X, ...)                                      \
  WHAT(X, 10) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_9(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_11(WHAT, X, ...)                                      \
  WHAT(X, 11) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_10(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_12(WHAT, X, ...)                                      \
  WHAT(X, 12) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_11(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_13(WHAT, X, ...)                                      \
  WHAT(X, 13) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_12(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_14(WHAT, X, ...)                                      \
  WHAT(X, 14) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_13(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_15(WHAT, X, ...)                                      \
  WHAT(X, 15) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_14(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_16(WHAT, X, ...)                                      \
  WHAT(X, 16) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_15(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_17(WHAT, X, ...)                                      \
  WHAT(X, 17) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_16(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_18(WHAT, X, ...)                                      \
  WHAT(X, 18) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_17(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_19(WHAT, X, ...)                                      \
  WHAT(X, 19) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_18(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_20(WHAT, X, ...)                                      \
  WHAT(X, 20) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_19(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_21(WHAT, X, ...)                                      \
  WHAT(X, 21) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_20(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_22(WHAT, X, ...)                                      \
  WHAT(X, 22) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_21(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_23(WHAT, X, ...)                                      \
  WHAT(X, 23) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_22(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_24(WHAT, X, ...)                                      \
  WHAT(X, 24) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_23(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_25(WHAT, X, ...)                                      \
  WHAT(X, 25) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_24(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_26(WHAT, X, ...)                                      \
  WHAT(X, 26) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_25(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_27(WHAT, X, ...)                                      \
  WHAT(X, 27) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_26(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_28(WHAT, X, ...)                                      \
  WHAT(X, 28) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_27(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_29(WHAT, X, ...)                                      \
  WHAT(X, 29) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_28(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_30(WHAT, X, ...)                                      \
  WHAT(X, 30) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_29(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_31(WHAT, X, ...)                                      \
  WHAT(X, 31) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_30(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_32(WHAT, X, ...)                                      \
  WHAT(X, 32) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_31(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_33(WHAT, X, ...)                                      \
  WHAT(X, 33) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_32(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_34(WHAT, X, ...)                                      \
  WHAT(X, 34) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_33(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_35(WHAT, X, ...)                                      \
  WHAT(X, 35) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_34(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_36(WHAT, X, ...)                                      \
  WHAT(X, 36) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_35(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_37(WHAT, X, ...)                                      \
  WHAT(X, 37) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_36(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_38(WHAT, X, ...)                                      \
  WHAT(X, 38) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_37(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_39(WHAT, X, ...)                                      \
  WHAT(X, 39) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_38(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_40(WHAT, X, ...)                                      \
  WHAT(X, 40) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_39(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_41(WHAT, X, ...)                                      \
  WHAT(X, 41) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_40(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_42(WHAT, X, ...)                                      \
  WHAT(X, 42) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_41(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_43(WHAT, X, ...)                                      \
  WHAT(X, 43) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_42(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_44(WHAT, X, ...)                                      \
  WHAT(X, 44) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_43(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_45(WHAT, X, ...)                                      \
  WHAT(X, 45) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_44(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_46(WHAT, X, ...)                                      \
  WHAT(X, 46) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_45(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_47(WHAT, X, ...)                                      \
  WHAT(X, 47) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_46(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_48(WHAT, X, ...)                                      \
  WHAT(X, 48) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_47(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_49(WHAT, X, ...)                                      \
  WHAT(X, 49) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_48(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_50(WHAT, X, ...)                                      \
  WHAT(X, 50) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_49(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_51(WHAT, X, ...)                                      \
  WHAT(X, 51) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_50(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_52(WHAT, X, ...)                                      \
  WHAT(X, 52) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_51(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_53(WHAT, X, ...)                                      \
  WHAT(X, 53) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_52(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_54(WHAT, X, ...)                                      \
  WHAT(X, 54) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_53(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_55(WHAT, X, ...)                                      \
  WHAT(X, 55) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_54(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_56(WHAT, X, ...)                                      \
  WHAT(X, 56) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_55(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_57(WHAT, X, ...)                                      \
  WHAT(X, 57) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_56(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_58(WHAT, X, ...)                                      \
  WHAT(X, 58) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_57(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_59(WHAT, X, ...)                                      \
  WHAT(X, 59) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_58(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_60(WHAT, X, ...)                                      \
  WHAT(X, 60) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_59(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_61(WHAT, X, ...)                                      \
  WHAT(X, 61) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_60(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_62(WHAT, X, ...)                                      \
  WHAT(X, 62) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_61(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_63(WHAT, X, ...)                                      \
  WHAT(X, 63) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_62(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_64(WHAT, X, ...)                                      \
  WHAT(X, 64) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_63(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_65(WHAT, X, ...)                                      \
  WHAT(X, 65) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_64(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_66(WHAT, X, ...)                                      \
  WHAT(X, 66) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_65(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_67(WHAT, X, ...)                                      \
  WHAT(X, 67) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_66(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_68(WHAT, X, ...)                                      \
  WHAT(X, 68) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_67(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_69(WHAT, X, ...)                                      \
  WHAT(X, 69) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_68(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_70(WHAT, X, ...)                                      \
  WHAT(X, 70) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_69(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_71(WHAT, X, ...)                                      \
  WHAT(X, 71) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_70(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_72(WHAT, X, ...)                                      \
  WHAT(X, 72) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_71(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_73(WHAT, X, ...)                                      \
  WHAT(X, 73) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_72(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_74(WHAT, X, ...)                                      \
  WHAT(X, 74) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_73(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_75(WHAT, X, ...)                                      \
  WHAT(X, 75) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_74(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_76(WHAT, X, ...)                                      \
  WHAT(X, 76) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_75(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_77(WHAT, X, ...)                                      \
  WHAT(X, 77) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_76(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_78(WHAT, X, ...)                                      \
  WHAT(X, 78) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_77(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_79(WHAT, X, ...)                                      \
  WHAT(X, 79) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_78(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_80(WHAT, X, ...)                                      \
  WHAT(X, 80) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_79(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_81(WHAT, X, ...)                                      \
  WHAT(X, 81) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_80(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_82(WHAT, X, ...)                                      \
  WHAT(X, 82) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_81(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_83(WHAT, X, ...)                                      \
  WHAT(X, 83) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_82(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_84(WHAT, X, ...)                                      \
  WHAT(X, 84) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_83(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_85(WHAT, X, ...)                                      \
  WHAT(X, 85) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_84(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_86(WHAT, X, ...)                                      \
  WHAT(X, 86) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_85(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_87(WHAT, X, ...)                                      \
  WHAT(X, 87) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_86(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_88(WHAT, X, ...)                                      \
  WHAT(X, 88) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_87(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_89(WHAT, X, ...)                                      \
  WHAT(X, 89) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_88(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_90(WHAT, X, ...)                                      \
  WHAT(X, 90) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_89(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_91(WHAT, X, ...)                                      \
  WHAT(X, 91) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_90(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_92(WHAT, X, ...)                                      \
  WHAT(X, 92) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_91(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_93(WHAT, X, ...)                                      \
  WHAT(X, 93) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_92(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_94(WHAT, X, ...)                                      \
  WHAT(X, 94) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_93(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_95(WHAT, X, ...)                                      \
  WHAT(X, 95) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_94(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_96(WHAT, X, ...)                                      \
  WHAT(X, 96) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_95(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_97(WHAT, X, ...)                                      \
  WHAT(X, 97) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_96(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_98(WHAT, X, ...)                                      \
  WHAT(X, 98) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_97(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_99(WHAT, X, ...)                                      \
  WHAT(X, 99) mc_maf_msvc_expand_va_args(mc_maf_foreach_i_98(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_100(WHAT, X, ...)                                     \
  WHAT(X, 100)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_99(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_101(WHAT, X, ...)                                     \
  WHAT(X, 101)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_100(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_102(WHAT, X, ...)                                     \
  WHAT(X, 102)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_101(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_103(WHAT, X, ...)                                     \
  WHAT(X, 103)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_102(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_104(WHAT, X, ...)                                     \
  WHAT(X, 104)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_103(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_105(WHAT, X, ...)                                     \
  WHAT(X, 105)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_104(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_106(WHAT, X, ...)                                     \
  WHAT(X, 106)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_105(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_107(WHAT, X, ...)                                     \
  WHAT(X, 107)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_106(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_108(WHAT, X, ...)                                     \
  WHAT(X, 108)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_107(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_109(WHAT, X, ...)                                     \
  WHAT(X, 109)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_108(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_110(WHAT, X, ...)                                     \
  WHAT(X, 110)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_109(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_111(WHAT, X, ...)                                     \
  WHAT(X, 111)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_110(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_112(WHAT, X, ...)                                     \
  WHAT(X, 112)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_111(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_113(WHAT, X, ...)                                     \
  WHAT(X, 113)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_112(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_114(WHAT, X, ...)                                     \
  WHAT(X, 114)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_113(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_115(WHAT, X, ...)                                     \
  WHAT(X, 115)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_114(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_116(WHAT, X, ...)                                     \
  WHAT(X, 116)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_115(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_117(WHAT, X, ...)                                     \
  WHAT(X, 117)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_116(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_118(WHAT, X, ...)                                     \
  WHAT(X, 118)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_117(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_119(WHAT, X, ...)                                     \
  WHAT(X, 119)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_118(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_120(WHAT, X, ...)                                     \
  WHAT(X, 120)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_119(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_121(WHAT, X, ...)                                     \
  WHAT(X, 121)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_120(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_122(WHAT, X, ...)                                     \
  WHAT(X, 122)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_121(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_123(WHAT, X, ...)                                     \
  WHAT(X, 123)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_122(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_124(WHAT, X, ...)                                     \
  WHAT(X, 124)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_123(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_125(WHAT, X, ...)                                     \
  WHAT(X, 125)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_124(WHAT, __VA_ARGS__))
#define mc_maf_foreach_i_126(WHAT, X, ...)                                     \
  WHAT(X, 126)                                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_foreach_i_125(WHAT, __VA_ARGS__))

//... repeat as needed

#define mc_maf_get_macro(                                                      \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
    _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, \
    _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, \
    _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, \
    _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, \
    _92, _93, _94, _95, _96, _97, _98, _99, _100, _101, _102, _103, _104,      \
    _105, _106, _107, _108, _109, _110, _111, _112, _113, _114, _115, _116,    \
    _117, _118, _119, _120, _121, _122, _123, _124, _125, NAME, ...)           \
  NAME

#define mc_maf_count_args(...)                                                 \
  mc_maf_msvc_expand_va_args(mc_maf_get_macro(                                 \
      __VA_ARGS__, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, \
      113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,    \
      99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82,  \
      81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64,  \
      63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,  \
      45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28,  \
      27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  \
      9, 8, 7, 6, 5, 4, 3, 2, 1))

#define mc_maf_for_each(action, ...)                                           \
  mc_maf_msvc_expand_va_args(mc_maf_get_macro(                                 \
      __VA_ARGS__, mc_maf_foreach_125, mc_maf_foreach_124, mc_maf_foreach_123, \
      mc_maf_foreach_122, mc_maf_foreach_121, mc_maf_foreach_120,              \
      mc_maf_foreach_119, mc_maf_foreach_118, mc_maf_foreach_117,              \
      mc_maf_foreach_116, mc_maf_foreach_115, mc_maf_foreach_114,              \
      mc_maf_foreach_113, mc_maf_foreach_112, mc_maf_foreach_111,              \
      mc_maf_foreach_110, mc_maf_foreach_109, mc_maf_foreach_108,              \
      mc_maf_foreach_107, mc_maf_foreach_106, mc_maf_foreach_105,              \
      mc_maf_foreach_104, mc_maf_foreach_103, mc_maf_foreach_102,              \
      mc_maf_foreach_101, mc_maf_foreach_100, mc_maf_foreach_99,               \
      mc_maf_foreach_98, mc_maf_foreach_97, mc_maf_foreach_96,                 \
      mc_maf_foreach_95, mc_maf_foreach_94, mc_maf_foreach_93,                 \
      mc_maf_foreach_92, mc_maf_foreach_91, mc_maf_foreach_90,                 \
      mc_maf_foreach_89, mc_maf_foreach_88, mc_maf_foreach_87,                 \
      mc_maf_foreach_86, mc_maf_foreach_85, mc_maf_foreach_84,                 \
      mc_maf_foreach_83, mc_maf_foreach_82, mc_maf_foreach_81,                 \
      mc_maf_foreach_80, mc_maf_foreach_79, mc_maf_foreach_78,                 \
      mc_maf_foreach_77, mc_maf_foreach_76, mc_maf_foreach_75,                 \
      mc_maf_foreach_74, mc_maf_foreach_73, mc_maf_foreach_72,                 \
      mc_maf_foreach_71, mc_maf_foreach_70, mc_maf_foreach_69,                 \
      mc_maf_foreach_68, mc_maf_foreach_67, mc_maf_foreach_66,                 \
      mc_maf_foreach_65, mc_maf_foreach_64, mc_maf_foreach_63,                 \
      mc_maf_foreach_62, mc_maf_foreach_61, mc_maf_foreach_60,                 \
      mc_maf_foreach_59, mc_maf_foreach_58, mc_maf_foreach_57,                 \
      mc_maf_foreach_56, mc_maf_foreach_55, mc_maf_foreach_54,                 \
      mc_maf_foreach_53, mc_maf_foreach_52, mc_maf_foreach_51,                 \
      mc_maf_foreach_50, mc_maf_foreach_49, mc_maf_foreach_48,                 \
      mc_maf_foreach_47, mc_maf_foreach_46, mc_maf_foreach_45,                 \
      mc_maf_foreach_44, mc_maf_foreach_43, mc_maf_foreach_42,                 \
      mc_maf_foreach_41, mc_maf_foreach_40, mc_maf_foreach_39,                 \
      mc_maf_foreach_38, mc_maf_foreach_37, mc_maf_foreach_36,                 \
      mc_maf_foreach_35, mc_maf_foreach_34, mc_maf_foreach_33,                 \
      mc_maf_foreach_32, mc_maf_foreach_31, mc_maf_foreach_30,                 \
      mc_maf_foreach_29, mc_maf_foreach_28, mc_maf_foreach_27,                 \
      mc_maf_foreach_26, mc_maf_foreach_25, mc_maf_foreach_24,                 \
      mc_maf_foreach_23, mc_maf_foreach_22, mc_maf_foreach_21,                 \
      mc_maf_foreach_20, mc_maf_foreach_19, mc_maf_foreach_18,                 \
      mc_maf_foreach_17, mc_maf_foreach_16, mc_maf_foreach_15,                 \
      mc_maf_foreach_14, mc_maf_foreach_13, mc_maf_foreach_12,                 \
      mc_maf_foreach_11, mc_maf_foreach_10, mc_maf_foreach_9,                  \
      mc_maf_foreach_8, mc_maf_foreach_7, mc_maf_foreach_6, mc_maf_foreach_5,  \
      mc_maf_foreach_4, mc_maf_foreach_3, mc_maf_foreach_2,                    \
      mc_maf_foreach_1)(action, __VA_ARGS__))

#define mc_maf_for_each_idx(action, ...)                                       \
  mc_maf_msvc_expand_va_args(mc_maf_get_macro(                                 \
      __VA_ARGS__, mc_maf_foreach_i_125, mc_maf_foreach_i_124,                 \
      mc_maf_foreach_i_123, mc_maf_foreach_i_122, mc_maf_foreach_i_121,        \
      mc_maf_foreach_i_120, mc_maf_foreach_i_119, mc_maf_foreach_i_118,        \
      mc_maf_foreach_i_117, mc_maf_foreach_i_116, mc_maf_foreach_i_115,        \
      mc_maf_foreach_i_114, mc_maf_foreach_i_113, mc_maf_foreach_i_112,        \
      mc_maf_foreach_i_111, mc_maf_foreach_i_110, mc_maf_foreach_i_109,        \
      mc_maf_foreach_i_108, mc_maf_foreach_i_107, mc_maf_foreach_i_106,        \
      mc_maf_foreach_i_105, mc_maf_foreach_i_104, mc_maf_foreach_i_103,        \
      mc_maf_foreach_i_102, mc_maf_foreach_i_101, mc_maf_foreach_i_100,        \
      mc_maf_foreach_i_99, mc_maf_foreach_i_98, mc_maf_foreach_i_97,           \
      mc_maf_foreach_i_96, mc_maf_foreach_i_95, mc_maf_foreach_i_94,           \
      mc_maf_foreach_i_93, mc_maf_foreach_i_92, mc_maf_foreach_i_91,           \
      mc_maf_foreach_i_90, mc_maf_foreach_i_89, mc_maf_foreach_i_88,           \
      mc_maf_foreach_i_87, mc_maf_foreach_i_86, mc_maf_foreach_i_85,           \
      mc_maf_foreach_i_84, mc_maf_foreach_i_83, mc_maf_foreach_i_82,           \
      mc_maf_foreach_i_81, mc_maf_foreach_i_80, mc_maf_foreach_i_79,           \
      mc_maf_foreach_i_78, mc_maf_foreach_i_77, mc_maf_foreach_i_76,           \
      mc_maf_foreach_i_75, mc_maf_foreach_i_74, mc_maf_foreach_i_73,           \
      mc_maf_foreach_i_72, mc_maf_foreach_i_71, mc_maf_foreach_i_70,           \
      mc_maf_foreach_i_69, mc_maf_foreach_i_68, mc_maf_foreach_i_67,           \
      mc_maf_foreach_i_66, mc_maf_foreach_i_65, mc_maf_foreach_i_64,           \
      mc_maf_foreach_i_63, mc_maf_foreach_i_62, mc_maf_foreach_i_61,           \
      mc_maf_foreach_i_60, mc_maf_foreach_i_59, mc_maf_foreach_i_58,           \
      mc_maf_foreach_i_57, mc_maf_foreach_i_56, mc_maf_foreach_i_55,           \
      mc_maf_foreach_i_54, mc_maf_foreach_i_53, mc_maf_foreach_i_52,           \
      mc_maf_foreach_i_51, mc_maf_foreach_i_50, mc_maf_foreach_i_49,           \
      mc_maf_foreach_i_48, mc_maf_foreach_i_47, mc_maf_foreach_i_46,           \
      mc_maf_foreach_i_45, mc_maf_foreach_i_44, mc_maf_foreach_i_43,           \
      mc_maf_foreach_i_42, mc_maf_foreach_i_41, mc_maf_foreach_i_40,           \
      mc_maf_foreach_i_39, mc_maf_foreach_i_38, mc_maf_foreach_i_37,           \
      mc_maf_foreach_i_36, mc_maf_foreach_i_35, mc_maf_foreach_i_34,           \
      mc_maf_foreach_i_33, mc_maf_foreach_i_32, mc_maf_foreach_i_31,           \
      mc_maf_foreach_i_30, mc_maf_foreach_i_29, mc_maf_foreach_i_28,           \
      mc_maf_foreach_i_27, mc_maf_foreach_i_26, mc_maf_foreach_i_25,           \
      mc_maf_foreach_i_24, mc_maf_foreach_i_23, mc_maf_foreach_i_22,           \
      mc_maf_foreach_i_21, mc_maf_foreach_i_20, mc_maf_foreach_i_19,           \
      mc_maf_foreach_i_18, mc_maf_foreach_i_17, mc_maf_foreach_i_16,           \
      mc_maf_foreach_i_15, mc_maf_foreach_i_14, mc_maf_foreach_i_13,           \
      mc_maf_foreach_i_12, mc_maf_foreach_i_11, mc_maf_foreach_i_10,           \
      mc_maf_foreach_i_9, mc_maf_foreach_i_8, mc_maf_foreach_i_7,              \
      mc_maf_foreach_i_6, mc_maf_foreach_i_5, mc_maf_foreach_i_4,              \
      mc_maf_foreach_i_3, mc_maf_foreach_i_2,                                  \
      mc_maf_foreach_i_1)(action, __VA_ARGS__))

#define mc_remove_first_arg_(first, ...) __VA_ARGS__
#define mc_remove_first_arg(...)                                               \
  mc_maf_msvc_expand_va_args(mc_remove_first_arg_(__VA_ARGS__))
