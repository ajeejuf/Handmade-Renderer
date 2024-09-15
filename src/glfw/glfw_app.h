
#ifndef GLFW_APP_H
#define GLFW_APP_H

typedef struct glfw_app_t {
    loaded_code_t code;
    app_func_table_t funcs;
    
    GLFWwindow *window;
    
    app_t plat_app;
} glfw_app_t;

internal void
glfw_init_key_conv_table()
{
#define VK_KEY(x, y) key_conv_table[GLFW_KEY_##y] = KEY_##x;
#include "glfw_keys.inc"
#undef VK_KEY
}

#endif //GLFW_APP_H
