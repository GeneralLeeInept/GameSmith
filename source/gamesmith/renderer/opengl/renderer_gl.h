#pragma once

#include "gspch.h"

namespace gs
{

class Window;

class RendererGL
{
public:
    bool init(const Window* window);

    void beginFrame();
    void endFrame();

private:
    HWND hwnd_{};
    HDC dc_{};
    HGLRC rc_{};
};

}
