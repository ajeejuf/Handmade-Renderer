
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

/*typedef struct metaballs_t {
    u32 *e_ids;
    u32 count;
    
    v3 color;
} metaballs_t;

metaballs_t metaballs;*/

typedef struct screen_grid_t {
    u32 width, height;
    u32 res, max_res;
    f32 thickness;
    f32 aspect;
    
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

u8 *c_tex_data;

u32 text_id;

camera_t *cam;

global u32 c_id[1];
global u32 p_id[2];

global screen_grid_t grid;

global metaballs_t metaballs;

/*internal void
process_input(app_t *app)
{
    renderer_t *rb = &app->rb;
    
    mat4 *trans = rb->transforms;
    v3 pos = HMM_V3(trans->Elements[3][0], trans->Elements[3][1], 0.0);
    
    v3 up = HMM_V3(0.0f, 1.0f, 0.0f);
    v3 right = HMM_V3(1.0f, 0.0f, 0.0f);
    
    if (is_key_down(app, KEY_W))
        pos = HMM_AddV3(pos, HMM_MulV3F(up, speed));
    if (is_key_down(app, KEY_S))
        pos = HMM_AddV3(pos, HMM_MulV3F(up, -speed));
    if (is_key_down(app, KEY_A))
        pos = HMM_AddV3(pos, HMM_MulV3F(right, -speed));
    if (is_key_down(app, KEY_D))
        pos = HMM_AddV3(pos, HMM_MulV3F(right, speed));
    
    trans->Elements[3][0] = pos.X;
    trans->Elements[3][1] = pos.Y;
    
    update_bind_group_layout(&app->rb, bg_ids[6], 0);
}

internal void
create_textures(renderer_t *rb, asset_manager_t *am)
{
    // NOTE(ajeej): Add Textures
    tex_data = (u8 *)malloc(width*height*4);
    g_tex_data = (u8 *)malloc(width*height*4);
    fb_data = (u8 *)malloc(rb->width*rb->height*4);
    prev_fb_data = (u8 *)malloc(rb->width*rb->height*4);
    
    memset(tex_data, 255, width*height*4);
    memset(g_tex_data, 255, width*height*4);
    memset(fb_data, 0, rb->width*rb->height*4);
    memset(prev_fb_data, 0, rb->width*rb->height*4);
    
    
    df_tex_id = add_texture(rb, tex_data, width, height, TEXTURE_FORMAT_RGBA8U_NORM, 
                            TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_COPY_DST |
                            TEXTURE_USAGE_TEXTURE_BINDING | TEXTURE_USAGE_STORAGE_BINDING);
    
    gdf_tex_id = add_texture(rb, g_tex_data, width, height, TEXTURE_FORMAT_RGBA8U_NORM,
                             TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_COPY_DST |
                             TEXTURE_USAGE_TEXTURE_BINDING | TEXTURE_USAGE_STORAGE_BINDING);
    
    fb_id = add_texture(rb, fb_data, rb->width, rb->height, TEXTURE_FORMAT_RGBA8U_NORM,
                        TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_COPY_DST | 
                        TEXTURE_USAGE_TEXTURE_BINDING | TEXTURE_USAGE_STORAGE_BINDING |
                        TEXTURE_USAGE_RENDER_ATTACHMENT);
    
    prev_fb_id = add_texture(rb, prev_fb_data, rb->width, rb->height, TEXTURE_FORMAT_RGBA8U_NORM,
                             TEXTURE_USAGE_COPY_DST | TEXTURE_USAGE_TEXTURE_BINDING |
                             TEXTURE_USAGE_STORAGE_BINDING);
    
    sampler_id = add_sampler(rb, ADDRESS_MODE_CLAMPTOEDGE, 
                             ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                             FILTER_LINEAR, FILTER_LINEAR);
    
    
    font_asset_t *font = am->font_assets + font_id;
    atlas_id = add_texture(rb, font->atlas_bitmap, font->atlas_w, font->atlas_h,
                           TEXTURE_FORMAT_RGBA8U_NORM,
                           TEXTURE_USAGE_COPY_DST | TEXTURE_USAGE_TEXTURE_BINDING);
}

internal void
create_uniforms(renderer_t *rb)
{
    // NOTE(ajeej): Configure uniforms
    u32 ub_ids[2];
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    
    bind_layout_t meta_df_layouts[1];
    bind_layout_t g_blur_layouts[2];
    bind_layout_t clear_layouts[1];
    bind_layout_t clear1_layouts[1];
    bind_layout_t meta_layouts[7];
    bind_layout_t filter_layouts[2];
    
    // NOTE(ajeej): Specify bind group layouts
    u32 bg_layout_counts[8] = {
        1, 2, 1, 1, 3, 2, 2, 2
    };
    
    bind_layout_t *bg_layout_table[8] = {
        meta_df_layouts,
        g_blur_layouts,
        clear_layouts,
        clear1_layouts,
        meta_layouts,
        meta_layouts+3,
        meta_layouts+5,
        filter_layouts
    };
    
    // NOTE(ajeej): Add bind groups
    bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[3] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[4] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    bg_ids[5] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    bg_ids[6] = add_bind_group(rb, BIND_GROUP_TYPE_DRAW);
    
    bg_ids[7] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    // NOTE(ajeej): Get bind group layouts
    meta_df_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, 
                                                         df_tex_id, TEXTURE_ACCESS_WRITEONLY);
    
    g_blur_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, df_tex_id);
    
    g_blur_layouts[1] = get_storage_texture_bind_layout(1, SHADER_VISIBILITY_COMPUTE,
                                                        gdf_tex_id, TEXTURE_ACCESS_WRITEONLY);
    
    clear_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE,
                                                       fb_id, TEXTURE_ACCESS_WRITEONLY);
    
    clear1_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE,
                                                        prev_fb_id, TEXTURE_ACCESS_WRITEONLY);
    
    
    meta_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, gdf_tex_id);
    
    meta_layouts[1] = get_texture_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, prev_fb_id);
    
    meta_layouts[2] = get_sampler_bind_layout(2, SHADER_VISIBILITY_FRAGMENT, sampler_id,
                                              SAMPLER_TYPE_FILTERING);
    
    meta_layouts[3] = get_buffer_bind_layout_for_struct(&cam->orth, 0, SHADER_VISIBILITY_VERTEX, 
                                                        ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    meta_layouts[4] = get_buffer_bind_layout_for_struct(&cam->view, 1, SHADER_VISIBILITY_VERTEX, 
                                                        ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    meta_layouts[5] = get_dynamic_buffer_bind_layout_for_struct(rb->transforms, metaballs.count, 0,
                                                                SHADER_VISIBILITY_VERTEX, ub_ids[1], BUFFER_TYPE_UNIFORM,
                                                                offsetof(render_cmd_t, trans_id));
    
    meta_layouts[6] = get_dynamic_buffer_bind_layout_for_struct(metaball_colors, 4, 1, 
                                                                SHADER_VISIBILITY_FRAGMENT, 
                                                                ub_ids[1], BUFFER_TYPE_UNIFORM,
                                                                offsetof(render_cmd_t, mat_id));
    
    
    filter_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, fb_id);
    
    filter_layouts[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id,
                                                SAMPLER_TYPE_FILTERING);
    
    // NOTE(ajeej): Set bind layouts
    u32 layout_count;
    bind_layout_t *layouts;
    for (u32 bg_idx = 0; bg_idx < ARRAY_COUNT(bg_ids); bg_idx++)
    {
        layout_count = bg_layout_counts[bg_idx];
        layouts = bg_layout_table[bg_idx];
        
        add_bind_layouts(rb, bg_ids[bg_idx], layouts, layout_count);
    }
    
}

internal f32
random_f32(f32 min, f32 max)
{
    return min + ((f32)rand() / RAND_MAX) * (max-min);
}

internal i32
random_int(i32 min, i32 max)
{
    return min + rand() % (max - min + 1);
}


internal u32
create_metaball(app_t *app, u32 mesh_id, u32 mat_id)
{
    f32 w_to_s = app->rb.cams[0].world_to_screen_or;
    
    u32 width = app->rb.width;
    u32 height = app->rb.height;
    
    f32 scale = random_f32(1.5f, 2.5f);
    f32 screen_scale = scale * w_to_s;
    
    transform_info_t t_info = {0};
    v3 screen_pos = HMM_V3(random_f32(-screen_scale, width+screen_scale), 
                           random_f32(-screen_scale, height+screen_scale),
                           0.0f);
    
    t_info.pos = HMM_DivV3F(HMM_SubV3(screen_pos, 
                                      HMM_V3(width/2.0f, height/2.0f, 0.0f)),
                            w_to_s);
    t_info.rot = HMM_V3(0.0f, 0.0f, 0.0f);
    t_info.scale = HMM_V3(scale, scale, 1.0f);
    
    
    v3 v_i = HMM_V3(random_f32(-0.2f, 0.2f), 
                    random_f32(-0.2f, 0.2f), 0.0f);
    u32 p_id = add_physics(app, v_i);
    
    return add_entity(app, t_info, mesh_id, mat_id, p_id);
}

internal metaballs_t
create_metaballs(app_t *app, u32 count, u32 mesh_id, u32 mat_id)
{
    metaballs_t m = {0};
    
    m.e_ids = malloc(sizeof(*m.e_ids)*count);
    m.count = count;
    
    for (u32 i = 0; i < count; i++)
        m.e_ids[i] = create_metaball(app, mesh_id, 1);
    
    return m;
}

internal void
clamp_entity_to_screen(app_t *app, u32 id)
{
    entity_t entity = app->entities[id];
    mat4 *trans = app->rb.transforms + entity.trans_id;
    
    f32 w_to_s = app->rb.cams[0].world_to_screen_or;
    u32 w = app->rb.width, h = app->rb.height;
    
    f32 scale = HMM_LenV3(HMM_V3(trans->Elements[0][0],
                                 trans->Elements[0][1],
                                 trans->Elements[0][2]));
    v3 pos = HMM_V3(trans->Elements[3][0],
                    trans->Elements[3][1],
                    0);
    
    v3 dc = HMM_V3(w/2.0f, h/2.0f, 0.0f);
    f32 screen_scale = scale * w_to_s;
    v3 screen_pos = HMM_AddV3(HMM_MulV3F(pos, w_to_s), dc);
    
    u32 unchanged = 0;
    if (screen_pos.X + screen_scale < 0)
        screen_pos.X = w-1 + screen_scale;
    else if (screen_pos.X - screen_scale > w-1)
        screen_pos.X = -screen_scale;
    else
        unchanged += 1;
    
    if (screen_pos.Y + screen_scale < 0)
        screen_pos.Y = h-1 + screen_scale;
    else if (screen_pos.Y - screen_scale > h-1)
        screen_pos.Y = -screen_scale;
    else
        unchanged += 2;
    
    if (unchanged == 3)
        return;
    
    pos = HMM_DivV3F(HMM_SubV3(screen_pos, dc), w_to_s);
    
    trans->Elements[3][0] = pos.X;
    trans->Elements[3][1] = pos.Y;
}*/

internal f32
random_f32(f32 min, f32 max)
{
    return min + ((f32)rand() / RAND_MAX) * (max-min);
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
    
    grid->res = res;
    
    f32 aspect = grid->aspect;
    
    f32 thickness = grid->thickness;
    u32 x_res = res, y_res = (u32)((f32)res * aspect);
    
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

internal void
update_metaballs(metaballs_t *metaballs)
{
    v2 pos;
    f32 scale;
    mat4 *trans = metaballs->trans;
    for (u32 i = 0; i < metaballs->count; i++)
    {
        pos = metaballs->pos[i];
        scale = metaballs->rad[i];
        
        update_transform(trans,
                         HMM_V3(pos.X, pos.Y, 0.0f),
                         HMM_V3(0.0f, 0.0f, 0.0f),
                         HMM_V3(scale, scale, 1.0f));
        
        trans++;
    }
}

internal metaballs_t
create_metaballs(renderer_t *rb, v2 *pos, f32 *rad, u32 count, u32 max,
                 v3 color, f32 width, f32 height)
{
    metaballs_t metaballs = {0};
    
    metaballs.pos = (v2 *)malloc(sizeof(v2)*max);
    metaballs.rad = (f32 *)malloc(sizeof(f32)*max);
    metaballs.vel = (v2 *)malloc(sizeof(v2)*max);
    metaballs.count = count;
    metaballs.max = max;
    metaballs.color = color;
    
    metaballs.top_left = HMM_V2(-width/2, height/2);
    metaballs.btm_right = HMM_V2(width/2, -height/2);
    
    metaballs.mesh_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    memcpy(metaballs.pos, pos, sizeof(v2)*count);
    memcpy(metaballs.rad, rad, sizeof(f32)*count);
    random_v2_array(metaballs.vel, count, -0.8, 0.8);
    
    metaballs.trans = (mat4 *)malloc(sizeof(mat4)*max);
    
    update_metaballs(&metaballs);
    
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
    
    update_metaballs(metaballs);
}

internal void
render_screen_grid(renderer_t *rb, screen_grid_t grid)
{
    f32 aspect = grid.height/(f32)grid.width;
    u32 line_count = grid.res-1 + (u32)(grid.res*aspect)-1;
    
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
    circle_tex_shader_id = add_shader(am, "circle.wgsl", COMPUTE_SHADER);
    circle_shader_id = add_shader(am, "render_circle.wgsl", VERTEX_FRAGMENT_SHADER);
    
    font_asset_id = add_font(am, "fonts\\hack\\Hack-Regular.ttf", 1024*5, 1024*5, ' ', '~'-' ', 64*5);
}

global u32 q_id;
global u32 t_id;

INIT_APP(init_app)
{
    srand((u32)time(NULL));
    
    renderer_t *rb = &app->rb;
    asset_manager_t *am = &app->am;
    
    cam = add_camera(rb, HMM_V3(0.0f, 0.0f, 10.0f), 20.0f, 45.0f, 1.0f, 100.0f);
    
    font_id = process_font_asset(rb, am, font_asset_id);
    
    v2 pos[10];
    f32 rad[10];
    random_v2_array(pos, 10, -12, 10);
    random_f32_array(rad, 10, 1.0, 2.0);
    
    metaballs = create_metaballs(rb, pos, rad, 10, 20, HMM_V3(1.0f, 0.4, 0.0f),
                                 20.0f * rb->width/(f32)rb->height, 20.0f);
    
    grid = create_screen_grid(rb, rb->width, rb->height, 16, 80, 1.0f);
    
    update_screen_grid(&grid, 32);
    
    //text_id = add_text(app, font_id, strlen("Hello World"), HMM_V3(0.0f, 0.0f, 0.0f), 1.0f/800.0f);
    
    
    /*u32 w = grid.res, h = grid.res*grid.aspect;
    u8 *tex_data = (u8 *)malloc(w*h*4);
    memset(tex_data, 0, w*h*4);
    u32 grid_tex = add_texture(rb, tex_data, w, h, TEXTURE_FORMAT_RGBA8U_NORM, 
                               TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_STORAGE_BINDING);*/
    
    
    u32 ct_w = 256, ct_h = 256;
    c_tex_data = (u8 *)malloc(ct_w*ct_h*4);
    memset(c_tex_data, 0, ct_w*ct_h*4);
    u32 circle_tex = add_texture(rb, c_tex_data, ct_w, ct_h, TEXTURE_FORMAT_RGBA8U_NORM,
                                 TEXTURE_USAGE_COPY_SRC | TEXTURE_USAGE_STORAGE_BINDING |
                                 TEXTURE_USAGE_COPY_DST | TEXTURE_USAGE_TEXTURE_BINDING);
    
    
    u32 vb_id = add_vertex_buffer(rb, rb->verts, get_stack_count(rb->verts),
                                  ATTRIBUTE_VERTEX, 0, MODE_VERTEX);
    
    
    u32 ub_ids[2];
    bind_layout_t c_layout[6];
    bind_layout_t ct_layout[1];
    bind_layout_t g_layout[1];
    
    u32 sampler_id = add_sampler(rb,  ADDRESS_MODE_CLAMPTOEDGE, 
                                 ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                                 FILTER_LINEAR, FILTER_LINEAR);
    
    ub_ids[0] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_UNIFORM);
    ub_ids[1] = add_buffer(rb, BUFFER_FLAG_COPY_DST | BUFFER_FLAG_STORAGE);
    
    u32 ct_ids[1], g_ids[2];
    c_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    c_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    ct_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    g_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    g_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    ct_layout[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, circle_tex, TEXTURE_ACCESS_WRITEONLY);
    
    add_bind_layouts(rb, ct_ids[0], ct_layout, 1);
    
    
    c_layout[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, circle_tex);
    c_layout[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id, SAMPLER_TYPE_FILTERING);
    c_layout[2] = get_buffer_bind_layout_for_struct(&cam->orth, 0, SHADER_VISIBILITY_VERTEX, 
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    c_layout[3] = get_buffer_bind_layout_for_struct(&cam->view, 1, SHADER_VISIBILITY_VERTEX,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    c_layout[4] = get_buffer_bind_layout_for_struct(&metaballs.color, 2, SHADER_VISIBILITY_FRAGMENT,
                                                    ub_ids[0], BUFFER_TYPE_UNIFORM);
    c_layout[5] = get_buffer_bind_layout_for_array(metaballs.trans, metaballs.count, 0, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    
    add_bind_layouts(rb, c_ids[0], c_layout, 2);
    add_bind_layouts(rb, c_ids[1], c_layout+2, 3);
    add_bind_layouts(rb, c_ids[2], c_layout+5, 1);
    
    
    g_layout[0] = get_buffer_bind_layout_for_array(grid.trans, grid.trans_count, 0, SHADER_VISIBILITY_VERTEX,
                                                   ub_ids[1], BUFFER_TYPE_READ_ONLY_STORAGE);
    
    g_ids[0] = c_ids[1];
    add_bind_layouts(rb, g_ids[1], g_layout, 1);
    
    
    c_id[0] = add_compute_pipeline(rb, circle_tex_shader_id, ct_ids, 1, ct_w/16.0f, ct_h/16.0f, 1);
    
    blend_comp_t blend = (blend_comp_t){ BLEND_FACTOR_SRCALPHA, BLEND_FACTOR_ONEMINUSSRCALPHA, BLEND_OP_ADD };
    
    p_id[0] = add_render_pipeline(rb, circle_shader_id, 1, HMM_V3(0.0f, 0.0f, 0.5f),
                                  blend, blend, &vb_id, 1, c_ids, 3);
    
    p_id[1] = add_render_pipeline(rb, grid_shader_id, 1, HMM_V3(0.0f, 0.0f, 0.0f),
                                  blend, blend, &vb_id, 1, g_ids, 2);
    
}

UPDATE_AND_RENDER(update_and_render)
{
    renderer_t *rb = &app->rb;
    
    update_bind_group_layout(rb, c_ids[2], 0);
    
    update_metaballs_physics(&metaballs, 0.02);
    
    submit_compute_pipeline(rb, c_id[0]);
    
    start_render_pipeline(rb, p_id[0]);
    {
        clear_color(rb, HMM_V3(0.0f, 0.2f, 0.5f));
        
        render_metaball_circles(rb, metaballs);
    }
    end_render_pipeline(rb);
    
    
    start_render_pipeline(rb, p_id[1]);
    {
        render_screen_grid(rb, grid);
    }
    end_render_pipeline(rb);
    
}