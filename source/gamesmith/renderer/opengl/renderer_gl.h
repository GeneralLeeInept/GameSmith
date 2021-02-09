#pragma once

#include "gspch.h"

namespace GameSmith
{

class Window;

class RendererGL
{
public:
    bool Init(const Window* window);

    void BeginFrame();
    void EndFrame();

private:
    HWND hwnd_{};
    HDC dc_{};
    HGLRC rc_{};
};

}
