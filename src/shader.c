
internal u32
get_gl_shader_type(shader_type_t type)
{
    switch(type)
    {
        case VERTEX_SHADER: {
            return GL_VERTEX_SHADER;
        } break;
        
        case FRAGMENT_SHADER: {
            return GL_FRAGMENT_SHADER;
        } break;
        
        default: {
            ASSERT("Invalid shader type.");
        } break;
    }
    
    return 0;
}

internal u32
create_program(u32 *s, u32 count)
{
    u32 res = glCreateProgram();
    for (int i = 0; i < count; i++)
        glAttachShader(res, s[i]);
    glLinkProgram(res);
    
    GLint success;
    glGetProgramiv(res, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(res, sizeof(info_log), NULL, info_log);
        LOG("Error: Failed to link shaders.\n%s", info_log);
    }
    
    return res;
}

internal u32
compile_shader(const char *src, u32 type)
{
    u32 res = glCreateShader(type);
    glShaderSource(res, 1, &src, NULL);
    glCompileShader(res);
    
    GLint success;
    glGetShaderiv(res, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(res, sizeof(info_log), NULL, info_log);
        LOG("Error: Failed to compile shader.\n%s", info_log);
    }
    
    return res;
}

internal u32
compile_shader_from_file(const char *file, u32 type)
{
    // TODO(ajeej): need to change this depending if usingn emscripten or windows
    char path[MAX_PATH] = "./data/shaders/";
    strcat(path, file);
    
    const char *src = read_file(path, NULL);
    
    u32 res = compile_shader(src, type);
    
    free((void *)src);
    return res;
}


internal void
load_shader(renderer_t *rb, shader_entry_t entry)
{
    shader_t *shader = stack_push(&rb->shaders);
    memset(shader, 0, sizeof(*shader));
    
    u32 *s = malloc(sizeof(*s)*entry.count);
    int i;
    for (i = 0; i < entry.count; i++) {
        s[i] = compile_shader_from_file(entry.fns[i], 
                                        get_gl_shader_type(entry.types[i]));
    }
    
    shader->id = create_program(s, entry.count);
    
    for (i = 0; i < entry.count; i++)
        glDeleteShader(s[i]);
    
    glUseProgram(shader->id);
    for (i = 0; i < SHADER_LOC_COUNT; i++)
        shader->loc[i] = glGetUniformLocation(shader->id, shader_loc_names[i]);
    
    glUseProgram(0);
}