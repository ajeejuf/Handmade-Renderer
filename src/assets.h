
#ifndef ASSETS_H
#define ASSETS_H

typedef enum asset_type_t {
    ASSET_SHADER,
    ASSET_FONT,
} asset_type_t; 

typedef enum shader_type_t {
    VERTEX_FRAGMENT_SHADER,
    COMPUTE_SHADER
} shader_type_t;

typedef struct shader_entry_t {
    char *fn;
    u32 type;
} shader_entry_t;

typedef struct font_entry_t {
    char *fn;
    u32 atlas_w, atlas_h;
    u32 ch_start, ch_count; 
    u32 font_size;
} font_entry_t;

typedef struct asset_entry_t {
    asset_type_t type;
    
    union {
        shader_entry_t shader;
        font_entry_t font;
    };
} asset_entry_t;

typedef struct shader_asset_t {
    char *code;
    u32 type;
} shader_asset_t;

typedef struct font_asset_t {
    void *atlas_bitmap;
    u32 atlas_w, atlas_h;
    u32 ch_start, ch_count;
    stbtt_packedchar *packed_chs;
    stbtt_aligned_quad *aligned_quads;
    i32 **kerning;
} font_asset_t;

typedef struct asset_manager_t {
    char *data_dir;
    
    STACK(asset_entry_t) *entries;
    
    u32 shader_count;
    STACK(shader_asset_t) *shader_assets;
    
    u32 font_count;
    STACK(font_asset_t) *font_assets;
} asset_manager_t; 

#endif //ASSETS_H
