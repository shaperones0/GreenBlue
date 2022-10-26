#include "gbapp_win32.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define GBAPP_WIN32_ASSERT(x, ret) { if (!(x)) return (ret); } 
#define GBAPP_WIN32_ASSERT_ARG(x) GBAPP_WIN32_ASSERT(x, gbapp_win32_INVALID_ARGS)

static char _str_find(const char* needle, const char* haystack) {
    const char* start = haystack;
    while (1) {
        const char* where = strstr(start, needle);
        if (!where) {
            return 0;
        }
        const char* terminator = where + strlen(needle);
        if ((where == start) || (*(where - 1) == ' ')) {
            if (*terminator == ' ' || *terminator == '\0') {
                break;
            }
        }
        start = terminator;
    }
    return 1;
}

static char _wgl_is_ext_supported(const char* ext, const char* supportedEXT, const char* supportedARB) {
    if (_str_find(ext, supportedEXT)) {
        return 1;
    }
    if (_str_find(ext, supportedARB)) {
        return 1;
    }
    return 0;
}

static char _wgl_get_pf_attrib(gbapp_win32_PFN_wgl_get_pixel_format_attrib_iv_ARB f, gbapp_win32_cHandle dc, int pf, int attrib, int* value_out) {
    return !f(dc, pf, 0, 1, &attrib, value_out);
}

