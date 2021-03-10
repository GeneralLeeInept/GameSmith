#include "gspch.h"

#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"

void gsAssertHandler(const char* file, int line, const char* function, const char* message)
{
    gs::Log::Print("%s(%d): %s: [ERROR]: ASSERTION FAILED: %s\n", file, line, function, message);
}
