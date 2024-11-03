
/*void glfw_framebuffer_size_callback(GLFWwindow *window, i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

void glfw_key_callback(GLFWwindow *window, int key,
                       int scancode, int action, int mods)
{
    input_t *input = (input_t *)glfwGetWindowUserPointer(window);
    
    u32 key_idx = key_conv_table[key];
    
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        input->key_state[key_idx] |= INPUT_STATE_DOWN;
    else if (action == GLFW_RELEASE)
        input->key_state[key_idx] &= ~INPUT_STATE_DOWN;
    
}*/

internal void
init_glfw_app(glfw_app_t *app, 
              const char **app_func_names, u32 app_func_count,
              const char **render_func_names, u32 render_func_count,
              const char *build_dir, const char *data_dir,
              const char *name)
{
    glfw_init_key_conv_table();
    
    // NOTE(ajeej): Load app code
    {
        init_loaded_code(&app->code, (void **)&app->funcs,
                         app_func_names, app_func_count,
                         build_dir, name, "dll", 1);
        load_code(&app->code);
        ASSERT_LOG(app->code.is_valid, "Error: Failed to load %s.dll", name);
    }
    
    // NOTE(ajeej): Load render code
    {
        // TODO(ajeej): do not hard code dll name, parameter
        //              or have all dll name for renderer to be the same
        init_loaded_code(&app->render_code, (void **)&app->render_funcs,
                         render_func_names, render_func_count,
                         build_dir, "glfw_wgpu", "dll", 1);
        load_code(&app->render_code);
        ASSERT_LOG(app->render_code.is_valid, "Error: Failed to load glfw_opengl.dll for %s", name);
    }
    
    // NOTE(ajeej): Create GLFW window
    {
        app->window = glfwCreateWindow(800, 600, name, NULL, NULL);
        ASSERT_LOG(app->window, "Error: Failed to create glfw window for %s", name);
    }
    
    /*// NOTE(ajeej): Initialize GLEW functions
    {
        glfwMakeContextCurrent(app->window);
        glewInit();
    }*/
    
    /*// TODO(ajeej): Move somewhere so doesn't have to be rewritten
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);*/
    
    // NOTE(ajeej): Init Plat App
    init_plat_app(&app->plat_app, 800, 600, (char *)data_dir);
    
    // TODO(ajeej): set callbacks
    {
        input_t *input = &app->plat_app.input;
        glfwSetWindowUserPointer(app->window, input);
        
        //glfwSetFramebufferSizeCallback(app->window, glfw_framebuffer_size_callback);
        //glfwSetKeyCallback(app->window, glfw_key_callback);
    }
}

internal void
free_glfw_app(glfw_app_t *app)
{
    free_loaded_code(&app->code);
    glfwDestroyWindow(app->window);
    free_plat_app(&app->plat_app);
}