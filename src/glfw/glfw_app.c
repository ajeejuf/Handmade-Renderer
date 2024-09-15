
internal void
init_glfw_app(glfw_app_t *app, const char **app_func_names, u32 func_count,
              const char *build_dir, const char *name)
{
    glfw_init_key_conv_table();
    
    // NOTE(ajeej): Load app code
    {
        init_loaded_code(&app->code, (void **)&app->funcs,
                         app_func_names, func_count,
                         build_dir, name, "dll", 1);
        load_code(&app->code);
        ASSERT_LOG(app->code.is_valid, "Error: Failed to load %s.dll", name);
    }
    
    // NOTE(ajeej): Create GLFW window
    {
        app->window = glfwCreateWindow(800, 600, name, NULL, NULL);
        ASSERT_LOG(app->window, "Error: Failed to create glfw window for %s", name);
    }
    
    // NOTE(ajeej): Initialize GLEW functions
    {
        glfwMakeContextCurrent(app->window);
        glewInit();
    }
    
    // TODO(ajeej): Move somewhere so doesn't have to be rewritten
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // NOTE(ajeej): Init Plat App
    init_plat_app(&app->plat_app, 800, 600);
    
    // TODO(ajeej): set callbacks
    {
        
    }
}

internal void
free_glfw_app(glfw_app_t *app)
{
    free_loaded_code(&app->code);
    glfwDestroyWindow(app->window);
    free_plat_app(&app->plat_app);
}