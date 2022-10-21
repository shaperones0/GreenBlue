#include "gbapp_win32.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define GBAPP_WIN32_ASSERT(x, ret) { if (!(x)) return (ret); } 
#define GBAPP_WIN32_ASSERT_ARG(x) GBAPP_WIN32_ASSERT(x, gbapp_win32_INVALID_ARGS)

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

gbapp_win32_eErrno gbapp_win32_create_window(const gbapp_win32_cApp* app, unsigned width, unsigned height, gbapp_win32_cWindow* window_out) {
    GBAPP_WIN32_ASSERT_ARG(app);
    GBAPP_WIN32_ASSERT_ARG(window_out);
    
    *window_out = (gbapp_win32_cWindow){ 0 };
    window_out->should_close = 0;

    const DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    const RECT rect = { 0, 0, width, height };

    WINBOOL awr_res = AdjustWindowRect((LPRECT)&rect, style, FALSE);
    GBAPP_WIN32_ASSERT(awr_res, gbapp_win32_create_window_ADJUST_WINDOW_RECT_FAILED);

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