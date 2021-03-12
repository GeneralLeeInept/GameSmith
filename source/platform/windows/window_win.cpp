#include "gspch.h"

#include "window_win.h"

#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"

static LRESULT CALLBACK gsWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static std::wstring gsUtf8ToWchar(const std::string& utf8);

namespace gs
{

ATOM PlatformWindow::wnd_class_{};

Window* Window::createApplicationWindow(const std::string& title, uint32_t width, uint32_t height)
{
    if (PlatformWindow::wnd_class_ == 0)
    {
        WNDCLASSEXW wndClassEx{};
        wndClassEx.cbSize = sizeof(WNDCLASSEXW);
        wndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wndClassEx.lpfnWndProc = gsWindowProc;
        wndClassEx.hInstance = GetModuleHandle(NULL);
        wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClassEx.lpszClassName = L"GameSmithWindow";
        PlatformWindow::wnd_class_ = RegisterClassExW(&wndClassEx);

        if (!PlatformWindow::wnd_class_)
        {
            GS_ERROR("RegisterClassExW failed [%x]", GetLastError());
            return nullptr;
        }
    }

    PlatformWindow* window = new PlatformWindow;
    window->title_ = gsUtf8ToWchar(title);
    window->width_ = width;
    window->height_ = height;

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    RECT clientRect = { 0, 0, (LONG)width, (LONG)height };
    AdjustWindowRectEx(&clientRect, dwStyle, FALSE, dwExStyle);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
    window->hwnd_ = CreateWindowExW(dwExStyle, L"GameSmithWindow", window->title_.c_str(), dwStyle, 0, 0, (LONG)width, (LONG)height, NULL, NULL,
                                   GetModuleHandle(NULL), NULL);

    if (!window->hwnd_)
    {
        GS_ERROR("CreateWindowExW failed [%x]", GetLastError());
        delete window;
        return nullptr;
    }

    SetPropW(window->hwnd_, L"GameSmithWindow", window);

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    SetWindowPos(window->hwnd_, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    ShowWindow(window->hwnd_, SW_SHOW);

    window->pumpMessages();

    return window;
}

bool PlatformWindow::isValid() const
{
    return !wantClose_;
}

uint32_t PlatformWindow::getWidth() const
{
    return width_;
}

uint32_t PlatformWindow::getHeight() const
{
    return height_;
}

void PlatformWindow::pumpMessages()
{
    MSG msg{};

    while (PeekMessageW(&msg, hwnd_, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

Window::NativeHandle PlatformWindow::getNativeHandle() const
{
    return (NativeHandle)hwnd_;
}

} // namespace GameSmith

std::wstring gsUtf8ToWchar(const std::string& utf8)
{
    int mbcs_len = MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, NULL, 0);
    std::wstring mbcs;
    mbcs.resize(mbcs_len);
    MultiByteToWideChar(CP_UTF8, 0, &utf8[0], -1, &mbcs[0], mbcs_len);
    return mbcs;
}

LRESULT CALLBACK gsWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    gs::PlatformWindow* window = (gs::PlatformWindow*)GetPropW(hwnd, L"GameSmithWindow");

    switch (msg)
    {
        case WM_ACTIVATEAPP:
        {
           //window->HandleEvent()
            break;
        }
        case WM_CLOSE:
        {
            window->wantClose_ = true;
            return 0L;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}
