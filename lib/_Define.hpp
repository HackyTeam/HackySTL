#pragma once

///
/// @file _Define.hpp
///

#if defined(__GNUC__)
    ///
    /// @brief GCC specific define.
    ///
    #define HSD_COMPILER_GCC
#elif defined(__INTEL_COMPILER)
    ///
    /// @brief ICC specific define.
    ///
    #define HSD_COMPILER_INTEL
#elif defined(__clang__)
    ///
    /// @brief Clang specific define.
    ///
    #define HSD_COMPILER_CLANG
#elif defined(_MSC_VER)
    ///
    /// @brief MSVC specific define.
    ///
    #define HSD_COMPILER_MSVC
#else
    ///
    /// @brief Unknown compiler or compiler not supported.
    ///
    #define HSD_COMPILER_UNKNOWN
#endif

///
/// @brief Definitions for PRETTY_FUNCTION depending on compiler.
///
#if defined(HSD_COMPILER_MSVC)
    #define HSD_FUNCTION_NAME __FUNCSIG__
#elif defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
    #define HSD_FUNCTION_NAME __PRETTY_FUNCTION__
#else
    #define HSD_FUNCTION_NAME __builtin_FUNCTION()
#endif


#if defined(_WIN64) || defined(_WIN32) || defined(__WINDOWS__)
    ///
    /// @brief Windows specific defines for handling
    /// different specific implementations.
    ///
    #define HSD_PLATFORM_WINDOWS
#elif defined(__linux) || defined(linux) || defined(__linux__)
    ///
    /// @brief Linux specific defines for handling
    /// different specific implementations.
    ///
    #define HSD_PLATFORM_LINUX
#elif defined(__DragonFly__) || defined(__FreeBSD__) || \
    defined(__NETBSD__) || defined(__OpenBSD__)
    ///
    /// @brief BSD specific defines for handling
    /// different specific implementations.
    ///
    #define HSD_PLATFORM_BSD
#elif defined(__APPLE__) || defined(macintosh) || defined(__MACH__)
    ///
    /// @brief OSX specific defines for handling
    /// different specific implementations.
    ///
    #define HSD_PLATFORM_OSX
#elif defined(__unix) || defined(unix)
    ///
    /// @brief UNIX specific defines for handling
    /// different specific implementations.
    ///
    #define HSD_PLATFORM_UNIX
#else
    ///
    /// @brief Unknown platform, or platform not supported.
    ///
    #define HSD_PLATFORM_UNKNOWN
#endif

///
/// @brief Defifinition for the platforms that
/// comply with the POSIX standard
///
#if defined(HSD_PLATFORM_LINUX) || defined(HSD_PLATFORM_BSD) || \
    defined(HSD_PLATFORM_OSX) || defined(HSD_PLATFORM_UNIX)
    #define HSD_PLATFORM_POSIX
#endif