#include "gspch.h"

#include "renderer_gl.h"
#include "glad/glad.h"
#include "wglext.h"

#include "gamesmith/core/debug.h"
#include "gamesmith/core/log.h"
#include "gamesmith/core/window.h"

#if defined(GS_DEBUG) || defined(GS_DEVELOPMENT)
#define GS_ENABLE_OPENGL_DEBUGGING 1
#else
#define GS_ENABLE_OPENGL_DEBUGGING 0
#endif

struct WglHelper
{
    typedef HGLRC(__stdcall* PFNWGLCREATECONTEXT)(HDC);
    typedef int(__stdcall* PFNWGLDELETECONTEXT)(HGLRC);
    typedef HGLRC(__stdcall* PFNWGLGETCURRENTCONTEXT)(void);
    typedef PROC(__stdcall* PFNWGLGETPROCADDRESS)(const char*);
    typedef int(__stdcall* PFNWGLMAKECURRENT)(HDC, HGLRC);

    HMODULE libgl = nullptr;
    PFNWGLCREATECONTEXT CreateContext = nullptr;
    PFNWGLDELETECONTEXT DeleteContext = nullptr;
    PFNWGLGETCURRENTCONTEXT GetCurrentContext = nullptr;
    PFNWGLGETPROCADDRESS GetProcAddress = nullptr;
    PFNWGLMAKECURRENT MakeCurrent = nullptr;
    PFNWGLSWAPINTERVALEXTPROC SwapInterval = nullptr;

    bool Load()
    {
        if (libgl)
        {
            GS_ERROR("Opengl32.dll already loaded.");
            return false;
        }

        libgl = LoadLibraryA("opengl32.dll");

        if (!libgl)
        {
            GS_ERROR("Failed to Load Opengl32.dll.\n");
            return false;
        }

        CreateContext = (PFNWGLCREATECONTEXT)::GetProcAddress(libgl, "wglCreateContext");
        DeleteContext = (PFNWGLDELETECONTEXT)::GetProcAddress(libgl, "wglDeleteContext");
        GetCurrentContext = (PFNWGLGETCURRENTCONTEXT)::GetProcAddress(libgl, "wglGetCurrentContext");
        GetProcAddress = (PFNWGLGETPROCADDRESS)::GetProcAddress(libgl, "wglGetProcAddress");
        MakeCurrent = (PFNWGLMAKECURRENT)::GetProcAddress(libgl, "wglMakeCurrent");

        if (!(CreateContext && DeleteContext && GetProcAddress && MakeCurrent))
        {
            Unload();
            return false;
        }

        return true;
    }

    void Unload()
    {
        if (libgl)
        {
            FreeLibrary(libgl);
            libgl = nullptr;
        }

        CreateContext = nullptr;
        DeleteContext = nullptr;
        GetCurrentContext = nullptr;
        GetProcAddress = nullptr;
        MakeCurrent = nullptr;
    }
};

struct DwmHelper
{
    bool Load()
    {
        if (dwm)
        {
            GS_ERROR("dwmapi.dll already loaded.\n");
            return false;
        }

        dwm = LoadLibraryA("dwmapi.dll");

        if (!dwm)
        {
            GS_ERROR("Failed to Load dwmapi.dll.\n");
            return false;
        }

        Flush = (PFNDWMFLUSH)GetProcAddress(dwm, "DwmFlush");

        if (!Flush)
        {
            Unload();
            return false;
        }

        return true;
    }


    void Unload()
    {
        if (dwm)
        {
            FreeLibrary(dwm);
            dwm = nullptr;
        }

        Flush = nullptr;
    }

    typedef HRESULT(__stdcall* PFNDWMFLUSH)(void);

    HMODULE dwm = nullptr;
    PFNDWMFLUSH Flush = nullptr;
};

static WglHelper gWglHelper{};
static DwmHelper gDwmHelper{};

class WindowCleaner
{
public:
    WindowCleaner(HINSTANCE instance)
        : instance_(instance)
    {
    }


    ~WindowCleaner() { Release(); }


    bool SetWindowClass(ATOM wc)
    {
        wc_ = wc;
        return !!wc_;
    }


