#include "gspch.h"

#include "gamesmith/core/log.h"

#if GS_ENABLE_LOGGING

#include <cstdarg>
#include <string>

namespace GameSmith
{

void Log::Print(const char* format, ...)
{
    static std::string buffer;
    static size_t buffer_size = 128;

    if (buffer.size() == 0)
    {
        buffer.resize(buffer_size);
    }


    for (;;)
    {
        std::va_list args;
        va_start(args, format);
        size_t len = (size_t)(std::vsnprintf(&buffer[0], buffer.size(), format, args)) + 1;
        va_end(args);

        if (len <= buffer.size())
        {
            break;
        }

        len = buffer.size() * 2;
        buffer.resize(len);
    }

    OutputDebugStringA(buffer.c_str());
}

} // namespace GameSmith

#endif
