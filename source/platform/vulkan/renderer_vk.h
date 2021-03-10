#pragma once

#include "gsvulkan.h"

namespace gs
{

class Window;

class RendererVk
{
public:
    static RendererVk* Create() { return nullptr; }

    bool Init(const Window* window) { return false; }

    void BeginFrame() {}
    void EndFrame() {}
};

} // namespace GameSmith
