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
#define PDXKA_CONCAT_T(x, y) x ## y

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

#endif // PDXKA_COMMON_H_
