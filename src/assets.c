

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
        
        case ASSET_FONT: {
            entry->font = *(font_entry_t *)data;
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
    cstr_cat_many(path, data_dir, "shaders/", entry.fn);
    type = entry.type;
    
    code = (char *)read_file(path, NULL);
    
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

internal font_asset_t
load_font_asset(char *data_dir, font_entry_t entry)
{
    font_asset_t asset = {0};
    
    void *atlas_bitmap = (void *)malloc(entry.atlas_w*entry.atlas_h);
    
    stbtt_packedchar *packed_chs = (stbtt_packedchar *)malloc(sizeof(*packed_chs)*entry.ch_count);
    stbtt_aligned_quad *aligned_quads = (stbtt_aligned_quad *)malloc(sizeof(*aligned_quads)*entry.ch_count);
    memset(packed_chs, 0, sizeof(*packed_chs)*entry.ch_count);
    memset(aligned_quads, 0, sizeof(*aligned_quads)*entry.ch_count);
    
    i32 *kern_data = (i32 *)malloc(sizeof(i32)*entry.ch_count*entry.ch_count);
    i32 **kerning = (i32 **)malloc(sizeof(i32 *)*entry.ch_count);
    for (u32 i = 0; i < entry.ch_count; i++)
    {
        kerning[i] = kern_data;
        kern_data += entry.ch_count;
    }
    
    char path[MAX_PATH];
    path[0] = 0;
    
    cstr_cat_many(path, data_dir, entry.fn);
    u8 *font_data = (u8 *)read_file(path, NULL);
    
    i32 font_count = stbtt_GetNumberOfFonts(font_data);
    if (font_count == -1)
        ASSERT_LOG(0, "The font file %s does not correspond to valid font data", entry.fn);
    
    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, font_data, stbtt_GetFontOffsetForIndex(font_data, 0));
    
    stbtt_pack_context ctx = {0};
    
    stbtt_PackBegin(&ctx,
                    (unsigned char *)atlas_bitmap,
                    entry.atlas_w,
                    entry.atlas_h,
                    0, 1, NULL);
    
    stbtt_PackSetOversampling(&ctx, 2, 2);
    
    stbtt_PackFontRange(&ctx,
                        font_data,
                        0,
                        entry.font_size,
                        entry.ch_start, entry.ch_count,
                        packed_chs);
    
    stbtt_PackEnd(&ctx);
    
    for (u32 i = 0; i < entry.ch_count; i++)
    {
        f32 x, y;
        
        stbtt_GetPackedQuad(packed_chs,
                            entry.atlas_w, entry.atlas_h,
                            i,
                            &x, &y,
                            aligned_quads+i,
                            0);
    }
    
    
    for (u32 i = 0; i < entry.ch_count; i++)
    {
        for (u32 j = 0; j < entry.ch_count; j++)
        {
            char prev_c = entry.ch_start + i;
            char cur_c = entry.ch_start + j;
            kerning[i][j] = stbtt_GetCodepointKernAdvance(&font_info, prev_c, cur_c);
        }
    }
    
    // NOTE(ajeej): test if it worked by outputting atlas
    /*stbi_write_png("testAtlas.png", 
                   entry.atlas_w, entry.atlas_h,
                   1,
                   atlas_bitmap,
                   entry.atlas_w);*/
    
    asset.atlas_bitmap = atlas_bitmap;
    asset.atlas_w = entry.atlas_w;
    asset.atlas_h = entry.atlas_h;
    asset.ch_start = entry.ch_start;
    asset.ch_count = entry.ch_count;
    asset.packed_chs = packed_chs;
    asset.aligned_quads = aligned_quads;
    asset.kerning = kerning;
    
    free(font_data);
    
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
                
                free(entry->shader.fn);
            } break;
            
            case ASSET_FONT: {
                font_asset_t *asset = (font_asset_t *)stack_push(&am->font_assets);
                *asset = load_font_asset(am->data_dir, entry->font);
                
                free(entry->font.fn);
            } break;
            
            default: {
                ASSERT("Invalid asset type");
            } break;
        }
    }
    
    stack_clear(am->entries);
}

internal u32
add_shader(asset_manager_t *am, char *fn, u32 type)
{
    u32 id = am->shader_count++;
    
    shader_entry_t entry = {0};
    entry.fn = cstr_dup(fn);
    entry.type = type;
    
    add_asset(am, ASSET_SHADER, &entry);
    
    return id;
}

internal u32
add_font(asset_manager_t *am, char *fn, 
         u32 atlas_w, u32 atlas_h,
         u32 ch_start, u32 ch_count,
         u32 font_size)
{
    u32 id = am->font_count++;
    
    font_entry_t entry = {0};
    
    entry.fn = cstr_dup(fn);
    entry.atlas_w = atlas_w;
    entry.atlas_h = atlas_h;
    entry.ch_start = ch_start;
    entry.ch_count = ch_count;
    entry.font_size = font_size;
    
    add_asset(am, ASSET_FONT, &entry);
    
    return id;
}