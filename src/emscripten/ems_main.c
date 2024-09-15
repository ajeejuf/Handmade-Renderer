

#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include <dlfcn.h>

typedef void *dll_t;

#define load_library(hnd) dlopen(hnd, RTLD_LAZY)
#define unload_library(hnd) dlclose(hnd)
#define load_func(hnd,fn) dlsym(hnd, fn)

#include "utils.h"
#include "load.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "app_funcs.h"
#include "ems_app.h"

#include "load.c"
#include "renderer.c"
#include "assets.c"

#include "shader.c"
#include "opengl.c"
#include "app.c"
#include "ems_app.c"


typedef struct main_loop_arg_t {
    ems_app_t *apps;
    u32 count;
} main_loop_arg_t;

internal void 
main_loop(void *arg) {
    main_loop_arg_t *data = (main_loop_arg_t *)arg;
    ems_app_t *apps = data->apps;
    u32 count = data->count;
    
    ems_app_t *app;
    app_t *plat_app;
    for (int i = 0; i < count; i++)
    {
        app = apps+i;
        plat_app = &app->plat_app;
        
        // NOTE(ajeej): Make canvas context current
        ASSERT_LOG(app->ctx > 0, "Error: invalid WebGL context %lu", app->ctx);
        emscripten_webgl_make_context_current(app->ctx);
        
        // NOTE(ajeej): Start Frame
        start_frame(&plat_app->rb);
        
        // NOTE(ajeej): Update and Render
        ASSERT_LOG(app->funcs.update_and_render, "Error: update_and_render is invalid");
        app->funcs.update_and_render(plat_app);
        
        // NOTE(ajeej): End Frame
        end_frame(&plat_app->rb);
    }
}

int
main(int argc, char **argv)
{
    ASSERT_LOG(argc > 1, "Error: No apps");
    ASSERT_LOG((argc-1)%2 == 0, "Error: Need wasm, canvas id pair");
    
    u32 app_count = (argc-1)/2;
    char *build_dir = get_build_dir(argv[0]);
    
    ems_app_t *apps = malloc(sizeof(ems_app_t)*app_count);
    memset(apps, 0, sizeof(ems_app_t)*app_count);
    
    ems_app_t *app;
    app_t *plat_app;
    const char *canvas, *name;
    for (int i = 0; i < app_count; i++)
    {
        canvas = cstr_dup(argv[2*i+1]);
        name = cstr_dup(argv[2*i+2]);
        app = apps+i;
        plat_app = &app->plat_app;
        
        // NOTE(ajeej): Initialize EMS App
        init_ems_app(app, app_func_names, ARRAY_COUNT(app_func_names),
                     build_dir, name, canvas);
        
        // NOTE(ajeej): Intialize app
        app->funcs.init_app(plat_app);
        
        // NOTE(ajeej): Update assets
        update_assets(&plat_app->rb, &plat_app->am);
        
        // NOTE(ajeej): Update renderer
        update_renderer(&plat_app->rb);
        
        
        free((void *)canvas);
        free((void *)name);
    }
    
    main_loop_arg_t data = {0};
    data.apps = apps;
    data.count = app_count;
    
    emscripten_set_main_loop_arg(main_loop, &data, 0, 1);
    
    for (int i = 0; i < app_count; i++)
        free_loaded_code(&apps[i].code);
    free(apps);
    free(build_dir);
    return 0;
}


