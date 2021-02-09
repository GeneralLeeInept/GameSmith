#pragma once

#include "gspch.h"

namespace GameSmith
{

class Window
{
public:
    using NativeHandle = intptr_t;

    virtual ~Window() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void PumpMessages() = 0;

    virtual NativeHandle GetNativeHandle() const = 0;

    static Window* CreateApplicationWindow(const std::string& title, uint32_t width, uint32_t height);
};

} // namespace GameSmith
