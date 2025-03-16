/**
 * @file common.h
 * @author Derek Huang
 * @brief C/C++ header for common macros
 * @copyright MIT License
 */

#ifndef PDXKA_COMMON_H_
#define PDXKA_COMMON_H_

/**
 * Concatenate arguments without macro expansion.
 *
 * @param x First argument
 * @param y Second argument
 */
#define PDXKA_CONCAT_I(x, y) x ## y

/**
 * Concatenate arguments with macro expansion.
 *
 * @param x First argument
 * @param y Second argument
 */
#define PDXKA_CONCAT(x, y) PDXKA_CONCAT_I(x, y)

/**
 * Stringify argument without macro expansion.
 *
 * @param x Argument to stringify
 */
#define PDXKA_STRINGIFY_I(x) #x

/**
 * Stringify argument with macro expansion.
 *
 * @param x Argument to stringify
 */
#define PDXKA_STRINGIFY(x) PDXKA_STRINGIFY_I(x)

#ifdef __cplusplus
/**
 * C++ version standard macro.
 *
 * This works correctly on MSVC without requiring `/Zc:__cplusplus`.
 */
#if defined(_MSVC_LANG)
#define PDXKA_CPLUSPLUS _MSVC_LANG
#else
#define PDXKA_CPLUSPLUS __cplusplus
#endif  // !defined(_MSVC_LANG)
#endif  // __cplusplus

/**
 * Function signature macro.
 *
 * If no compiler-specific macro is available, `__func__` is used for C99/C++11.
 * Earlier C/C++ standards will simply have `"(unknown)" used instead.
 */
#if defined(_MSC_VER)
#define PDXKA_PRETTY_FUNCTION_NAME __FUNCSIG__
#elif defined(__GNUC__)
#define PDXKA_PRETTY_FUNCTION_NAME __PRETTY_FUNCTION__
// C++
#elif defined(PDXKA_CPLUSPLUS)
// __func__ is standard in C++11
#if PDXKA_CPLUSPLUS >= 201103L
#define PDXKA_PRETTY_FUNCTION_NAME __func__
#else
#define PDXKA_PRETTY_FUNCTION_NAME "(unknown)"
#endif  // PDXKA_CPLUSPLUS < 201103L
// ANSI C
#elif defined(__STDC_VERSION__)
// __func__ is standard in C99
#if __STDC_VERSION__ >= 199901L
#define PDXKA_PRETTY_FUNCTION_NAME __func__
#else
#define PDXKA_PRETTY_FUNCTION_NAME "(unknown)"
#endif  // __STDC_VERSION__ < 199901L
// K&R C
#else
#define PDXKA_PRETTY_FUNCTION_NAME "(unknown)"
#endif  // !defined(_MSC_VER) && !defined(__GNUC__) &&
        // !defined(PDXKA_CPLUSPLUS) && !defined(__STDC_VERSION__)

#endif // PDXKA_COMMON_H_