    bool SetHwnd(HWND wnd)
    {
        wnd_ = wnd;
        return !!wnd_;
    }


    bool SetDC(HDC dc)
    {
        dc_ = dc;
        return !!dc_;
    }


    bool SetGLRC(HGLRC glrc)
    {
        glrc_ = glrc;
        return !!glrc_;
    }


    void Release()
    {
        if (glrc_)
        {
            if (gWglHelper.GetCurrentContext() == glrc_)
            {
                gWglHelper.MakeCurrent(nullptr, nullptr);
            }

            gWglHelper.DeleteContext(glrc_);
        }

        if (dc_)
        {
            ReleaseDC(wnd_, dc_);
        }

        if (wnd_)
        {
            DestroyWindow(wnd_);
        }

        if (wc_)
        {
            UnregisterClassW(MAKEINTATOM(wc_), instance_);
        }
    }


    explicit operator HINSTANCE() { return instance_; }
    explicit operator HWND() { return wnd_; }
    explicit operator HDC() { return dc_; }
    explicit operator HGLRC() { return glrc_; }

private:
    HINSTANCE instance_{};
    ATOM wc_{};
    HWND wnd_{};
    HDC dc_{};
    HGLRC glrc_{};
};

static bool CreateDummyWindow(WindowCleaner& cleaner)
{
    static LPCWSTR dummy_window_class = L"_datasmith_ogl";
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = (HINSTANCE)cleaner;
    wc.lpszClassName = dummy_window_class;

    if (!cleaner.SetWindowClass(RegisterClassExW(&wc)))
    {
        return false;
    }

    HWND hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, dummy_window_class, dummy_window_class, WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1, 1,
                                nullptr, nullptr, (HINSTANCE)cleaner, nullptr);

    if (!hwnd)
    {
        return false;
    }

    cleaner.SetHwnd(hwnd);
    ShowWindow((HWND)cleaner, SW_HIDE);

    MSG msg;
    while (PeekMessageW(&msg, (HWND)cleaner, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return true;
}


static bool LoadOpenGL(WindowCleaner& cleaner)
{
    if (!cleaner.SetDC(GetDC((HWND)cleaner)))
    {
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat((HDC)cleaner, &pfd);

    if (!pixelFormat)
    {
        return false;
    }

    if (!SetPixelFormat((HDC)cleaner, pixelFormat, &pfd))
    {
        return false;
    }

    HGLRC glrc = gWglHelper.CreateContext((HDC)cleaner);

    if (!glrc)
    {
        GS_ERROR("wglCreateContext failed - %08X", GetLastError());
        return false;
    }

    cleaner.SetGLRC(glrc);

    if (!gWglHelper.MakeCurrent((HDC)cleaner, (HGLRC)cleaner))
    {
        return false;
    }

    if (!gladLoadGL())
    {
        return false;
    }

    gWglHelper.SwapInterval = (PFNWGLSWAPINTERVALEXTPROC)gWglHelper.GetProcAddress("wglSwapIntervalEXT");

    return true;
}

static void APIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                          const void* userParam)
{
    const char* source_string = "???";

    if (source == GL_DEBUG_SOURCE_API)
    {
        source_string = "API";
    }
    else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
    {
        source_string = "WINDOW SYSTEM";
    }
    else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
    {
        source_string = "SHADER COMPILER";
    }
    else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
    {
        source_string = "THIRD PARTY";
    }
    else if (source == GL_DEBUG_SOURCE_APPLICATION)
    {
        source_string = "APPLICATION";
    }
    else if (source == GL_DEBUG_SOURCE_OTHER)
    {
        source_string = "OTHER";
    }

    const char* type_string = "???";

    if (type == GL_DEBUG_TYPE_ERROR)
    {
        type_string = "ERROR";
    }
    else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
    {
        type_string = "DEPRECATED BEHAVIOR";
    }
    else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
    {
        type_string = "UNDEFINED BEHAVIOR";
    }
    else if (type == GL_DEBUG_TYPE_PORTABILITY)
    {
        type_string = "PORTABILITY";
    }
    else if (type == GL_DEBUG_TYPE_PERFORMANCE)
    {
        type_string = "PERFORMANCE";
    }
    else if (type == GL_DEBUG_TYPE_OTHER)
    {
        type_string = "OTHER";
    }

    const char* severity_string = "???";

    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        severity_string = "HIGH";
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        severity_string = "MEDIUM";
    }
    else if (severity == GL_DEBUG_SEVERITY_LOW)
    {
        severity_string = "LOW";
    }
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        severity_string = "NOTIFICATION";
    }

    GS_TRACE("source = 0x%x [%s], type = 0x%x [%s], severity = 0x%x [%s], message = %s", source, source_string, type, type_string, severity,
             severity_string, message);
}

