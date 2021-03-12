#pragma once

#include "gamesmith/core/config.h"

namespace gs
{

class Log
{
public:
    static void print(const char* format, ...);
};

} // namespace GameSmith

#if GS_ENABLE_LOGGING
#define GS_LOG_PRINT(level, format, ...) \
    gs::Log::print("%s(%d): %s: [" level "]: " format "\n", __FILE__, __LINE__, (const char*)(__FUNCTION__), __VA_ARGS__)

#define GS_TRACE(format, ...) GS_LOG_PRINT("TRACE", format, __VA_ARGS__)
#define GS_INFO(format, ...) GS_LOG_PRINT("INFO", format, __VA_ARGS__)
#define GS_WARN(format, ...) GS_LOG_PRINT("WARN", format, __VA_ARGS__)
#define GS_ERROR(format, ...) GS_LOG_PRINT("ERROR", format, __VA_ARGS__)

#else

#define GS_TRACE(...)((void)0)
#define GS_INFO(...)((void)0)
#define GS_WARN(...)((void)0)
#define GS_ERROR(...)((void)0)

#endif
