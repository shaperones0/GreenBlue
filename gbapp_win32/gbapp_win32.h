#ifndef GBAPP_WIN32_INCLUDED
#define GBAPP_WIN32_INCLUDED

#pragma region Function Errors

/* Function Errors */
typedef enum {
    /* Success */
    gbapp_win32_OK,

    /* Invalid arguments passed into function */
    gbapp_win32_INVALID_ARGS,

    /* WinAPI function `RegisterClass` returned an error */
    gbapp_win32_init_app_REGISTER_CLASS_FAILED,

    /* Failed obtaining opengl32.dll or one of it's required functions */
    gbapp_win32_init_wgl_LOAD_OPENGL_FAILED,

    /* Failed loading required OpenGL extensions or one of extension loading functions */
    gbapp_win32_init_wgl_LOAD_OPENGL_EXT_FAILED,

    /* Failed creating dummy window (WinAPI function `CreateWindow` returned an error) */
    gbapp_win32_init_wgl_CREATE_DUMMY_WINDOW_FAILED,

    /* Failed creating dummy device context (WinAPI function `GetDC` returned an error) */
    gbapp_win32_init_wgl_CREATE_DUMMY_DC_FAILED,

    /* WinAPI function `ChoosePixelFormat` returned an error */
    gbapp_win32_init_wgl_CHOOSE_PIXEL_FORMAT_FAILED,

    /* WinAPI function `SetPixelFormat` returned an error */
    gbapp_win32_init_wgl_SET_PIXEL_FORMAT_FAILED,

    /* Failed creating dummy render context*/
    gbapp_win32_init_wgl_CREATE_DUMMY_RC_FAILED,

    /* Failed making dummy render context current */
    gbapp_win32_init_wgl_MAKE_DUMMY_RC_CURRENT_FAILED,

    /* WinAPI function `AdjustWindowRect` returned an error */
    gbapp_win32_create_window_ADJUST_WINDOW_RECT_FAILED,

    /* WinAPI function `CreateWindow` returned an error */
    gbapp_win32_create_window_CREATE_WINDOW_FAILED,

    /* WinAPI function `GetDC` returned an error */
    gbapp_win32_create_window_GET_DC_FAILED,
} gbapp_win32_eErrno;

#pragma endregion
#pragma region Structs

/* WinAPI handle type */
typedef struct { int unused; } *gbapp_win32_cHandle;

/* Various function pointers */
typedef long long (__stdcall *gbapp_win32_PFN_proc)();
typedef gbapp_win32_cHandle (__stdcall *gbapp_win32_PFN_wgl_create_context)(gbapp_win32_cHandle);
typedef int (__stdcall *gbapp_win32_PFN_wgl_delete_context)(gbapp_win32_cHandle);
typedef gbapp_win32_PFN_proc (__stdcall *gbapp_win32_PFN_wgl_get_proc_address)(char*);
typedef gbapp_win32_cHandle (__stdcall *gbapp_win32_PFN_wgl_get_current_DC)();
typedef int (__stdcall *gbapp_win32_PFN_wgl_make_current)(gbapp_win32_cHandle, gbapp_win32_cHandle);

typedef int (__stdcall *gbapp_win32_PFN_wgl_swap_interval_EXT)(int);
typedef int (__stdcall *gbapp_win32_PFN_wgl_get_pixel_format_attrib_iv_ARB)(gbapp_win32_cHandle, int, int, unsigned, const int*, int*);
typedef const char* (__stdcall *gbapp_win32_PFN_wgl_get_extension_string_EXT)(void);
typedef const char* (__stdcall *gbapp_win32_PFN_wgl_get_extension_string_ARB)(gbapp_win32_cHandle);
typedef gbapp_win32_cHandle (__stdcall *gbapp_win32_PFN_wgl_create_context_attribs_ARB)(gbapp_win32_cHandle, gbapp_win32_cHandle, const int*);

/**
 * @brief Application class. 
 * 
 * Application class. Contains handle to current module. 
 * Create this class using `gbapp_win32_init_app(...)`. 
 * 
 */
typedef struct {
    gbapp_win32_cHandle instance;   // WinAPI module handle to current executable module. 
} gbapp_win32_cApp;

/**
 * @brief Window class. 
 * 
 * Class representing window on screen. 
 * Create this class using `gbapp_win32_create_window(...)`.
 * 
 */
typedef struct {
    char should_close;              // Flag indicating whether window close was requested (e.g. `Alt+F4`). 
    const gbapp_win32_cApp* app;    // Pointer to own application class. 
    gbapp_win32_cHandle wnd;        // WinAPI window handle.
    gbapp_win32_cHandle dc;         // WinAPI handle to window's device context. 
} gbapp_win32_cWindow;

/**
 * @brief WGL class.
 * 
 * Class loading functions related to OpenGL contexts and extensions. 
 * 
 */
typedef struct {
    gbapp_win32_cHandle opengl32;
    gbapp_win32_PFN_wgl_create_context wgl_create_context;
    gbapp_win32_PFN_wgl_delete_context wgl_delete_context;
    gbapp_win32_PFN_wgl_get_current_DC wgl_get_current_dc;
    gbapp_win32_PFN_wgl_get_proc_address wgl_get_proc_address;
    gbapp_win32_PFN_wgl_make_current wgl_make_current;
    gbapp_win32_PFN_wgl_get_extension_string_EXT wgl_get_extension_string_EXT;
    gbapp_win32_PFN_wgl_get_extension_string_ARB wgl_get_extension_string_ARB;
    gbapp_win32_PFN_wgl_create_context_attribs_ARB wgl_create_context_attribs_ARB;
    gbapp_win32_PFN_wgl_swap_interval_EXT wgl_swap_interval_EXT;
    gbapp_win32_PFN_wgl_get_pixel_format_attrib_iv_ARB wgl_get_pixel_format_attrib_iv_ARB;

    char arb_multisample;
    char arb_create_context;
    char arb_create_context_profile;
    char ext_swap_control;
    char arb_pixel_format;
} gbapp_win32_cWGL;

#pragma endregion
#pragma region Functions

gbapp_win32_eErrno gbapp_win32_init_app(gbapp_win32_cApp* app_out);

gbapp_win32_eErrno gbapp_win32_init_wgl(const gbapp_win32_cApp* app, gbapp_win32_cWGL* wgl_out);

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