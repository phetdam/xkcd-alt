/**
 * @file dllexport.h
 * @author Derek Huang
 * @brief C/C++ for symbol export macros
 * @copyright MIT License
 */

#ifndef PDKXA_DLLEXPORT_H_
#define PDKXA_DLLEXPORT_H_

// PDXKA_BUILD_DLL should be defined during the pdxka library's own compilation
// if it is being built as a shared library. implies PDXKA_DLL, which should
// be defined whenever the library is being built *or* used as shared
#if defined(PDXKA_BUILD_DLL) && !defined(PDXKA_DLL)
#define PDXKA_DLL
#endif  // defined(PDXKA_BUILD_DLL) && !defined(PDXKA_DLL)

// if building/using pdxka as shared
#ifdef PDXKA_DLL
// MSVC
#if defined(_MSC_VER)
// symbol export when building
#if defined(PDXKA_BUILD_DLL)
#define PDXKA_PUBLIC __declspec(dllexport)
// otherwise, symbol import
#else
#define PDXKA_PUBLIC __declspec(dllimport)
#endif  // !defined(PDXKA_BUILD_DLL)
// GCC/Clang
#elif defined(__GNUC__)
#define PDXKA_PUBLIC __attribute__((visibility("default")))
#endif  // !defined(_MSC_VER) && !defined(__GNUC__)
#endif  // PDXKA_DLL

// otherwise, define to nothing if static library or unsupported compiler
#ifndef PDXKA_PUBLIC
#define PDXKA_PUBLIC
#endif  // PDXKA_PUBLIC

#endif  // PDKXA_DLLEXPORT_H_
