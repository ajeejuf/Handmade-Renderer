
#include "windows.h"

#define load_library(hnd) LoadLibraryA(hnd)
#define unload_library(hnd) FreeLibrary(hnd)
#define load_func(hnd, fn) GetProcAddress(hnd, fn)

#include "utils.h"
#include "load.h"

#include "load.c"


int WinMain(HINSTANCE instance, HINSTANCE prev_instance,
            PSTR cmd_line, INT cmd_show)
{
    WNDCLASSA window_class = {0};
    {
        // NOTE(ajeej): init window class
        window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
        window_class.lpfnWndProce = win32_window_process;
        window_class.hInstance = instance;
        window_class.hCursor = LoadCursor(0, IDC_ARROW);
        window_class.lpszClassName = "Win32WindowClass";
    }
    
    // NOTE(ajeej): register window class
    if (!RegisterClassA(&window_class)) {
        // TODO(ajeej): assert
        return 1;
    }
    
    // NOTE(ajeej): create window
    HWND window = CreateWindowExA(0, window_class.lpszClassName,
                                  "Win32 Window", WS_OVERLAPPED_WINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  800, 600, 0, 0, instance, 0);
    
    if (!window) {
        // TODO(ajeej): assert
        return 1;
    }
    
    
}