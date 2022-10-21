#ifndef GBAPP_WIN32_INCLUDED
#define GBAPP_WIN32_INCLUDED

#pragma region Function Errors

/* function errors */
typedef enum {
    gbapp_win32_OK,
    gbapp_win32_INVALID_ARGS,
    gbapp_win32_init_app_REGISTER_CLASS_FAILED,
    gbapp_win32_create_window_ADJUST_WINDOW_RECT_FAILED,
    gbapp_win32_create_window_CREATE_WINDOW_FAILED,
    gbapp_win32_create_window_GET_DC_FAILED,
} gbapp_win32_eErrno;

#pragma endregion
#pragma region Structs

typedef struct { int unused; } *gbapp_win32_cHandle;

typedef struct {
    gbapp_win32_cHandle instance;
} gbapp_win32_cApp;

typedef struct {
    char should_close;
    const gbapp_win32_cApp* app;
    gbapp_win32_cHandle wnd;
    gbapp_win32_cHandle dc;
} gbapp_win32_cWindow;

#pragma endregion
#pragma region Functions

gbapp_win32_eErrno gbapp_win32_init_app(gbapp_win32_cApp* app_out);

gbapp_win32_eErrno gbapp_win32_create_window(
    const gbapp_win32_cApp* app, 
    unsigned width, unsigned height,
    gbapp_win32_cWindow* window_out
);

gbapp_win32_eErrno gbapp_win32_show_window(const gbapp_win32_cWindow* window);

gbapp_win32_eErrno gbapp_win32_swap_buffers(const gbapp_win32_cWindow* window);

gbapp_win32_eErrno gbapp_win32_poll_events(const gbapp_win32_cWindow* window);

gbapp_win32_eErrno gbapp_win32_destroy_window(const gbapp_win32_cWindow* window);

gbapp_win32_eErrno gbapp_win32_terminate_app(const gbapp_win32_cApp* app);

#endif