static LRESULT CALLBACK _wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    LONG_PTR vv = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    gbapp_win32_cWindow* window = (gbapp_win32_cWindow*)vv;
    
    if (window == 0) {
        //haven't yet finished WM_CREATE event and not set userdata ptr
        if (Msg == WM_CREATE) {
            CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
            int* userdata = (int*)cs->lpCreateParams;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)userdata);
        }
    }
    else {
        switch (Msg) {
        case WM_PAINT:
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            window->should_close = 1;
            return 0;
        }
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

gbapp_win32_eErrno gbapp_win32_init_app(gbapp_win32_cApp* app_out) {
    GBAPP_WIN32_ASSERT_ARG(app_out);

    *app_out = (gbapp_win32_cApp){ 0 };
    app_out->instance = (gbapp_win32_cHandle)GetModuleHandleW(NULL);
    
    const WNDCLASSW wndclass = { 
        .lpszClassName = L"GBAPP_WIN32",
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = (WNDPROC)_wndProc,
        .cbClsExtra = sizeof(void*),
        .cbWndExtra = 0,
        .hInstance = (HINSTANCE)app_out->instance,
        .hIcon = LoadIconW(NULL, IDI_WINLOGO),
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = NULL,
        .lpszMenuName = NULL,
    };

    ATOM result = RegisterClassW(&wndclass);
    GBAPP_WIN32_ASSERT(result, gbapp_win32_init_app_REGISTER_CLASS_FAILED);

    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_init_wgl(const gbapp_win32_cApp* app, gbapp_win32_cWGL* wgl_out) {
    GBAPP_WIN32_ASSERT_ARG(app);
    GBAPP_WIN32_ASSERT_ARG(wgl_out);

    // load wgl functions for loading extensions 
    *wgl_out = (gbapp_win32_cWGL){ 0 };
    wgl_out->opengl32 = (gbapp_win32_cHandle)LoadLibraryA("opengl32.dll");
    GBAPP_WIN32_ASSERT(wgl_out->opengl32, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    wgl_out->wgl_create_context = (gbapp_win32_PFN_wgl_create_context)(void*)
        GetProcAddress((HMODULE)wgl_out->opengl32, "wglCreateContext");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_create_context, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    wgl_out->wgl_delete_context = (gbapp_win32_PFN_wgl_delete_context)(void*)
        GetProcAddress((HMODULE)wgl_out->opengl32, "wglDeleteContext");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_delete_context, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    wgl_out->wgl_get_proc_address = (gbapp_win32_PFN_wgl_get_proc_address)(void*)
        GetProcAddress((HMODULE)wgl_out->opengl32, "wglGetProcAddress");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_get_proc_address, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    wgl_out->wgl_get_current_dc = (gbapp_win32_PFN_wgl_get_current_DC)(void*)
        GetProcAddress((HMODULE)wgl_out->opengl32, "wglGetCurrentDC");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_get_current_dc, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    wgl_out->wgl_make_current = (gbapp_win32_PFN_wgl_make_current)(void*)
        GetProcAddress((HMODULE)wgl_out->opengl32, "wglMakeCurrent");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_make_current, gbapp_win32_init_wgl_LOAD_OPENGL_FAILED);

    // create dummy window to later make dummy device context
    HWND dummyWindow = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
        L"GBAPP_WIN32",
        L"GB Engine dummy window",
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 1, 1,
        NULL, NULL, 
        (HINSTANCE)app->instance,
        NULL
    );
    GBAPP_WIN32_ASSERT(dummyWindow, gbapp_win32_init_wgl_CREATE_DUMMY_WINDOW_FAILED);
    
    ShowWindow(dummyWindow, SW_HIDE);
    // flush message queue 
    MSG msg;
    while (PeekMessageW(&msg, dummyWindow, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // create dummy device context to later load with it our pixel format
    // and create dummy wgl render context
    HDC dummyDC = GetDC(dummyWindow);
    GBAPP_WIN32_ASSERT(dummyDC, gbapp_win32_init_wgl_CREATE_DUMMY_DC_FAILED);

    // set pixel format to our dummy dc
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    int pixelFormat = ChoosePixelFormat(dummyDC, &pfd);
    GBAPP_WIN32_ASSERT(pixelFormat, gbapp_win32_init_wgl_CHOOSE_PIXEL_FORMAT_FAILED);

    WINBOOL bResult = SetPixelFormat(dummyDC, pixelFormat, &pfd);
    GBAPP_WIN32_ASSERT(bResult, gbapp_win32_init_wgl_SET_PIXEL_FORMAT_FAILED);

    // create dummy render context to later load with it extensions 
    HGLRC dummyRC = (HGLRC)wgl_out->wgl_create_context((gbapp_win32_cHandle)dummyDC);
    
    GBAPP_WIN32_ASSERT(dummyRC, gbapp_win32_init_wgl_CREATE_DUMMY_RC_FAILED);

    bResult = wgl_out->wgl_make_current((gbapp_win32_cHandle)dummyDC, (gbapp_win32_cHandle)dummyRC);
    GBAPP_WIN32_ASSERT(bResult, gbapp_win32_init_wgl_MAKE_DUMMY_RC_CURRENT_FAILED);

    //load extension loading functions (can do that only after wgl_make_current)
    wgl_out->wgl_get_extension_string_EXT = (gbapp_win32_PFN_wgl_get_extension_string_EXT)(void*) 
        wgl_out->wgl_get_proc_address("wglGetExtensionsStringEXT");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_get_extension_string_EXT, gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED);

    wgl_out->wgl_get_extension_string_ARB = (gbapp_win32_PFN_wgl_get_extension_string_ARB)(void*) 
        wgl_out->wgl_get_proc_address("wglGetExtensionsStringARB");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_get_extension_string_ARB, gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED);

    wgl_out->wgl_create_context_attribs_ARB = (gbapp_win32_PFN_wgl_create_context_attribs_ARB)(void*) 
        wgl_out->wgl_get_proc_address("wglCreateContextAttribsARB");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_create_context_attribs_ARB, gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED);

    wgl_out->wgl_swap_interval_EXT = (gbapp_win32_PFN_wgl_swap_interval_EXT)(void*)
        wgl_out->wgl_get_proc_address("wglSwapIntervalEXT");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_swap_interval_EXT, gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED);

    wgl_out->wgl_get_pixel_format_attrib_iv_ARB = (gbapp_win32_PFN_wgl_get_pixel_format_attrib_iv_ARB)(void*)
        wgl_out->wgl_get_proc_address("wglGetPixelFormatAttribivARB");
    GBAPP_WIN32_ASSERT(wgl_out->wgl_get_pixel_format_attrib_iv_ARB, gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED);

    // get extension strings
    gbapp_win32_cHandle curDC = wgl_out->wgl_get_current_dc();
    const char* extsARB = wgl_out->wgl_get_extension_string_ARB(curDC);
    const char* extsEXT = wgl_out->wgl_get_extension_string_EXT();

    wgl_out->arb_multisample = _wgl_is_ext_supported("WGL_ARB_multisample", extsEXT, extsARB);
    wgl_out->arb_create_context = _wgl_is_ext_supported("WGL_ARB_create_context", extsEXT, extsARB);
    wgl_out->arb_create_context_profile = _wgl_is_ext_supported("WGL_ARB_create_context_profile", extsEXT, extsARB);
    wgl_out->ext_swap_control = _wgl_is_ext_supported("WGL_EXT_swap_control", extsEXT, extsARB);
    wgl_out->arb_pixel_format = _wgl_is_ext_supported("WGL_ARB_pixel_format", extsEXT, extsARB);
    
    // done loading extensions. reset current context.
    wgl_out->wgl_make_current((gbapp_win32_cHandle)dummyDC, 0);
    wgl_out->wgl_delete_context((gbapp_win32_cHandle)dummyRC);

    DestroyWindow(dummyWindow);

    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_create_window(const gbapp_win32_cApp* app, unsigned width, unsigned height, gbapp_win32_cWindow* window_out) {
    GBAPP_WIN32_ASSERT_ARG(app);
    GBAPP_WIN32_ASSERT_ARG(window_out);
    
    *window_out = (gbapp_win32_cWindow){ 0 };
    window_out->should_close = 0;

    const DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    const RECT rect = { 0, 0, width, height };

    WINBOOL awrRes = AdjustWindowRect((LPRECT)&rect, style, FALSE);
    GBAPP_WIN32_ASSERT(awrRes, gbapp_win32_create_window_ADJUST_WINDOW_RECT_FAILED);

    window_out->app = app;
    window_out->wnd = (gbapp_win32_cHandle)CreateWindowExW(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
        L"GBAPP_WIN32",
        L"GB Window",
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
        CW_USEDEFAULT,
        SW_HIDE,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, (HINSTANCE)app->instance, (void*)window_out
    );
    GBAPP_WIN32_ASSERT(window_out->wnd, gbapp_win32_create_window_CREATE_WINDOW_FAILED);

    window_out->dc = (gbapp_win32_cHandle)GetDC((HWND)window_out->wnd);
    GBAPP_WIN32_ASSERT(window_out->dc, gbapp_win32_create_window_GET_DC_FAILED);

    //process window create events so they don't bother us afterwards
    MSG msg;
    while (PeekMessageW(&msg, (HWND)window_out->wnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_show_window(const gbapp_win32_cWindow* window) {
    GBAPP_WIN32_ASSERT_ARG(window);

    ShowWindow((HWND)window->wnd, SW_SHOW);
    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_swap_buffers(const gbapp_win32_cWindow* window) {
    GBAPP_WIN32_ASSERT_ARG(window);
    //FIXME? possibly check DwmIsCompositionEnabled (GLFW)
    SwapBuffers((HDC)window->dc);
    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_poll_events(const gbapp_win32_cWindow* window) {
    GBAPP_WIN32_ASSERT_ARG(window);

    MSG msg;
    while (PeekMessageW(&msg, (HWND)window->wnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);     // translate keyboard codes to virtual keycodes
        DispatchMessageW(&msg);     // pass it to wndproc
    }

    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_destroy_window(const gbapp_win32_cWindow* window) {
    GBAPP_WIN32_ASSERT_ARG(window);

    DestroyWindow((HWND)window->wnd);
    return gbapp_win32_OK;
}

gbapp_win32_eErrno gbapp_win32_terminate_app(const gbapp_win32_cApp* app) {
    GBAPP_WIN32_ASSERT_ARG(app);

    UnregisterClassW(L"GBAPP_WIN32", (HINSTANCE)app->instance);
    return gbapp_win32_OK;
}