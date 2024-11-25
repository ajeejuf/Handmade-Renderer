
#ifndef GLFW_RENDERER_H
#define GLFW_RENDERER_H

#define INIT_RENDERER(name) void name(GLFWwindow *window, renderer_t *rb, asset_manager_t *am)
typedef INIT_RENDERER(init_renderer_t);

#define START_FRAME(name) void name(renderer_t *rb)
typedef START_FRAME(start_frame_t);

#define END_FRAME(name) void name(renderer_t *rb)
typedef END_FRAME(end_frame_t);

typedef struct render_function_table_t {
    init_renderer_t *init_renderer;
    start_frame_t *start_frame;
    end_frame_t *end_frame;
} render_function_table_t;

global const char *render_func_names[] = {
    "init_renderer",
    "start_frame",
    "end_frame",
};

#endif //GLFW_RENDERER_H
