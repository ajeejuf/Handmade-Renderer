
typedef struct ems_app_t {
    // NOTE(ajeej): App loading
    loaded_code_t code;
    app_func_table_t funcs;
    
    loaded_code_t render_code;
    render_function_table_t render_funcs;
    
    // NOTE(ajeej): WebGL
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    const char *canvas;
    
    // NOTE(ajeej): Platform Indepent App
    app_t plat_app;
} ems_app_t;

internal void
ems_init_key_conv_table()
{
#define VK_KEY(x, y) key_conv_table[DOM_VK_##x] = KEY_##y;
#include "ems_keys.inc"
#undef VK_KEY
}