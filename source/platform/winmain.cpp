#include "gspch.h"

#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "gamesmith/core/window.h"
#include "gamesmith/renderer/opengl/renderer_gl.h"
#include "gamesmith/renderer/opengl/glad/glad.h"

struct ComHelper
{
    ComHelper() { CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); }
    ~ComHelper() { CoUninitialize(); }
};

int wWinMainInternal(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    ComHelper comHelper;

    GameSmith::Window* applicationWindow = GameSmith::Window::CreateApplicationWindow("GameSmith Application", 1920, 1080);

    if (!applicationWindow)
    {
        GS_ERROR("Failed to create application window.");
        return -1;
    }

    GameSmith::RendererGL renderer;

    if (!renderer.Init(applicationWindow))
    {
        GS_ERROR("Failed to initialize renderer.");
        return -1;
    }

    while (IsWindow((HWND)applicationWindow->GetNativeHandle()))
    {
        applicationWindow->PumpMessages();
        renderer.BeginFrame();
        glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.EndFrame();
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    int result;

    if (IsDebuggerPresent())
    {
        result = wWinMainInternal(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
    }
    else
    {
        __try
        {
            result = wWinMainInternal(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
        }
        __except (UnhandledExceptionFilter(GetExceptionInformation()))
        {
            MessageBoxW(0, L"An exception occurred. The application will exit.", L"Unhandled exception", MB_ICONERROR | MB_OK);
            result = -1;
        }
    }

    return result;
}
