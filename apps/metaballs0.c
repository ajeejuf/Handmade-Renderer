
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

u32 c_ids[3];

u32 res_bind_id;

u32 font_asset_id;
u32 font_id;
u32 text_shader_id;
u32 grid_shader_id;
u32 circle_tex_shader_id;
u32 circle_shader_id;
u32 metaball_shader_id;
u32 fb_shader_id;
u32 clear_shader_id ;

u32 text_id;

camera_t *cam;

global u32 c_id[4];
global u32 p_id[3];

global screen_grid_t grid;

global metaballs_t metaballs;

global u32 fb_quad;

global u32 render_debug_circles = 1;
global u32 render_grid = 0;
global u32 pause_balls = 0;
global u32 draw_balls = 0;


global renderer_t *global_rb = NULL;



GET_CALLBACKS(get_callbacks)
{
    func_hash = NULL;
}

LOAD_ASSETS(load_assets)
{
    text_shader_id = add_shader(am, "text_test.wgsl", VERTEX_FRAGMENT_SHADER);
    grid_shader_id = add_shader(am, "grid.wgsl", VERTEX_FRAGMENT_SHADER);
    circle_shader_id = add_shader(am, "circle.wgsl", COMPUTE_SHADER);
    metaball_shader_id = add_shader(am, "gpu_metaballs.wgsl", COMPUTE_SHADER);
    fb_shader_id = add_shader(am, "framebuffer.wgsl", VERTEX_FRAGMENT_SHADER);
    clear_shader_id = add_shader(am, "clear.wgsl", COMPUTE_SHADER);
    
    //font_asset_id = add_font(am, "fonts/hack/Hack-Regular.ttf", 1024*5, 1024*5, ' ', '~'-' ', 64*5);
}

