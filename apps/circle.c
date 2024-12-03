#if __EMSCRIPTEN__

#include <emscripten.h>

#endif

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "callback.h"
#include "app_funcs.h"

#include "shape.c"
#include "renderer.c"
#include "assets.c"
#include "entity.c"

#include "metaballs.h"

global metaballs_t metaballs = {0};
global screen_grid_t grid = {0};

global f32 circle_range = 1.0f;

global u32 cp_ids[2], rp_ids[2];

global u32 fb_quad;

global u32 clear_shader_id;
global u32 pixel_shader_id;
global u32 fb_shader_id;
global u32 grid_shader_id;

global u32 res_bind_id;
global u32 grid_trans_bg_id;

global renderer_t *global_rb;

global callback_info_t callback_info[2];

void set_range_2(void *args) {
    f32 *range = (f32 *)args;
    
    circle_range = *range;
    
    LOG("set circle_range to %f", circle_range);
    
    update_bind_group_layout(global_rb, res_bind_id, 2);
}

void set_res_2(void *args) {
    u32 *res = (u32 *)args;
    
    update_screen_grid(&grid, *res);
    update_bind_group_layout(global_rb, res_bind_id, 0);
    update_bind_group_layout(global_rb, grid_trans_bg_id, 0);
}


GET_CALLBACKS(get_callbacks)
{
    
    callback_info[0] = get_callback_info("set_range", strlen("set_range"),
                                         TYPE_VOID, set_range_2);
    
    callback_info[1] = get_callback_info("set_res", strlen("set_res"),
                                         TYPE_VOID, set_res_2);
    
    hashmap_create(2, *func_hash);
    
    ASSERT_LOG(!hashmap_put(*func_hash, "set_range", strlen("set_range"), callback_info),
               "Failed to hash %s function.", "set_range");
    
    ASSERT_LOG(!hashmap_put(*func_hash, "set_res", strlen("set_res"), callback_info+1),
               "Failed to hash %s function.", "set_res");
    
}

LOAD_ASSETS(load_assets)
{
    clear_shader_id = add_shader(am, "clear.wgsl", COMPUTE_SHADER);
    pixel_shader_id = add_shader(am, "gpu_circle.wgsl", COMPUTE_SHADER);
    fb_shader_id = add_shader(am, "framebuffer.wgsl", VERTEX_FRAGMENT_SHADER);
    grid_shader_id = add_shader(am, "grid.wgsl", VERTEX_FRAGMENT_SHADER);
    
    /*font_asset_id = add_font(am, "fonts\\hack\\Hack-Regular.ttf", 1024, 512, ' ', '~'-' ', 64);*/
}

