
const char *ems_events[] = { 
    "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
    "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
    "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
    "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout"
};

internal inline const char *ems_event_type_to_cstr(i32 event_type)
{
    if (event_type < 0) event_type = 0;
    else if (event_type > ARRAY_COUNT(ems_events)) event_type = ARRAY_COUNT(ems_events)-1;
    
    return ems_events[event_type];
}

// TODO(ajeej): test every result coming from emscripten and print out the result
internal const char *ems_result_to_cstr(EMSCRIPTEN_RESULT res)
{
    if (res == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
    if (res == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
    if (res == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
    if (res == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
    if (res == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
    if (res == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
    if (res == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
    if (res == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
    if (res == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
    return "Unknown EMSCRIPTEN_RESULT!";
}


internal EM_BOOL 
ems_key_callback(i32 event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
    LOG("%s, key: \"%s\", code: \"%s\", location: %u,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %u, keyCode: %u, which: %u, timestamp: %lf",
        ems_event_type_to_cstr(event_type), e->key, e->code, e->location,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
        e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which, e->timestamp);
    
    input_t *input = (input_t *)user_data;
    
    switch(event_type)
    {
        case EMSCRIPTEN_EVENT_KEYDOWN:
        {
            input->key_state[key_conv_table[e->keyCode]] |= INPUT_STATE_DOWN;
        } break;
        
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            input->key_state[key_conv_table[e->keyCode]] &= ~INPUT_STATE_DOWN;
        } break;
    }
    
    return 0;
}

internal EM_BOOL 
ems_mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *user_data) {
    /*LOG("%s, screen: (%d,%d), client: (%d,%d),%s%s%s%s button: %hu, buttons: %hu, movement: (%d,%d), canvas: (%d,%d), timestamp: %lf",
        ems_event_type_to_cstr(eventType), e->screenX, e->screenY, e->clientX, e->clientY,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
        e->button, e->buttons, e->movementX, e->movementY, e->canvasX, e->canvasY,
        e->timestamp);*/
    
    return 0;
}


internal void
init_ems_app(ems_app_t *app, 
             const char **app_func_names, u32 app_func_count,
             const char **render_func_names, u32 render_func_count,
             const char *build_dir, const char *data_dir,
             const char *name, const char *canvas)
{
    char *full_canvas;
    
    ems_init_key_conv_table();
    
    // NOTE(ajeej): Load app code
    {
        init_loaded_code(&app->code, (void **)&app->funcs,
                         app_func_names, app_func_count,
                         build_dir, name, "wasm", 0);
        load_code(&app->code);
        ASSERT_LOG(app->code.is_valid, "Error: Failed to load %s.wasm", name);
    }
    
    // NOTE(ajeej): Load render code
    {
        init_loaded_code(&app->render_code, (void **)&app->render_funcs,
                         render_func_names, render_func_count,
                         build_dir, "ems_wgpu", "wasm", 0);
        load_code(&app->render_code);
        ASSERT_LOG(app->code.is_valid, "Error: Failed to load em_wgpu");
    }
    
    // NOTE(ajeej): Create Context
    {
        const char *temp[2] = { "#", canvas };
        full_canvas = cstr_make((const char **)temp, 2);
        LOG("CANVAS: %s", full_canvas);
        /*EmscriptenWebGLContextAttributes attribs = {0};
        emscripten_webgl_init_context_attributes(&attribs);
        attribs.majorVersion = 2;
        
        app->ctx = emscripten_webgl_create_context(full_canvas, &attribs);
        
        ASSERT_LOG(app->ctx > 0, "Error: invalid WebGL context %lu", app->ctx);
        emscripten_webgl_make_context_current(app->ctx);*/
    }
    
    // NOTE(ajeej): Init Plat App
    i32 w, h;
    emscripten_get_canvas_element_size(full_canvas, &w, &h);
    LOG("Width: %d  Height: %d", w, h);
    init_plat_app(&app->plat_app, w, h, (char *)data_dir);
    
    // NOTE(ajeej): Set emscripten callbacks
    {
        input_t *input = &app->plat_app.input;
        renderer_t *rb = &app->plat_app.rb;
        
        emscripten_set_keypress_callback(full_canvas, input, 1, ems_key_callback);
        emscripten_set_keydown_callback(full_canvas, input, 1, ems_key_callback);
        emscripten_set_keyup_callback(full_canvas, input, 1, ems_key_callback);
        
        emscripten_set_click_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mousedown_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mouseup_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_dblclick_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mousemove_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mouseenter_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mouseleave_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mouseover_callback(full_canvas, input, 1, ems_mouse_callback);
        emscripten_set_mouseout_callback(full_canvas, input, 1, ems_mouse_callback);
    }
    
    app->canvas = full_canvas;
}

internal void
free_ems_app(ems_app_t *app)
{
    free_loaded_code(&app->code);
    emscripten_webgl_destroy_context(app->ctx);
    free_plat_app(&app->plat_app);
}