INIT_APP(init_app)
{
    srand((u32)time(NULL));
    
    renderer_t *rb = &app->rb;
    global_rb = rb;
    
    asset_manager_t *am = &app->am;
    
    cam = add_camera(rb, HMM_V3(0.0f, 0.0f, 10.0f), 2.0f, 45.0f, 1.0f, 100.0f);
    
    f32 w = 20.0f, h = w * rb->width/(f32)rb->height;
    
    //font_id = process_font_asset(rb, am, font_asset_id);
    
    v2 pos[10];
    f32 rad[10];
    v2 vel[10];
    random_v2_array(pos, 10, 0, rb->height);
    random_f32_array(rad, 10, rb->height/15.0f, rb->height/8.0f);
    random_v2_array(vel, 10, -70.0, 70.0);
    
    metaballs = create_metaballs(rb, pos, rad, vel, 10, 20, HMM_V3(1.0f, 0.5f, 0.0f), HMM_V3(1.0, 0.0, 0.0),
                                 rb->width, rb->height);
    
    grid = create_screen_grid(rb, rb->width, rb->height, 64, rb->width, 1.0f);
    
    fb_quad = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    //text_id = add_text(app, font_id, strlen("Hello World"), HMM_V3(0.0f, 0.0f, 0.0f), 1.0f/800.0f);
    
    
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
    
    
    u32 ub_ids[3];
    bind_layout_t c_layout[5];
    bind_layout_t g_layout[3];
    bind_layout_t m_layout[3];
    bind_layout_t f_layout[2];
    bind_layout_t cr_layout[1];
    bind_layout_t cf_layout[2];
    bind_layout_t pf_layout[2];
    
    u32 sampler_id = add_sampler(rb,  ADDRESS_MODE_CLAMPTOEDGE, 
                                 ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                                 FILTER_LINEAR, FILTER_LINEAR);
    
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM, 0);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE, 0);
    ub_ids[2] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE, 0);
    
    u32 g_ids[2], m_ids[3], f_ids[1], cr0_ids[1], cr1_ids[1], cf_ids[1];
    c_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    g_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    g_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    m_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    m_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    m_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    f_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    cr0_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    cr1_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    cf_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    c_layout[0] = get_buffer_bind_layout_for_array(metaballs.pos, metaballs.count, 0, SHADER_VISIBILITY_COMPUTE,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    c_layout[1] = get_buffer_bind_layout_for_array(metaballs.rad, metaballs.count, 1, SHADER_VISIBILITY_COMPUTE,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    c_layout[2] = get_buffer_bind_layout_for_struct(&metaballs.count, 2, SHADER_VISIBILITY_COMPUTE,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    c_layout[3] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, c_tex, TEXTURE_ACCESS_WRITEONLY);
    
    c_layout[4] = get_buffer_bind_layout_for_struct(&metaballs.debug_color, 0, SHADER_VISIBILITY_COMPUTE,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    
    add_bind_layouts(rb, c_ids[0], c_layout, 3);
    add_bind_layouts(rb, c_ids[1], c_layout+3, 1);
    add_bind_layouts(rb, c_ids[2], c_layout+4, 1);
    
    
    g_layout[0] = get_buffer_bind_layout_for_array(grid.trans, grid.trans_count, 0, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    
    g_layout[1] = get_buffer_bind_layout_for_struct(&cam->view, 0, SHADER_VISIBILITY_VERTEX,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    g_layout[2] =  get_buffer_bind_layout_for_struct(&cam->orth, 1, SHADER_VISIBILITY_VERTEX,
                                                     ub_ids[0], BUFFER_TYPE_UNIFORM);;
    
    add_bind_layouts(rb, g_ids[0], g_layout, 1);
    add_bind_layouts(rb, g_ids[1], g_layout+1, 2);
    
    
    m_layout[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, fb_tex, TEXTURE_ACCESS_WRITEONLY);
    m_layout[1] = get_buffer_bind_layout(grid.res, 0, SHADER_VISIBILITY_COMPUTE, ub_ids[0], BUFFER_TYPE_UNIFORM,
                                         sizeof(*grid.res)*2, 1, 0, 0);
    m_layout[2] = get_buffer_bind_layout_for_struct(&metaballs.color, 1, SHADER_VISIBILITY_COMPUTE,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    m_ids[0] = c_ids[0];
    add_bind_layouts(rb, m_ids[1], m_layout, 1);
    add_bind_layouts(rb, m_ids[2], m_layout+1, 2);
    res_bind_id = m_ids[2];
    
    
    f_layout[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, fb_tex);
    f_layout[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id, SAMPLER_TYPE_FILTERING);
    
    add_bind_layouts(rb, f_ids[0], f_layout, 2);
    
    
    cr_layout[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, fb_tex, TEXTURE_ACCESS_WRITEONLY);
    
    add_bind_layouts(rb, cr0_ids[0], cr_layout, 1);
    
    cr_layout[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, c_tex, TEXTURE_ACCESS_WRITEONLY);
    
    add_bind_layouts(rb, cr1_ids[0], cr_layout, 1);
    
    
    cf_layout[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, c_tex);
    cf_layout[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id, SAMPLER_TYPE_FILTERING);
    
    add_bind_layouts(rb, cf_ids[0], cf_layout, 2);
    
    
    c_id[0] = add_compute_pipeline(rb, clear_shader_id,  cr0_ids, 1, 
                                   ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    c_id[1] = add_compute_pipeline(rb, clear_shader_id,  cr1_ids, 1, 
                                   ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    c_id[2] = add_compute_pipeline(rb, metaball_shader_id, m_ids, 3, 
                                   ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    c_id[3] = add_compute_pipeline(rb, circle_shader_id, c_ids, 3, ceilf(rb->width/16.0f), ceilf(rb->height/16.0f), 1);
    
    blend_comp_t blend0 = (blend_comp_t){ BLEND_FACTOR_SRCALPHA, BLEND_FACTOR_ONEMINUSSRCALPHA, BLEND_OP_ADD };
    blend_comp_t blend1 = (blend_comp_t){ BLEND_FACTOR_ONE, BLEND_FACTOR_ONE, BLEND_OP_ADD };
    
    
    p_id[0] = add_render_pipeline(rb, fb_shader_id, fb_tex, HMM_V3(0.0f, 0.0f, 0.0f), 
                                  blend0, blend0, &vb_id, 1, cf_ids, 1);
    
    p_id[1] = add_render_pipeline(rb, grid_shader_id, fb_tex, HMM_V3(0.0f, 0.0f, 0.0f),
                                  blend0, blend0, &vb_id, 1, g_ids, 2);
    
    p_id[2] = add_render_pipeline(rb, fb_shader_id, 1, HMM_V3(0.0f, 0.0f, 0.0f),
                                  blend0, blend0, &vb_id, 1, f_ids, 1);
    
}

UPDATE_AND_RENDER(update_and_render)
{
    renderer_t *rb = &app->rb;
    
    if (!pause_balls) {
        update_metaballs_physics(&metaballs, 0.02);
        update_bind_group_layout(rb, c_ids[0], 0);
    }
    
    // NOTE(ajeej): Clear framebuffer
    submit_compute_pipeline(rb, c_id[0]);
    
    if (render_debug_circles) {
        // NOTE(ajeej): Clear circle texture
        submit_compute_pipeline(rb, c_id[1]);
    }
    
    if (draw_balls) {
        // NOTE(ajeej): Write metaball pixels
        submit_compute_pipeline(rb, c_id[2]);
    }
    
    if (render_debug_circles) {
        // NOTE(ajeej): Write debug circles
        submit_compute_pipeline(rb, c_id[3]);
    }
    
    
    if (render_debug_circles) {
        // NOTE(ajeej): Write circle texture on framebuffer
        start_render_pipeline(rb, p_id[0]);
        {
            push_render_cmd(rb, fb_quad, 0, 0, 1);
        }
        end_render_pipeline(rb);
    }
    
    if (render_grid) {
        // NOTE(ajeej): Write grid
        start_render_pipeline(rb, p_id[1]);
        {
            //render_screen_grid(rb, grid);
        }
        end_render_pipeline(rb);
    }
    
    // NOTE(ajeej): Write framebuffer to screen
    start_render_pipeline(rb, p_id[2]);
    {
        clear_color(rb, HMM_V3(0.0f, 0.2f, 0.5f));
        
        push_render_cmd(rb, fb_quad, 0, 0, 1);
    }
    end_render_pipeline(rb);
    
}