#pragma once

#include "gamesmith/core/window.h"

namespace gs
{

class PlatformWindow : public Window
{
public:
    PlatformWindow() = default;

    bool isValid() const override;

    uint32_t getWidth() const override;
    uint32_t getHeight() const override;

    void pumpMessages() override;

    NativeHandle getNativeHandle() const override;

    bool wantClose_{};

protected:
    HWND hwnd_{};
    uint32_t width_{};
    uint32_t height_{};
    std::wstring title_{};

    static ATOM wnd_class_;

    friend class Window;
};

}