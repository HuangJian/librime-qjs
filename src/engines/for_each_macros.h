#pragma once
// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// =============== FOR_EACH ===============
#define EXPAND(...) __VA_ARGS__

// #define FOR_EACH_0(...) // NOT WORKING, c++20's __VA_OPT__ is required
// #define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_OPT__(,) __VA_ARGS__, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define FOR_EACH_1(macro, x) macro(x)
#define FOR_EACH_2(macro, x, ...) macro(x) EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, x, ...) macro(x) EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, x, ...) macro(x) EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH_5(macro, x, ...) macro(x) EXPAND(FOR_EACH_4(macro, __VA_ARGS__))
#define FOR_EACH_6(macro, x, ...) macro(x) EXPAND(FOR_EACH_5(macro, __VA_ARGS__))
#define FOR_EACH_7(macro, x, ...) macro(x) EXPAND(FOR_EACH_6(macro, __VA_ARGS__))
#define FOR_EACH_8(macro, x, ...) macro(x) EXPAND(FOR_EACH_7(macro, __VA_ARGS__))
#define FOR_EACH_9(macro, x, ...) macro(x) EXPAND(FOR_EACH_8(macro, __VA_ARGS__))
#define FOR_EACH_10(macro, x, ...) macro(x) EXPAND(FOR_EACH_9(macro, __VA_ARGS__))
#define FOR_EACH_11(macro, x, ...) macro(x) EXPAND(FOR_EACH_10(macro, __VA_ARGS__))
#define FOR_EACH_12(macro, x, ...) macro(x) EXPAND(FOR_EACH_11(macro, __VA_ARGS__))
#define FOR_EACH_13(macro, x, ...) macro(x) EXPAND(FOR_EACH_12(macro, __VA_ARGS__))
#define FOR_EACH_14(macro, x, ...) macro(x) EXPAND(FOR_EACH_13(macro, __VA_ARGS__))
#define FOR_EACH_15(macro, x, ...) macro(x) EXPAND(FOR_EACH_14(macro, __VA_ARGS__))
#define FOR_EACH_16(macro, x, ...) macro(x) EXPAND(FOR_EACH_15(macro, __VA_ARGS__))
#define FOR_EACH_17(macro, x, ...) macro(x) EXPAND(FOR_EACH_16(macro, __VA_ARGS__))
#define FOR_EACH_18(macro, x, ...) macro(x) EXPAND(FOR_EACH_17(macro, __VA_ARGS__))
#define FOR_EACH_19(macro, x, ...) macro(x) EXPAND(FOR_EACH_18(macro, __VA_ARGS__))
#define FOR_EACH_20(macro, x, ...) macro(x) EXPAND(FOR_EACH_19(macro, __VA_ARGS__))

// Get number of arguments
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, \
                    _18, _19, _20, N, ...)                                                      \
  N
#define COUNT_ARGS(...) \
  COUNT_ARGS_(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Select the appropriate FOR_EACH macro based on argument count
#define INTERNAL_FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) INTERNAL_FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

// =============== FOR_EACH_COMMA ===============
#define FOR_EACH_COMMA_1(macro, x) macro(x)
#define FOR_EACH_COMMA_2(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_1(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_3(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_2(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_4(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_3(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_5(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_4(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_6(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_5(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_7(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_6(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_8(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_7(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_9(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_8(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_10(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_9(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_11(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_10(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_12(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_11(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_13(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_12(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_14(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_13(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_15(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_14(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_16(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_15(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_17(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_16(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_18(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_17(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_19(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_18(macro, __VA_ARGS__))
#define FOR_EACH_COMMA_20(macro, x, ...) macro(x), EXPAND(FOR_EACH_COMMA_19(macro, __VA_ARGS__))

#define INTERNAL_FOR_EACH_COMMA_N(N, macro, ...) FOR_EACH_COMMA_##N(macro, __VA_ARGS__)
#define FOR_EACH_COMMA_N(N, macro, ...) INTERNAL_FOR_EACH_COMMA_N(N, macro, __VA_ARGS__)
#define FOR_EACH_COMMA(macro, ...) FOR_EACH_COMMA_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

// =============== FOR_EACH_PAIR ===============
#define FOR_EACH_PAIR_0(...)
#define FOR_EACH_PAIR_1(macro, x, y) macro(x, y)
#define FOR_EACH_PAIR_2(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_1(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_3(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_2(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_4(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_3(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_5(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_4(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_6(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_5(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_7(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_6(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_8(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_7(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_9(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_8(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_10(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_9(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_11(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_10(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_12(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_11(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_13(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_12(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_14(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_13(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_15(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_14(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_16(macro, x, y, ...) macro(x, y) EXPAND(FOR_EACH_PAIR_15(macro, __VA_ARGS__))

// Get number of argument pairs
#define COUNT_PAIRS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                     _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, \
                     ...)                                                                          \
  N

#define COUNT_PAIRS(...)                                                                           \
  COUNT_PAIRS_(__VA_ARGS__, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, \
               7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 0, 0)

// Select the appropriate FOR_EACH_PAIR macro based on pair count
#define INTERNAL_FOR_EACH_PAIR_N(N, macro, ...) FOR_EACH_PAIR_##N(macro, __VA_ARGS__)
#define FOR_EACH_PAIR_N(N, macro, ...) INTERNAL_FOR_EACH_PAIR_N(N, macro, __VA_ARGS__)
#define FOR_EACH_PAIR(macro, ...) FOR_EACH_PAIR_N(COUNT_PAIRS(__VA_ARGS__), macro, __VA_ARGS__)

// =============== FOR_EACH_PAIR_COMMA ===============
#define FOR_EACH_PAIR_COMMA_1(macro, x, y) macro(x, y)
#define FOR_EACH_PAIR_COMMA_2(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_1(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_3(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_2(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_4(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_3(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_5(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_4(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_6(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_5(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_7(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_6(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_8(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_7(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_9(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_8(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_10(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_9(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_11(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_10(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_12(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_11(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_13(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_12(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_14(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_13(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_15(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_14(macro, __VA_ARGS__))
#define FOR_EACH_PAIR_COMMA_16(macro, x, y, ...) \
  macro(x, y), EXPAND(FOR_EACH_PAIR_COMMA_15(macro, __VA_ARGS__))

#define INTERNAL_FOR_EACH_PAIR_COMMA_N(N, macro, ...) FOR_EACH_PAIR_COMMA_##N(macro, __VA_ARGS__)
#define FOR_EACH_PAIR_COMMA_N(N, macro, ...) INTERNAL_FOR_EACH_PAIR_COMMA_N(N, macro, __VA_ARGS__)
#define FOR_EACH_PAIR_COMMA(macro, ...) \
  FOR_EACH_PAIR_COMMA_N(COUNT_PAIRS(__VA_ARGS__), macro, __VA_ARGS__)

// NOLINTEND(cppcoreguidelines-macro-usage)
