#pragma once

#include "gamesmith/core/window.h"

namespace GameSmith
{

class PlatformWindow : public Window
{
public:
    PlatformWindow() = default;

    uint32_t GetWidth() const override;
    uint32_t GetHeight() const override;

    void PumpMessages() override;

    NativeHandle GetNativeHandle() const override;

protected:
    HWND hwnd_{};
    uint32_t width_{};
    uint32_t height_{};
    std::wstring title_{};

    static ATOM wnd_class_;

    friend class Window;
};

}