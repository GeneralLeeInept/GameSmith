#pragma once

#ifndef GS_ENABLE_ASSERTS
#if defined(GS_DEBUG) || defined(GS_DEVELOPMENT)
#define GS_ENABLE_ASSERTS 1
#else
#define GS_ENABLE_ASSERTS 0
#endif
#endif

#ifndef GS_ENABLE_LOGGING
#if defined(GS_DEBUG) || defined(GS_DEVELOPMENT)
#define GS_ENABLE_LOGGING 1
#else
#define GS_ENABLE_LOGGING 0
#endif
#endif

#if defined(_WIN32)
#define GS_PLATFORM_WINDOWS

#if defined(GS_RELEASE)
#define GS_BREAKPOINT() ((void)0)
#else
#define GS_BREAKPOINT() DebugBreak()
#endif

#endif
