
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

global u32 shader_id;
global u32 inst_shader_id;
global u32 inst_id;
global u32 mats_id;
global u32 trans_id;

global u32 cube_id;
global u32 entity_id;

internal void
process_input(app_t *app)
{
    camera_t *cam = &app->rb.cam;
    
    if (is_key_down(app, KEY_W))
        move_camera(cam, cam->front);
    if (is_key_down(app, KEY_S))
        move_camera(cam, HMM_MulV3F(cam->front, -1.0f));
    if (is_key_down(app, KEY_A))
        move_camera(cam, HMM_MulV3F(cam->right, -1.0f));
    if (is_key_down(app, KEY_D))
        move_camera(cam, cam->right);
    if (is_key_down(app, KEY_Q))
        move_camera(cam, cam->world_up);
    if (is_key_down(app, KEY_E))
        move_camera(cam, HMM_MulV3F(cam->world_up, -1.0f));
    
    if (is_key_down(app, KEY_U))
        rotate_camera(cam, cam->right, 1.0f);
    if (is_key_down(app, KEY_J))
        rotate_camera(cam, cam->right, -1.0f);
    if (is_key_down(app, KEY_H))
        rotate_camera(cam, cam->up, 1.0f);
    if (is_key_down(app, KEY_K))
        rotate_camera(cam, cam->up, -1.0f);
    
    
    update_camera(cam, CAMERA_UPDATE_VIEW | CAMERA_UPDATE_ORIENTATION);
}

internal u32
create_materials_from_colors(renderer_t *rb, u32 count, v3 *colors)
{
    u32 id = get_stack_count(rb->mats);
    
    v3 color;
    for (u32 i = 0; i < count; i++) {
        color = colors[i];
        create_material(rb, color, color, color, 10.0f);
    }
    
    return id;
}

internal void
get_transforms_from_pos(transform_info_t *info, u32 count, v3 *pos)
{
    for (u32 i = 0; i < count; i++)
    {
        info[i].pos = pos[i];
        info[i].rot = HMM_V3(0.0f, 0.0f, 0.0f);
        info[i].scale = HMM_V3(1.0f, 1.0f, 1.0f);
    }
}

INIT_APP(init_app)
{
    shader_id = add_shader(&app->rb, &app->am, 2,
                           VERTEX_SHADER, "vert.glsl",
                           FRAGMENT_SHADER, "frag.glsl");
    
    inst_shader_id = add_shader(&app->rb, &app->am, 2,
                                VERTEX_SHADER, "inst_vert.glsl",
                                FRAGMENT_SHADER, "inst_frag.glsl");
    
    mats_id = create_material(&app->rb, HMM_V3(1.0f, 0.0f, 0.0f), HMM_V3(1.0f, 0.0f, 0.0f), 
                              HMM_V3(1.0f, 0.0f, 0.0f), 10.0f);
    
    transform_info_t t_info = (transform_info_t){
        HMM_V3(0.0f, 0.0f, 0.0f),
        HMM_V3(0.0f, 0.0f, 0.0f),
        HMM_V3(1.0f, 1.0f, 1.0f),
    };
    cube_id = create_cube(&app->rb, make_color(255, 0, 0, 255));
    entity_id = create_entity(app, t_info, cube_id, mats_id);
    
    
    v3 colors[4] = {
        HMM_V3(1.0f, 0.0f, 0.0f),
        HMM_V3(0.0f, 1.0f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
        HMM_V3(0.0f, 0.0f, 0.0f)
    };
    mats_id = create_materials_from_colors(&app->rb, 4, colors);
    
    inst_id = create_icosahedron_instance(&app->rb,  make_color(255, 0, 0, 255),
                                          4);
    
    transform_info_t trans[4];
    v3 pos[4] = {
        HMM_V3(-8.0f, 0.0f, 0.0f),
        HMM_V3(-3.0f, 0.0f, 0.0f),
        HMM_V3(3.0f, 0.0f, 0.0f),
        HMM_V3(8.0f, 0.0f, 0.0f)
    };
    get_transforms_from_pos(trans, 4, pos);
    trans_id = create_transforms(&app->rb, trans, 4);
    
    u32 trans_info_count = 0, mat_info_count = 0;
    attrib_info_t *trans_attrib_info = get_attrib_info_by_type(ATTRIB_TYPE_MAT4, 
                                                               &trans_info_count);
    attrib_info_t *mat_attrib_info = get_attrib_info_by_type(ATTRIB_TYPE_MATERIAL,
                                                             &mat_info_count);
    
    add_attribute(&app->rb, inst_shader_id, inst_id,
                  app->rb.transforms+trans_id,
                  sizeof(mat4), 4, 1,
                  trans_attrib_info, trans_info_count, "inst_model");
    
    add_attribute(&app->rb, inst_shader_id, inst_id,
                  app->rb.mats+mats_id,
                  sizeof(material_t), 4, 1,
                  mat_attrib_info, mat_info_count, NULL);
    
    free(trans_attrib_info);
    free(mat_attrib_info);
    
    
    init_dir_light(&app->rb.dir_light, HMM_V3(-0.2f, 1.0f, -0.3f),
                   HMM_V3(0.4f, 0.4f, 0.4f), 
                   HMM_V3(0.5f, 0.5f, 0.5f),
                   HMM_V3(0.1f, 0.1f, 0.1f));
}

UPDATE_AND_RENDER(update_and_render)
{
    process_input(app);
    
    use_shader(&app->rb, shader_id);
    add_entity(app, entity_id);
    
    use_shader(&app->rb, inst_shader_id);
    add_instance(&app->rb, inst_id);
}