
#include <webgpu\webgpu.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include <dlfcn.h>

#define load_library(hnd) dlopen(hnd, RTLD_LAZY)
#define unload_library(hnd) dlclose(hnd)
#define load_func(hnd,fn) dlsym(hnd, fn)

#include "utils.h"
#include "load.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "callback.h"
#include "app_funcs.h"
#include "ems_renderer.h"
#include "ems_app.h"

#include "load.c"
#include "shape.c"
#include "renderer.c"
#include "assets.c"

//#include "shader.c"
//#include "opengl.c"
#include "app.c"

global STACK(char *) *mod_names = NULL;
global STACK(struct hashmap_s) *func_hashes = NULL;
global struct hashmap_s *mod_hash = NULL;

#include "ems_app.c"


typedef struct request_adapter_callback_data_t {
    void *adapter;
    b8 request_ended;
} request_adapter_callback_data_t;

typedef struct request_device_callback_data_t {
    void *device;
    b8 request_ended;
} request_device_callback_data_t;

void request_adapter_callback(u32 status, void *adapter,
                              char const *message, void *user_data)
{
    request_adapter_callback_data_t *data = (request_adapter_callback_data_t *)user_data;
    
    if (status == 0) {
        data->adapter = adapter;
    } else {
        LOG("Could not get WebGPU adapter: %s", message);
    }
    
    data->request_ended = 1;
}

void request_device_callback(u32 status, void *device,
                             char const *message, void *user_data)
{
    request_device_callback_data_t *data = (request_device_callback_data_t *)user_data;
    
    if (status == 0) {
        data->device = device;
    } else {
        LOG("Could not get WebGPU device: %s", message);
    }
    
    data->request_ended = 1;
}


EMSCRIPTEN_KEEPALIVE
void call_registered_function(const char *module, const char *func,
                              void *return_value, void *args)
{
    LOG("module: %s, func: %s", module, func);
    
    struct hashmap_s *func_hash = (struct hashmap_s *)hashmap_get(mod_hash, module, strlen(module));
    
    ASSERT_LOG(func_hash, "Failed to get module");
    
    callback_info_t *callback = (callback_info_t *)hashmap_get(func_hash, func, strlen(func));
    
    ASSERT_LOG(callback, "Failed to get function");
    
    call_func_callback(callback, return_value, args);
}


const u32 TARGET_FPS = 60;
const f64 FRAME_DURATION = 1000.0 / TARGET_FPS;
global f64 last_time = 0;

typedef struct main_loop_arg_t {
    ems_app_t *apps;
    u32 count;
} main_loop_arg_t;

internal void 
main_loop(void *arg) {
    main_loop_arg_t *data = (main_loop_arg_t *)arg;
    ems_app_t *apps = data->apps;
    u32 count = data->count;
    
    f64 current_time = emscripten_get_now();
    f64 dt = current_time - last_time;
    
    if (dt >= FRAME_DURATION) {
        ems_app_t *app;
        app_t *plat_app;
        for (int i = 0; i < count; i++)
        {
            app = apps+i;
            plat_app = &app->plat_app;
            
            // NOTE(ajeej): Make canvas context current
            //ASSERT_LOG(app->ctx > 0, "Error: invalid WebGL context %lu", app->ctx);
            //emscripten_webgl_make_context_current(app->ctx);
            
            // NOTE(ajeej): Start Frame
            app->render_funcs.start_frame(&plat_app->rb);
            
            // NOTE(ajeej): Update and Render
            ASSERT_LOG(app->funcs.update_and_render, "Error: update_and_render is invalid");
            app->funcs.update_and_render(plat_app);
            
            // NOTE(ajeej): End Frame
            app->render_funcs.end_frame(&plat_app->rb);
        }
        
    }
}

int
main(int argc, char **argv)
{
    ASSERT_LOG(argc > 1, "Error: No apps");
    ASSERT_LOG((argc-1)%2 == 0, "Error: Need wasm, canvas id pair");
    
    u32 app_count = (argc-1)/2;
    char *build_dir = get_build_dir(argv[0]);
    char data_dir[] = "./data/";
    
    main_loop_arg_t data = {0};
    
    mod_hash = malloc(sizeof(*mod_hash));
    hashmap_create(2, mod_hash);
    
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
                     render_func_names, ARRAY_COUNT(render_func_names),
                     build_dir, data_dir,
                     name, canvas);
        
        LOG("Loading Assets");
        
        app->funcs.load_assets(&plat_app->am);
        
        LOG("Udating Assets");
        
        // NOTE(ajeej): Update assets
        update_assets(&plat_app->am);
        
        LOG("Init app");
        
        // NOTE(ajeej): Intialize app
        app->funcs.init_app(plat_app);
        
        LOG("Init Renderer");
        
        // NOTE(ajeej): Initialize renderer
        app->render_funcs.init_renderer(app->canvas, &plat_app->rb, &plat_app->am,
                                        request_adapter_callback,
                                        request_device_callback);
        
        free((void *)canvas);
        free((void *)name);
    }
    
    data.apps = apps;
    data.count = app_count;
    
    emscripten_set_main_loop_arg(main_loop, &data, 0, 1);
    
    for (int i = 0; i < app_count; i++)
        free_loaded_code(&apps[i].code);
    free(apps);
    free(build_dir);
    return 0;
}


