
internal void
init_plat_app(app_t *app, i32 width, i32 height)
{
    memset(app, 0, sizeof(*app));
    
    init_key_conv_table();
    
    // NOTE(ajeej): Initialize renderer
    init_renderer(&app->rb, width, height);
    
    init_asset_manager(&app->am);
}

internal void
free_plat_app(app_t *app)
{
    free_renderer(&app->rb);
}
