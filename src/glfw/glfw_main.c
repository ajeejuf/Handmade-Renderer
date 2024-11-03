

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#define load_library(hnd) LoadLibraryA(hnd)
#define unload_library(hnd) FreeLibrary(hnd)
#define load_func(hnd,fn) GetProcAddress(hnd, fn)
#define get_working_directory(p, s) GetCurrentDirectory(s, p);

#else
#include <unistd.h>
#include <dlfcn.h>

#define load_library(hnd) dlopen(hnd, RTLD_LAZY)
#define unload_library(hnd) dlclose(hnd)
#define load_func(hnd,fn) dlsym(hnd, fn)
#define get_working_directory(p, s) getcwd(p, s)

#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "utils.h"
#include "load.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "app_funcs.h"
#include "glfw_renderer.h"
#include "glfw_app.h"

#include "load.c"
#include "renderer.c"
#include "assets.c"

//#include "shader.c"
//#include "opengl.c"
#include "app.c"
#include "glfw_app.c"

int main(int argc, char **argv)
{
    ASSERT_LOG(argc > 1, "Error: No apps");
    
    char build_dir[MAX_PATH];
    // TODO(ajeej): parameterize
    char data_dir[] = "..\\data\\";
    get_working_directory(build_dir, MAX_PATH);
    
    u32 app_count = argc-1;
    
    if (!glfwInit())
    {
        // TODO(ajeej): LOG
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    glfw_app_t *apps = malloc(sizeof(glfw_app_t)*app_count);
    memset(apps, 0, sizeof(glfw_app_t)*app_count);
    
    glfw_app_t *app;
    app_t *plat_app;
    const char *name;
    for (int i = 0; i < app_count; i++)
    {
        name = cstr_dup(argv[i+1]);
        app = apps+i;
        plat_app = &app->plat_app;
        
        // NOTE(ajeej): Initialize GLFW App
        init_glfw_app(app, 
                      app_func_names, ARRAY_COUNT(app_func_names),
                      render_func_names, ARRAY_COUNT(render_func_names),
                      build_dir, data_dir, name);
        
        // NOTE(ajeej): Initialize app
        app->funcs.init_app(plat_app);
        
        // NOTE(ajeej): Update assets defined during initalization
        update_assets(&plat_app->am);
        
        // NOTE(ajeej): Initialize renderer
        app->render_funcs.init_renderer(app->window, &plat_app->rb, &plat_app->am);
        
        free((void *)name);
    }
    
    glfw_app_t *cur_app = app;
    plat_app = &cur_app->plat_app;
    while(!glfwWindowShouldClose(cur_app->window))
    {
        glfwMakeContextCurrent(cur_app->window);
        
        app->render_funcs.start_frame(&plat_app->rb);
        //start_frame(&plat_app->rb);
        
        cur_app->funcs.update_and_render(plat_app);
        
        app->render_funcs.end_frame(&plat_app->rb);
        //end_frame(&plat_app->rb);
        
        glfwSwapBuffers(cur_app->window);
        glfwPollEvents();
    }
    
    
    return 0;
}