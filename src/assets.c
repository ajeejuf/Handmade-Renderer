

internal void
init_asset_manager(asset_manager_t *am)
{
    memset(am, 0, sizeof(*am));
}

internal void
add_asset(asset_manager_t *am, asset_type_t type, void *data)
{
    asset_entry_t *entry = stack_push(&am->entries);
    entry->type = type;
    
    switch(type)
    {
        case ASSET_SHADER: {
            entry->shader = *(shader_entry_t *)data;
        } break;
        
        default: {
            ASSERT("Invalid asset type");
        } break;
    }
}

internal u32
add_shader(renderer_t *rb, asset_manager_t *am, u32 count, ...)
{
    u32 id = get_stack_count(am->entries);
    
    shader_entry_t entry = {0};
    char **fns = malloc(sizeof(*fns)*count);
    shader_type_t *types = malloc(sizeof(*types)*count);
    
    va_list args;
    va_start(args, count);
    
    char *str;
    for(u32 i = 0; i < count; i++) {
        types[i] = va_arg(args, shader_type_t);
        str = va_arg(args, char *);
        fns[i] = cstr_dup(str);
    }
    
    va_end(args);
    
    entry.fns = fns;
    entry.types = types;
    entry.count = count;
    
    add_asset(am, ASSET_SHADER, &entry);
    
    return id;
}