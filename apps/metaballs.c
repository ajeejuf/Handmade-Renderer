
#if __EMSCRIPTEN__

#include <emscripten.h>

#endif

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "app_funcs.h"

#include "shape.c"
#include "renderer.c"
#include "assets.c"
#include "entity.c"


typedef struct screen_grid_t {
    
    union {
        struct {
            u32 width;
            u32 height;
        };
        
        u32 dim[2];
    };
    
    u32 max_res;
    f32 thickness;
    f32 aspect;
    
    u32 res[2];
    
    u32 mesh_id;
    
    u32 line_count;
    
    u32 trans_count;
    mat4 *trans;
} screen_grid_t;

typedef struct metaballs_t {
    v2 *pos;
    v2 *vel;
    f32 *rad;
    
    u32 mesh_id;
    
    u32 count;
    u32 max;
    
    v2 top_left, btm_right;
    v3 color;
    
    mat4 *trans;
} metaballs_t;

u32 c_ids[3];

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


internal f32
random_f32(f32 min, f32 max)
{
    return min + ((f32)rand() / (f32)RAND_MAX) * (max-min);
}

internal v2
random_v2(f32 min, f32 max)
{
    return HMM_V2(random_f32(min, max), random_f32(min, max));
}

internal void
random_f32_array(f32 *a, u32 count, f32 min, f32 max)
{
    for (u32 i = 0; i < count; i++)
        a[i] = random_f32(min, max);
}

internal void
random_v2_array(v2 *v, u32 count, f32 min, f32 max)
{
    for (u32 i = 0; i < count; i++)
        v[i] = random_v2(min, max);
}

internal void
update_screen_grid(screen_grid_t *grid, u32 res)
{
    if (res > grid->max_res) return;
    
    f32 aspect = grid->aspect;
    
    f32 thickness = grid->thickness;
    u32 x_res = res, y_res = (u32)((f32)res * aspect);
    
    grid->res[0] = x_res; grid->res[1] = y_res;
    
    grid->line_count = x_res + y_res - 2;
    
    f32 scale_x = 2.0f/x_res, scale_y = 2.0f/y_res;
    mat4 *cur_trans = grid->trans;
    for (u32 x = 1; x < x_res; x++)
    {
        update_transform(cur_trans, 
                         HMM_V3(scale_x * x - 1.0f, 0.0f, 0.0f),
                         HMM_V3(0.0f, 0.0f, 0.0f),
                         HMM_V3((2.0f + thickness)/grid->width, 2.0f, 1.0f));
        cur_trans++;
    }
    
    for (u32 y = 1; y < y_res; y++)
    {
        update_transform(cur_trans, 
                         HMM_V3(0.0f, scale_y * y - 1.0f, 0.0f),
                         HMM_V3(0.0f, 0.0f, 180.0f),
                         HMM_V3(2.0f, (2.0f + thickness)/grid->height, 1.0f));
        cur_trans++;
    }
}

