#include "gspch.h"

#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "gamesmith/core/window.h"

// TESTING/HACKING - get imgui in here
#include "gamesmith/renderer/opengl/renderer_gl.h"
#include "gamesmith/renderer/opengl/glad/glad.h"

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "gamesmith/renderer/opengl/glad/glad.h"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

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

    // IMGUI HACKING
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontDefault();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init((void*)applicationWindow->GetNativeHandle());
    ImGui_ImplOpenGL3_Init();
    ////////////////

    while (IsWindow((HWND)applicationWindow->GetNativeHandle()))
    {
        applicationWindow->PumpMessages();

        // IMGUI HACKING
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        io.MousePos.x = cursor_pos.x;
        io.MousePos.y = cursor_pos.y;
        io.MouseDown[0] = GetKeyState(VK_LBUTTON) & 0x8000;
        io.MouseDown[1] = GetKeyState(VK_RBUTTON) & 0x8000;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Begin("Another Window");
        ImGui::End();

        ImGui::Render();
        ////////////////

        renderer.BeginFrame();
        glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // IMGUI HACKING

        renderer.EndFrame();
    }

    // IMGUI HACKING
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ////////////////

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
