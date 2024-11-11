

internal void
init_asset_manager(asset_manager_t *am, char *data_dir)
{
    memset(am, 0, sizeof(*am));
    am->data_dir = cstr_dup(data_dir);
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

internal shader_asset_t
load_shader_asset(char *data_dir, shader_entry_t entry)
{
    shader_asset_t asset = {0};
    
    char path[MAX_PATH];
    char *code, ext[10];
    u32 type;
    
    path[0] = 0;
    cstr_cat_many(path, data_dir, "shaders\\", entry.fn);
    type = entry.type;
    
    code = read_file(path, NULL);
    
    get_extension(ext, path);
    
    if (strcmp(ext, "glsl") == 0)
    {
        ASSERT("Not implemented to process glsl");
        
        /*
split_code(code)
push two codes and types and seperate
the vertex and fragment shaders
*/
    }
    else if (strcmp(ext, "wgsl") == 0)
    {
        asset.code = code;
        asset.type = type;
    }
    else
        ASSERT("Invalid shader extension");
    
    return asset;
}

internal void
update_assets(asset_manager_t *am)
{
    u32 entry_count = get_stack_count(am->entries);
    asset_entry_t *entry = NULL;
    for (u32 i = 0; i < entry_count; i++)
    {
        entry = am->entries+i;
        
        switch(entry->type)
        {
            case ASSET_SHADER: {
                shader_asset_t *asset = (shader_asset_t *)stack_push(&am->shader_assets);
                *asset = load_shader_asset(am->data_dir, entry->shader);
            } break;
            
            default: {
                ASSERT("Invalid asset type");
            } break;
        }
    }
    
    stack_clear(am->entries);
}

internal u32
add_shader(renderer_t *rb, asset_manager_t *am, char *fn, u32 type)
{
    u32 id = get_stack_count(am->entries);
    
    shader_entry_t entry = {0};
    entry.fn = cstr_dup(fn);
    entry.type = type;
    
    add_asset(am, ASSET_SHADER, &entry);
    
    return id;
}