namespace GameSmith
{

bool RendererGL::Init(const Window* window)
{
    HWND hwnd = (HWND)window->GetNativeHandle();

    if (!(gDwmHelper.Load() && gWglHelper.Load()))
    {
        return false;
    }

    HINSTANCE hinstance = GetModuleHandle(nullptr);
    WindowCleaner cleaner(hinstance);

    if (!(CreateDummyWindow(cleaner) && LoadOpenGL(cleaner)))
    {
        return false;
    }

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)gWglHelper.GetProcAddress("wglChoosePixelFormatARB");

    if (!wglChoosePixelFormatARB)
    {
        return false;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)gWglHelper.GetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB)
    {
        return false;
    }

    cleaner.Release();

    class LocalDc
    {
    public:
        LocalDc(HWND wnd)
            : wnd_(wnd)
        {
            dc_ = GetDC(wnd);
        }

        ~LocalDc()
        {
            if (dc_)
            {
                ReleaseDC(wnd_, dc_);
            }
        }

        void Reset() { dc_ = nullptr; }

        operator bool() { return !!dc_; }
        operator HDC() { return dc_; }

    private:
        HWND wnd_;
        HDC dc_;
    };

    LocalDc dc(hwnd);

    if (!dc)
    {
        return false;
    }

    /* clang-format off */
    const int pixel_attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_RED_BITS_ARB,       8,
        WGL_GREEN_BITS_ARB,     8,
        WGL_BLUE_BITS_ARB,      8,
        WGL_ALPHA_BITS_ARB,     8,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        WGL_SAMPLE_BUFFERS_ARB, 1,
        WGL_SAMPLES_ARB,        1,  // TODO: MSAA
        0, 0
    };
    /* clang-format on */

    int pixel_format;
    UINT num_formats;

    if (!(wglChoosePixelFormatARB(dc, pixel_attributes, nullptr, 1, &pixel_format, &num_formats) && num_formats))
    {
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;

    if (!(DescribePixelFormat(dc, pixel_format, sizeof(pfd), &pfd) && SetPixelFormat(dc, pixel_format, &pfd)))
    {
        return false;
    }

    /* clang-format off */
    int context_attributes[] =
    { 
        WGL_CONTEXT_MAJOR_VERSION_ARB,  4, 
        WGL_CONTEXT_MINOR_VERSION_ARB,  6,
        WGL_CONTEXT_PROFILE_MASK_ARB,   WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
#if GS_ENABLE_OPENGL_DEBUGGING
        WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };
    /* clang-format on */

    HGLRC glrc = wglCreateContextAttribsARB(dc, 0, context_attributes);

    if (!glrc)
    {
        return false;
    }

    if (!gWglHelper.MakeCurrent(dc, glrc))
    {
        gWglHelper.DeleteContext(glrc);
        return false;
    }

#if GS_ENABLE_OPENGL_DEBUGGING
    glDebugMessageCallback(DebugMessageCallback, nullptr);
#endif

    gWglHelper.MakeCurrent(nullptr, nullptr);
    hwnd_ = hwnd;
    dc_ = dc;
    rc_ = glrc;
    dc.Reset();

    return true;
}

void RendererGL::BeginFrame()
{
    gWglHelper.MakeCurrent(dc_, rc_);
}

void RendererGL::EndFrame()
{
    SwapBuffers(dc_);
    gWglHelper.MakeCurrent(nullptr, nullptr);
}

} // namespace GameSmith