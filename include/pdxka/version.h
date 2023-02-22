/**
 * @file version.h
 * @author Derek Huang
 * @brief Header for version management
 * @copyright MIT License
 */

#ifndef PDXKA_VERSION_H_
#define PDXKA_VERSION_H_

#include <string>

// major, minor, patch versions, real build defined by CMake
#ifndef PDXKA_MAJOR_VERSION
#define PDXKA_MAJOR_VERSION 0
#endif  // PDXKA_MAJOR_VERSION
#ifndef PDXKA_MINOR_VERSION
#define PDXKA_MINOR_VERSION 0
#endif  // PDXKA_MINOR_VERSION
#ifndef PDXKA_PATCH_VERSION
#define PDXKA_PATCH_VERSION 0
#endif  // PDXKA_PATCH_VERSION

namespace pdxka {

/**
 * Version string.
 */
inline const std::string version{
  std::to_string(PDXKA_MAJOR_VERSION) + "." +
  std::to_string(PDXKA_MINOR_VERSION) + "." +
  std::to_string(PDXKA_PATCH_VERSION)
};

/**
 * Version as a C string.
 */
inline const char* c_version = version.c_str();

}  // namespace pdxka

// version as a macro, using the C string
#define PDXKA_VERSION pdxka::c_version

// build version, real build defines as lowercase CMake CMAKE_BUILD_TYPE
#ifndef PDXKA_BUILD_TYPE
#define PDXKA_BUILD_TYPE "debug"
#endif  // PDXKA_BUILD_TYPE

namespace pdxka {

/*
 * Build type string.
 */
inline const std::string build_type{PDXKA_BUILD_TYPE};

}  // namespace pdxka

// system name, real build defines using CMAKE_SYSTEM_NAME
#ifndef PDXKA_SYSTEM_NAME
// try to guess if not defined
#if defined(_WIN32)
#define PDXKA_SYSTEM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
#define PDXKA_SYSTEM_NAME "MacOS"
#elif defined(__CYGWIN__)
#define PDXKA_SYSTEM_NAME "Cygwin"
#elif defined(__linux__)
#define PDXKA_SYSTEM_NAME "Linux"
#else
#define PDXKA_SYSTEM_NAME "unknown"
#endif
#endif  // PDXKA_SYSTEM_NAME

// system version, hard to determine. real build uses CMAKE_SYSTEM_VERSION
#ifndef PDXKA_SYSTEM_VERSION
#define PDXKA_SYSTEM_VERSION "unknown"
#endif  // PDXKA_SYSTEM_VERSION

// system arch, hard to determine. real build uses CMAKE_SYSTEM_PROCESSOR
#ifndef PDXKA_SYSTEM_ARCH
#if defined(_WIN32)
#ifdef _WIN64
#define PDXKA_SYSTEM_ARCH "AMD64"
#else
#define PDXKA_SYSTEM_ARCH "x86"
#endif  // !_WIN64
#elif defined(__linux__)
#define PDXKA_SYSTEM_ARCH "x86_64"
#else
#define PDXKA_SYSTEM_ARCH "unknown"
#endif
#endif  // PDXKA_SYSTEM_ARCH

namespace pdxka {

/*
 * System name string.
 */
inline const std::string system_name{PDXKA_SYSTEM_NAME};

/**
 * System version string.
 */
inline const std::string system_version{PDXKA_SYSTEM_VERSION};

/**
 * System architecture string.
 */
inline const std::string system_arch{PDXKA_SYSTEM_ARCH};

}  // namespace pdxka

#endif  // PDXKA_VERSION_H_
