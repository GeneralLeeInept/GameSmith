#pragma once

#include "gspch.h"

namespace gs
{

class Window
{
public:
    using NativeHandle = intptr_t;

    virtual ~Window() = default;

    virtual bool isValid() const = 0;

    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;

    virtual void pumpMessages() = 0;

    virtual NativeHandle getNativeHandle() const = 0;

    static Window* createApplicationWindow(const std::string& title, uint32_t width, uint32_t height);
};

} // namespace GameSmith
