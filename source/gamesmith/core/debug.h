#pragma once

#include "gamesmith/core/config.h"

#if GS_ENABLE_ASSERTS
void gsAssertHandler(const char* file, int line, const char* function, const char* message);
#define GS_ASSERT(condition) \
    (void)((!!(condition)) || (gsAssertHandler(__FILE__, __LINE__, (const char*)(__FUNCTION__), #condition), GS_BREAKPOINT(), 0))
#else
#define GS_ASSERT(condition) ((void)0)
#endif
