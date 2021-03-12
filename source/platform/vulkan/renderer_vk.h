#pragma once

#include "gsvulkan.h"

namespace gs
{

class Window;

class RendererVk
{
public:
    static RendererVk* Create() { return nullptr; }

    bool init(const Window* window) { return false; }

    void beginFrame() {}
    void endFrame() {}
};

} // namespace GameSmith
