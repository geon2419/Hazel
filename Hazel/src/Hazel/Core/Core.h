#pragma once

#include <memory>

#if defined(HZ_PLATFORM_MACOS)
#include <csignal>
#endif

#if defined(HZ_PLATFORM_WINDOWS)
#if HZ_DYNAMIC_LINK
#ifdef HZ_BUILD_DLL
#define HAZEL_API __declspec(dllexport)
#else
#define HAZEL_API __declspec(dllimport)
#endif
#else
#define HAZEL_API
#endif
#elif defined(HZ_PLATFORM_MACOS)
#define HAZEL_API
#endif

#if defined(HZ_PLATFORM_WINDOWS)
#define HZ_DEBUGBREAK() __debugbreak()
#elif defined(HZ_PLATFORM_MACOS)
#define HZ_DEBUGBREAK() raise(SIGTRAP)
#else
#define HZ_DEBUGBREAK()
#endif

#ifdef HZ_ENABLE_ASSERTS
#define HZ_ASSERT(x, ...)                                                                                              \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                            \
            HZ_DEBUGBREAK();                                                                                           \
        }                                                                                                              \
    }
#define HZ_CORE_ASSERT(x, ...)                                                                                         \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                       \
            HZ_DEBUGBREAK();                                                                                           \
        }                                                                                                              \
    }
#else
#define HZ_ASSERT(x, ...)
#define HZ_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define HZ_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace Hazel
{

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T> using Ref = std::shared_ptr<T>;
} // namespace Hazel