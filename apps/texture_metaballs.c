
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


typedef struct metaballs_t {
    u32 *e_ids;
    u32 count;
    
    v3 color;
} metaballs_t;

metaballs_t metaballs;

u8 *tex_data, *g_tex_data, *fb_data, *prev_fb_data;
u32 width = 256, height = 256;

u32 df_tex_id, gdf_tex_id, fb_id, prev_fb_id, sampler_id;
camera_t *cam;
global f32 speed = 0.01f;
global u32 c_ids[3];
global u32 p_id[2];
global u32 q_id;
global u32 e_id[3];

global u32 s_df_id;
global u32 s_g_id;
global u32 s_c_id;
global u32 s_id;
global u32 s_f_id;

global u32 bg_ids[7];



internal void
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
    
    update_bind_group_layout(&app->rb, bg_ids[5], 0);
}

internal void
create_textures(renderer_t *rb)
{
    // NOTE(ajeej): Add Textures
    tex_data = (u8 *)malloc(width*height*4);
    g_tex_data = (u8 *)malloc(width*height*4);
    fb_data = (u8 *)malloc(rb->width*rb->height*4);
    
    memset(tex_data, 255, width*height*4);
    memset(g_tex_data, 255, width*height*4);
    memset(fb_data, 0, rb->width*rb->height*4);
    
    
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
    
    sampler_id = add_sampler(rb, ADDRESS_MODE_CLAMPTOEDGE, 
                             ADDRESS_MODE_CLAMPTOEDGE, ADDRESS_MODE_CLAMPTOEDGE,
                             FILTER_LINEAR, FILTER_LINEAR);
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
    bind_layout_t meta_layouts[6];
    bind_layout_t filter_layouts[2];
    
    // NOTE(ajeej): Specify bind group layouts
    u32 bg_layout_counts[8] = {
        1, 2, 1, 2, 3, 1, 2
    };
    
    bind_layout_t *bg_layout_table[8] = {
        meta_df_layouts,
        g_blur_layouts,
        clear_layouts,
        meta_layouts,
        meta_layouts+2,
        meta_layouts+5,
        filter_layouts
    };
    
    // NOTE(ajeej): Add bind groups
    bg_ids[0] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[1] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[2] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    bg_ids[3] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    bg_ids[4] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    bg_ids[5] = add_bind_group(rb, BIND_GROUP_TYPE_DRAW);
    
    bg_ids[6] = add_bind_group(rb, BIND_GROUP_TYPE_FRAME);
    
    // NOTE(ajeej): Get bind group layouts
    meta_df_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, 
                                                         df_tex_id, TEXTURE_ACCESS_WRITEONLY);
    
    g_blur_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE, df_tex_id);
    
    g_blur_layouts[1] = get_storage_texture_bind_layout(1, SHADER_VISIBILITY_COMPUTE,
                                                        gdf_tex_id, TEXTURE_ACCESS_WRITEONLY);
    
    clear_layouts[0] = get_storage_texture_bind_layout(0, SHADER_VISIBILITY_COMPUTE,
                                                       fb_id, TEXTURE_ACCESS_WRITEONLY);
    
    
    meta_layouts[0] = get_texture_bind_layout(0, SHADER_VISIBILITY_FRAGMENT, gdf_tex_id);
    
    meta_layouts[1] = get_sampler_bind_layout(1, SHADER_VISIBILITY_FRAGMENT, sampler_id,
                                              SAMPLER_TYPE_FILTERING);
    
    meta_layouts[2] = get_buffer_bind_layout_for_struct(&cam->orth, 0, SHADER_VISIBILITY_VERTEX, 
                                                        ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    meta_layouts[3] = get_buffer_bind_layout_for_struct(&cam->view, 1, SHADER_VISIBILITY_VERTEX, 
                                                        ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    meta_layouts[4] = get_buffer_bind_layout_for_struct(&metaballs.color, 2, 
                                                        SHADER_VISIBILITY_FRAGMENT, 
                                                        ub_ids[0], BUFFER_TYPE_UNIFORM);
    
    meta_layouts[5] = get_dynamic_buffer_bind_layout_for_struct(rb->transforms, metaballs.count, 0,
                                                                SHADER_VISIBILITY_VERTEX, ub_ids[1], BUFFER_TYPE_UNIFORM,
                                                                offsetof(render_cmd_t, trans_id));
    
    
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
    return min + ((f32)rand() / (f32)RAND_MAX) * (max-min);
}
internal i32
random_int(i32 min, i32 max)
{
    return min + rand() % (max - min + 1);
}
internal u32
create_metaball(app_t *app, u32 mesh_id)
{
    f32 w_to_s = app->rb.cams[0].world_to_screen_or;
    
    u32 width = app->rb.width;
    u32 height = app->rb.height;
    
    f32 scale = random_f32(0.5f, 2.0f);
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
    
    return add_entity(app, t_info, mesh_id, 0, p_id);
}
internal metaballs_t
create_metaballs(app_t *app, u32 count, u32 mesh_id, v3 color)
{
    metaballs_t m = {0};
    
    m.e_ids = malloc(sizeof(*m.e_ids)*count);
    m.count = count;
    m.color = color;
    
    for (u32 i = 0; i < count; i++)
        m.e_ids[i] = create_metaball(app, mesh_id);
    
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
    f32 screen_scale = scale * w_to_s * 2.0f/3.0f;
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
}


LOAD_ASSETS(load_assets)
{
    s_df_id = add_shader(am, "metaball_distance_field.wgsl", COMPUTE_SHADER);
    s_g_id = add_shader(am, "gaussian_blur.wgsl", COMPUTE_SHADER);
    s_c_id = add_shader(am, "clear.wgsl", COMPUTE_SHADER);
    s_id = add_shader(am, "metaballs.wgsl", VERTEX_FRAGMENT_SHADER);
    s_f_id = add_shader(am, "metaball_filter.wgsl", VERTEX_FRAGMENT_SHADER);
}


INIT_APP(init_app)
{
    srand((u32)time(NULL));
    
    renderer_t *rb = &app->rb;
    asset_manager_t *am = &app->am;
    
    cam = add_camera(rb, HMM_V3(0.0f, 0.0f, 10.0f), 5.0f, 45.0f, 1.0f, 100.0f);
    
    // NOTE(ajeej): Create meshes
    q_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    //create_metaball(app, q_id);
    
    metaballs = create_metaballs(app, 50, q_id, HMM_V3(1.0, 0.4, 0.0));
    
    create_textures(rb);
    
    u32 vb_id = add_vertex_buffer(rb, rb->verts, get_stack_count(rb->verts),
                                  ATTRIBUTE_VERTEX, 0, MODE_VERTEX);
    
    create_uniforms(rb);
    
    blend_comp_t blends[3] = {
        { BLEND_FACTOR_ONE, BLEND_FACTOR_ONE, BLEND_OP_ADD },
        { BLEND_FACTOR_SRCALPHA, BLEND_FACTOR_ONEMINUSSRCALPHA, BLEND_OP_ADD },
        { BLEND_FACTOR_ONE, BLEND_FACTOR_ZERO, BLEND_OP_ADD },
    };
    
    // NOTE(ajeej): Add Pipelines
    c_ids[0] = add_compute_pipeline(rb, s_df_id, bg_ids, 1, 32, 32, 1);
    c_ids[1] = add_compute_pipeline(rb, s_g_id, bg_ids+1, 1, 32, 32, 1);
    c_ids[2] = add_compute_pipeline(rb, s_c_id, bg_ids+2, 1, (rb->width+15)/16, (rb->height+15)/16, 1);
    
    p_id[0] = add_render_pipeline(rb, s_id, fb_id, 
                                  HMM_V3(0.0, 0.0, 0.0), 
                                  blends[2], blends[0],
                                  &vb_id, 1, bg_ids+3, 3);
    p_id[1] = add_render_pipeline(rb, s_f_id, 1, 
                                  HMM_V3(0.0, 0.2, 0.5),
                                  blends[1], blends[1],
                                  &vb_id, 1, bg_ids+6, 1);
    
}

UPDATE_AND_RENDER(update_and_render)
{
    renderer_t *rb = &app->rb;
    
    process_input(app);
    
    for (u32 i = 0; i < metaballs.count; i++)
        update_entity_physics(app, metaballs.e_ids[i], 0.02);
    
    for (u32 i = 0; i < metaballs.count; i++)
        clamp_entity_to_screen(app, metaballs.e_ids[i]);
    
    submit_compute_pipeline(rb, c_ids[0]);
    submit_compute_pipeline(rb, c_ids[1]);
    submit_compute_pipeline(rb, c_ids[2]);
    
    start_render_pipeline(rb, p_id[0]);
    {
        for (u32 i = 0; i < metaballs.count; i++)
            render_entity(app, metaballs.e_ids[i]);
    }
    end_render_pipeline(rb);
    
    /*copy_texture(rb, fb_id, 0, 0, prev_fb_id, 0, 0, rb->width, rb->height);
    
    start_render_pipeline(rb, p_id[0]);
    {
        for (u32 i = 5; i < 10; i++)
            render_entity(app, metaballs.e_ids[i]);
    }
    end_render_pipeline(rb);
    
    copy_texture(rb, fb_id, 0, 0, prev_fb_id, 0, 0, rb->width, rb->height);*/
    
    start_render_pipeline(rb, p_id[1]);
    {
        push_render_cmd(&app->rb, q_id, 0, 0, 1);
    }
    end_render_pipeline(rb);
    
}