
internal void
init_plat_app(app_t *app, i32 width, i32 height, char *data_dir)
{
    memset(app, 0, sizeof(*app));
    
    // NOTE(ajeej): Initialize renderer
    init_app_renderer(&app->rb, width, height);
    
    init_asset_manager(&app->am, data_dir);
}

internal void
free_plat_app(app_t *app)
{
    free_app_renderer(&app->rb);
}