INIT_APP(init_app)
{
    renderer_t *rb = &app->rb;
    global_rb = rb;
    
    asset_manager_t *am = &app->am;
    camera_t *cam = add_camera(rb, HMM_V3(0.0f, 0.0f, 10.0f), 2.0f, 45.0f, 1.0f, 100.0f);
    
    f32 largest_dim = MAX(rb->width, rb->height);
    v2 pos = HMM_V2(rb->width/2.0f, rb->height/2.0f);
    f32 rad = largest_dim/4.0f;
    v2 vel = HMM_V2(0.0f, 0.0f);
    metaballs = create_metaballs(rb, &pos, &rad, &vel, 1, 1, HMM_V3(1.0f, 0.5f, 0.0f), HMM_V3(1.0f, 0.0f, 0.0f),
                                 rb->width, rb->height);
    
    
    grid = create_screen_grid(rb, rb->width, rb->height, 16.0f, rb->width, 1.0f);
    
    fb_quad = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    
    u8 *fb_data = (u8 *)malloc(rb->width*rb->height*4);
    memset(fb_data, 0, rb->width*rb->height*4);
    u32 fb_tex = add_texture(rb, fb_data, rb->width, rb->height, TEXTURE_FORMAT_RGBA8U_NORM,
                             TEXTURE_USAGE_COPY_DST | TEXTURE_USAGE_STORAGE_BINDING |
                             TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_TEXTURE_BINDING |
                             TEXTURE_USAGE_RENDER_ATTACHMENT);
    
    u8 *c_data = (u8 *)malloc(rb->width*rb->height*4);
    memset(c_data, 0, rb->width*rb->height*4);
    u32 c_tex = add_texture(rb, c_data, rb->width, rb->height, TEXTURE_FORMAT_RGBA8U_NORM,
                            TEXTURE_USAGE_COPY_DST | TEXTURE_USAGE_STORAGE_BINDING |
                            TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_TEXTURE_BINDING);
    
    
    u32 vb_id = add_vertex_buffer(rb, rb->verts, get_stack_count(rb->verts),
                                  ATTRIBUTE_VERTEX, 0, MODE_VERTEX);
    
    
    
    
    
    
    u32 ub_ids[4];
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM, 0);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE, 0);
    ub_ids[2] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM, 0);
    ub_ids[3] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_COPY_SRC | BUFFER_FLAG_STORAGE, 0);
    
    
    u32 sampler_id = add_sampler(rb,  ADDRESS_MODE_CLAMPTOEDGE, 
                                 ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                                 FILTER_LINEAR, FILTER_LINEAR);
    
    
    // NOTE(ajeej): Pixel Shader
    
    u32 px_bg_ids[3];
    bind_layout_t px_layouts[7];
    
    px_bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    px_bg_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    px_bg_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    px_layouts[0] = get_buffer_bind_layout_for_array(metaballs.pos, metaballs.count, 0,
                                                     SHADER_VISIBILITY_COMPUTE,
                                                     ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    px_layouts[1] = get_buffer_bind_layout_for_array(metaballs.rad, metaballs.count, 1,
                                                     SHADER_VISIBILITY_COMPUTE,
                                                     ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    px_layouts[2] = get_buffer_bind_layout_for_struct(&metaballs.count, 2, SHADER_VISIBILITY_COMPUTE,
                                                      ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    px_layouts[3] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, fb_tex,
                                                    TEXTURE_ACCESS_WRITEONLY);
    
    px_layouts[4] = get_buffer_bind_layout(grid.res, 0, SHADER_VISIBILITY_COMPUTE, ub_ids[0], BUFFER_TYPE_UNIFORM,
                                           sizeof(*grid.res)*2, 1, 0, 0);
    px_layouts[5] = get_buffer_bind_layout_for_struct(&metaballs.color, 1, SHADER_VISIBILITY_COMPUTE,
                                                      ub_ids[0], BUFFER_TYPE_UNIFORM);
    px_layouts[6] = get_buffer_bind_layout_for_struct(&circle_range, 2, SHADER_VISIBILITY_COMPUTE,
                                                      ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    
    add_bind_layouts(rb, px_bg_ids[0], px_layouts, 3);
    add_bind_layouts(rb, px_bg_ids[1], px_layouts+3, 1);
    add_bind_layouts(rb, px_bg_ids[2], px_layouts+4, 3);
    
    res_bind_id = px_bg_ids[2];
    
    
    
    // NOTE(ajeej): Clear Framebuffer
    u32 cf_bg_ids[1];
    bind_layout_t cf_layouts[1];
    
    cf_bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    cf_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, fb_tex,
                                                    TEXTURE_ACCESS_WRITEONLY);
    
    add_bind_layouts(rb, cf_bg_ids[0], cf_layouts, 1);
    
    
    // NOTE(ajeej): Write Framebuffer
    u32 fb_bg_ids[1];
    bind_layout_t fb_layouts[2];
    
    fb_bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    fb_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, fb_tex);
    fb_layouts[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id, 
                                            SAMPLER_TYPE_FILTERING);
    
    add_bind_layouts(rb, fb_bg_ids[0], fb_layouts, 2);
    
    
    // NOTE(ajeej): Render Grid
    u32 g_bg_ids[2];
    bind_layout_t g_layouts[3];
    
    g_bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    g_bg_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    g_layouts[0] = get_buffer_bind_layout_for_array(grid.trans, grid.trans_count, 0, SHADER_VISIBILITY_VERTEX,
                                                    ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    
    g_layouts[1] = get_buffer_bind_layout_for_struct(&cam->view, 0, SHADER_VISIBILITY_VERTEX,
                                                     ub_ids[0], BUFFER_TYPE_UNIFORM);
    g_layouts[2] = get_buffer_bind_layout_for_struct(&cam->orth, 1, SHADER_VISIBILITY_VERTEX,
                                                     ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    add_bind_layouts(rb, g_bg_ids[0], g_layouts, 1);
    add_bind_layouts(rb, g_bg_ids[1], g_layouts+1, 2);
    
    grid_trans_bg_id = g_bg_ids[0];
    
    
    cp_ids[0] = add_compute_pipeline(rb, clear_shader_id, cf_bg_ids, 1,
                                     ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    cp_ids[1] = add_compute_pipeline(rb, pixel_shader_id, px_bg_ids, 3,
                                     ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    blend_comp_t blend = (blend_comp_t){ 
        BLEND_FACTOR_SRCALPHA, 
        BLEND_FACTOR_ONEMINUSSRCALPHA, 
        BLEND_OP_ADD 
    };
    rp_ids[0] = add_render_pipeline(rb, grid_shader_id, fb_tex, HMM_V3(0.0, 0.0, 0.0),
                                    blend, blend, &vb_id, 1, g_bg_ids, 2);
    
    
    rp_ids[1] = add_render_pipeline(rb, fb_shader_id, 1, HMM_V3(0.0f, 0.0f, 0.0f),
                                    blend, blend, &vb_id, 1, fb_bg_ids, 1);
}

UPDATE_AND_RENDER(update_and_render)
{
    renderer_t *rb = &app->rb;
    
    
    // NOTE(ajeej): Clear Framebuffer
    submit_compute_pipeline(rb, cp_ids[0]);
    
    // NOTE(ajeej): Fill Pixels
    submit_compute_pipeline(rb, cp_ids[1]);
    
    // NOTE(ajeej): Render Grid
    start_render_pipeline(rb, rp_ids[0]);
    {
        render_screen_grid(rb, grid);
    }
    end_render_pipeline(rb);
    
    
    // NOTE(ajeej): Write Framebuffer
    start_render_pipeline(rb, rp_ids[1]);
    {
        clear_color(rb, HMM_V3(0.0f, 0.2f, 0.5f));
        
        push_render_cmd(rb, fb_quad, 0, 0, 1);
    }
    end_render_pipeline(rb);
}