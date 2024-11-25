
#ifndef APP_FUNC_H
#define APP_FUNC_H

#define LOAD_ASSETS(name) void name(asset_manager_t *am)
typedef LOAD_ASSETS(load_assets_t);

#define INIT_APP(name) void name(app_t *app)
typedef INIT_APP(init_app_t);

#define UPDATE_AND_RENDER(name) void name(app_t *app)
typedef UPDATE_AND_RENDER(update_and_render_t);

typedef struct app_func_table_t {
    load_assets_t *load_assets;
    init_app_t *init_app;
    update_and_render_t *update_and_render;
} app_func_table_t;

const char *app_func_names[] = {
    "load_assets",
    "init_app",
    "update_and_render"
};

#endif //APP_FUNC_H
