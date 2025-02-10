/**
 * @file warnings.h
 * @author Derek Huang
 * @brief C/C++ header for compiler warning suppression macros
 * @copyright MIT License
 */

#ifndef PDXKA_WARNINGS_H_
#define PDXKA_WARNINGS_H_

// MSVC warning control
#if defined(_MSC_VER)
/**
 * Push current warning settings.
 */
#define PDXKA_MSVC_WARNING_PUSH() __pragma(warning(push))

/**
 * Disable the specified space-separated list of warning numbers.
 *
 * @param wnos MSVC warning numbers, e.g. 4820, 5294
 */
#define PDXKA_MSVC_WARNING_DISABLE(wnos) __pragma(warning(disable: wnos))

/**
 * Pop the current warning settings.
 */
#define PDXKA_MSVC_WARNING_POP() __pragma(warning(pop))
#else
#define PDXKA_MSVC_WARNING_PUSH()
#define PDXKA_MSVC_WARNING_DISABLE(wnos)
#define PDXKA_MSVC_WARNING_POP()
#endif  // !defined(_MSC_VER)

#endif  // PDXKA_WARNINGS_H_