internal screen_grid_t 
create_screen_grid(renderer_t *rb, 
                   u32 width, u32 height, 
                   u32 res, u32 max_res, 
                   f32 line_thickness)
{
    screen_grid_t grid = {0};
    
    grid.width = width;
    grid.height = height;
    grid.max_res = max_res;
    grid.thickness = line_thickness;
    grid.aspect = height/(f32)width;
    
    grid.mesh_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(1.0f, 1.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    f32 aspect = grid.aspect;
    u32 line_count = max_res-1 + (u32)(max_res*aspect)-1;
    grid.trans = (mat4 *)malloc(sizeof(mat4)*line_count);
    grid.trans_count = line_count;
    
    update_screen_grid(&grid, res);
    
    return grid;
}

internal metaballs_t
create_metaballs(renderer_t *rb, v2 *pos, f32 *rad, v2 *vel, u32 count, u32 max,
                 v3 color, v2 center, f32 width, f32 height)
{
    metaballs_t metaballs = {0};
    
    metaballs.pos = (v2 *)malloc(sizeof(v2)*max);
    metaballs.rad = (f32 *)malloc(sizeof(f32)*max);
    metaballs.vel = (v2 *)malloc(sizeof(v2)*max);
    metaballs.count = count;
    metaballs.max = max;
    metaballs.color = color;
    
    metaballs.top_left = HMM_V2(center.X - width/2, center.Y + height/2);
    metaballs.btm_right = HMM_V2(center.X + width/2, center.Y - height/2);
    
    metaballs.mesh_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    memcpy(metaballs.pos, pos, sizeof(v2)*count);
    memcpy(metaballs.rad, rad, sizeof(f32)*count);
    memcpy(metaballs.vel, vel, sizeof(v2)*count);
    
    return metaballs;
}

internal void
update_metaballs_physics(metaballs_t *metaballs, f32 dt)
{
    v2 tl = metaballs->top_left, br = metaballs->btm_right;
    f32 rad;
    v2 pos, vel;
    for (u32 i = 0; i < metaballs->count; i++)
    {
        pos = metaballs->pos[i];
        vel = metaballs->vel[i];
        rad = metaballs->rad[i];
        
        pos = HMM_AddV2(pos, HMM_MulV2F(vel, dt));
        
        if (pos.X - rad < tl.X) {
            pos.X = tl.X + rad;
            vel.X = -vel.X;
        } else if (pos.X + rad > br.X) {
            pos.X = br.X - rad;
            vel.X = -vel.X;
        }
        
        if (pos.Y - rad < br.Y) {
            pos.Y = br.Y + rad;
            vel.Y = -vel.Y;
        } else if (pos.Y + rad > tl.Y) {
            pos.Y = tl.Y - rad;
            vel.Y = -vel.Y;
        }
        
        metaballs->pos[i] = pos;
        metaballs->vel[i] = vel;
    }
}

internal void
render_screen_grid(renderer_t *rb, screen_grid_t grid)
{
    f32 aspect = grid.height/(f32)grid.width;
    u32 line_count = grid.res[0]-1 + grid.res[1]-1;
    
    push_render_cmd(rb, grid.mesh_id, 0, 0, line_count);
}

internal void
render_metaball_circles(renderer_t *rb, metaballs_t metaballs)
{
    push_render_cmd(rb, metaballs.mesh_id, 0, 0, metaballs.count);
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

global u32 q_id;
global u32 t_id;

INIT_APP(init_app)
{
    srand((u32)time(NULL));
    
    renderer_t *rb = &app->rb;
    asset_manager_t *am = &app->am;
    
    cam = add_camera(rb, HMM_V3(0.0f, 0.0f, 10.0f), 20.0f, 45.0f, 1.0f, 100.0f);
    
    f32 w = 20.0f, h = w * rb->width/(f32)rb->height;
    
    //font_id = process_font_asset(rb, am, font_asset_id);
    
    v2 pos[10];
    f32 rad[10];
    v2 vel[10];
    random_v2_array(pos, 10, 0, 600);
    random_f32_array(rad, 10, 50.0f, 60.0f);
    random_v2_array(vel, 10, -70.0, 70.0);
    
    metaballs = create_metaballs(rb, pos, rad, vel, 10, 20, HMM_V3(1.0f, 0.0f, 0.0f), 
                                 HMM_V2(rb->width/2.0f, rb->height/2.0f), rb->width, rb->height);
    
    grid = create_screen_grid(rb, rb->width, rb->height, 64, rb->width, 1.0f);
    
    //update_screen_grid(&grid, 100);
    
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
    bind_layout_t m_layout[2];
    bind_layout_t f_layout[2];
    bind_layout_t cr_layout[1];
    bind_layout_t cf_layout[2];
    bind_layout_t pf_layout[2];
    
    u32 sampler_id = add_sampler(rb,  ADDRESS_MODE_CLAMPTOEDGE, 
                                 ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                                 FILTER_LINEAR, FILTER_LINEAR);
    
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE);
    ub_ids[2] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE);
    
    u32 g_ids[1], m_ids[3], f_ids[1], cr0_ids[1], cr1_ids[1], cf_ids[1];
    c_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    g_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
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
    
    c_layout[4] = get_buffer_bind_layout_for_struct(&metaballs.color, 0, SHADER_VISIBILITY_COMPUTE,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    
    add_bind_layouts(rb, c_ids[0], c_layout, 3);
    add_bind_layouts(rb, c_ids[1], c_layout+3, 1);
    add_bind_layouts(rb, c_ids[2], c_layout+4, 1);
    
    
    g_layout[0] = get_buffer_bind_layout_for_array(grid.trans, grid.trans_count, 0, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    
    add_bind_layouts(rb, g_ids[0], g_layout, 1);
    
    
    m_layout[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, fb_tex, TEXTURE_ACCESS_WRITEONLY);
    m_layout[1] = get_buffer_bind_layout(grid.res, 0, SHADER_VISIBILITY_COMPUTE, ub_ids[0], BUFFER_TYPE_UNIFORM,
                                         sizeof(*grid.res)*2, 1, 0, 0);
    
    m_ids[0] = c_ids[0];
    add_bind_layouts(rb, m_ids[1], m_layout, 1);
    add_bind_layouts(rb, m_ids[2], m_layout+1, 1);
    
    
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
                                  blend0, blend0, &vb_id, 1, g_ids, 1);
    
    p_id[2] = add_render_pipeline(rb, fb_shader_id, 1, HMM_V3(0.0f, 0.0f, 0.0f),
                                  blend0, blend0, &vb_id, 1, f_ids, 1);
    
}

UPDATE_AND_RENDER(update_and_render)
{
    renderer_t *rb = &app->rb;
    
    update_bind_group_layout(rb, c_ids[0], 0);
    
    update_metaballs_physics(&metaballs, 0.02);
    
    // NOTE(ajeej): Clear framebuffer
    submit_compute_pipeline(rb, c_id[0]);
    
    // NOTE(ajeej): Clear circle texture
    submit_compute_pipeline(rb, c_id[1]);
    
    // NOTE(ajeej): Write metaball pixels
    submit_compute_pipeline(rb, c_id[2]);
    
    // NOTE(ajeej): Write debug circles
    submit_compute_pipeline(rb, c_id[3]);
    
    // NOTE(ajeej): Write circle texture on framebuffer
    start_render_pipeline(rb, p_id[0]);
    {
        push_render_cmd(rb, fb_quad, 0, 0, 1);
    }
    end_render_pipeline(rb);
    
    // NOTE(ajeej): Write grid
    start_render_pipeline(rb, p_id[1]);
    {
        //render_screen_grid(rb, grid);
    }
    end_render_pipeline(rb);
    
    // NOTE(ajeej): Write framebuffer to screen
    start_render_pipeline(rb, p_id[2]);
    {
        clear_color(rb, HMM_V3(0.0f, 0.2f, 0.5f));
        
        push_render_cmd(rb, fb_quad, 0, 0, 1);
    }
    end_render_pipeline(rb);
    
}