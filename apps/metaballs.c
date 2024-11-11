
#if __EMSCRIPTEN__

#include <emscripten.h>

#endif

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "app_funcs.h"

#include "renderer.c"
#include "assets.c"
#include "shape.c"
#include "entity.c"

u8 *tex_data;
u32 width = 128, height = 128;

global u32 c_id;
global u32 p_id;
global u32 q_id;

INIT_APP(init_app)
{
    renderer_t *rb = &app->rb;
    asset_manager_t *am = &app->am;
    
    u32 cs_id = add_shader(rb, am, "metaball_distance_field.wgsl", COMPUTE_SHADER);
    u32 s_id = add_shader(rb, am, "metaballs.wgsl", VERTEX_FRAGMENT_SHADER);
    
    q_id = create_quad(rb, HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    tex_data = (u8 *)malloc(width*height*4);
    memset(tex_data, 255, width*height*4);
    
    u32 tex_id = add_texture(rb, tex_data, width, height, TEXTURE_FORMAT_RGBA8U_NORM, 
                             TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_COPY_DST |
                             TEXTURE_USAGE_TEXTURE_BINDING | TEXTURE_USAGE_STORAGE_BINDING);
    
    u32 vb_id = add_vertex_buffer(rb, rb->verts, get_stack_count(rb->verts),
                                  ATTRIBUTE_VERTEX, 0, MODE_VERTEX);
    
    
    bind_layout_t layouts[2];
    layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, 
                                                 tex_id, TEXTURE_ACCESS_WRITEONLY);
    
    layouts[1] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, tex_id);
    
    
    u32 bg_ids[2];
    bg_ids[0] = add_bind_group(rb, layouts, 1, BIND_GROUP_TYPE_FRAME);
    bg_ids[1] = add_bind_group(rb, layouts+1, 1, BIND_GROUP_TYPE_FRAME);
    
    
    c_id = add_compute_pipeline(rb, cs_id, bg_ids, 1, 8, 8, 1);
    p_id = add_render_pipeline(rb, s_id, &vb_id, 1, bg_ids+1, 1);
}

UPDATE_AND_RENDER(update_and_render)
{
    use_render_pipeline(&app->rb, p_id);
    
    push_render_cmd(&app->rb, q_id, 0, 0);
    
    submit_pipeline(&app->rb, PIPELINE_TYPE_COMPUTE, c_id);
    submit_pipeline(&app->rb, PIPELINE_TYPE_RENDER, p_id);
}