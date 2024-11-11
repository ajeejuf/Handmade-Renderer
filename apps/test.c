
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

global u32 id = 0;
global u32 mesh_id; 

global v3 color0 = {0};
global v3 color1 = {0};

f32 offsetx[2] = { 0.5f, -0.5f };
f32 offsety[2] = { 0.5f, -0.5f };

INIT_APP(init_app)
{
    renderer_t *rb = &app->rb;
    asset_manager_t *am = &app->am;
    
    u32 ub_ids[2];
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    
    u32 s_id = add_shader(rb, am, "shader.wgsl");
    
    //u32 c_id = add_shader(rb, am, "compute.wgsl");
    
    mesh_id = create_triangle(rb, HMM_V4(0.0f, 0.0f, 1.0f, 1.0f));
    
    u32 vb_id = add_vertex_buffer(rb, rb->verts, get_stack_count(rb->verts),
                                  ATTRIBUTE_VERTEX, 0, MODE_VERTEX);
    
    color0 = HMM_V3(0.0f, 0.0f, 0.0f);
    color1 = HMM_V3(0.0f, 0.0f, 1.0f);
    
    bind_layout_t layouts[4];
    layouts[0] = get_buffer_bind_layout_for_struct(&color0, 0, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[0], BUFFER_TYPE_UNIFORM);
    layouts[1] = get_buffer_bind_layout_for_struct(&color1, 1, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    layouts[2] = get_dynamic_buffer_bind_layout_for_struct(offsetx, ARRAY_COUNT(offsetx),
                                                           0, SHADER_VISIBILITY_VERTEX,
                                                           ub_ids[1], BUFFER_TYPE_UNIFORM,
                                                           offsetof(render_cmd_t, trans_id));
    layouts[3] = get_dynamic_buffer_bind_layout_for_struct(offsety, ARRAY_COUNT(offsety),
                                                           1, SHADER_VISIBILITY_VERTEX,
                                                           ub_ids[1], BUFFER_TYPE_UNIFORM,
                                                           offsetof(render_cmd_t, mat_id));
    
    u32 bg_ids[2];
    bg_ids[0] = add_bind_group(rb, layouts, 2, BIND_GROUP_TYPE_FRAME);
    bg_ids[1] = add_bind_group(rb, layouts+2, 2, BIND_GROUP_TYPE_DRAW);
    
    id = add_render_pipeline(rb, s_id, &vb_id, 1, bg_ids, 2);
    
    //c_id = add_compute_pipeline(0, 0, 0, 0);
}

UPDATE_AND_RENDER(update_and_render)
{
    use_render_pipeline(&app->rb, id);
    
    push_render_cmd(&app->rb, mesh_id, 0, 0);
    push_render_cmd(&app->rb, mesh_id, 1, 0);
    
    submit_pipeline(&app->rb, PIPELINE_TYPE_RENDER, id);
}