
#ifndef ASSETS_H
#define ASSETS_H

typedef enum asset_type_t {
    ASSET_SHADER,
} asset_type_t; 

typedef enum shader_type_t {
    VERTEX_FRAGMENT_SHADER,
    COMPUTE_SHADER
} shader_type_t;

typedef struct shader_entry_t {
    char *fn;
    u32 type;
} shader_entry_t;

typedef struct asset_entry_t {
    asset_type_t type;
    
    union {
        shader_entry_t shader;
    };
} asset_entry_t;

typedef struct shader_asset_t {
    char *code;
    u32 type;
} shader_asset_t;

typedef struct asset_manager_t {
    char *data_dir;
    
    STACK(asset_entry_t) *entries;
    STACK(shader_asset_t) *shader_assets;
} asset_manager_t; 

#endif //ASSETS_H
