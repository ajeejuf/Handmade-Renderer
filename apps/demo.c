
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

#define PLANETARY_G 1

typedef struct planet_params_t {
    f32 semi_major;
    f32 eccentricity;
    f32 inclination;
    f32 ascending_longitude;
    f32 periapsis;
    f32 true_anomaly;
} planet_params_t;

typedef struct planet_t {
    v3 v, a;
    
    planet_params_t params;
} planet_t;

global u32 sun_shader_id;
global u32 planet_shader_id;
global u32 star_shader_id;

global u32 entity_id;

global planet_t planets[500];
global u32 planet_ids[2];
global u32 trans_attrib_ids[2];
global u32 planet_trans_id;

global u32 planet_shape_count = ARRAY_COUNT(planet_ids);
global u32 planet_count = ARRAY_COUNT(planets);

global u32 star_id;

global f32 sun_mass = 100;

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

internal f32 random_float(f32 min, f32 max)
{
    return min + ((f32)rand() / (f32)RAND_MAX) * (max - min);
}


internal void
init_rand_planet_params(planet_params_t *params, f32 min_a, f32 max_a, f32 min_e, f32 max_e,
                        f32 min_i, f32 max_i, f32 min_omega, f32 max_omega, f32 min_w, f32 max_w,
                        f32 min_v, f32 max_v)
{
    params->semi_major = random_float(min_a, max_a);
    params->eccentricity = random_float(min_e, max_e);
    params->inclination = random_float(min_i, max_i);
    params->ascending_longitude = HMM_ToRad(random_float(min_omega, max_omega));
    params->periapsis = HMM_ToRad(random_float(min_w, max_w));
    params->true_anomaly = HMM_ToRad(random_float(min_v, max_v));
}

internal v3
get_planet_acceleration(f32 M, v3 r)
{
    v3 res;
    f32 len = HMM_LenV3(r);
    
    if (len <= 0)
        return HMM_V3(0.0f, 0.0f, 0.0f);
    
    f64 scale = -PLANETARY_G * M / (len*len*len);
    res = HMM_MulV3F(r, scale);
    
    return res;
}

internal v3
rotate3(v3 v, f32 theta)
{
    v3 res;
    res.X = HMM_CosF(theta) * v.X - HMM_SinF(theta) * v.Z;
    res.Z = HMM_SinF(theta) * v.X + HMM_CosF(theta) * v.Z;
    res.Y = v.Y;
    
    return res;
}

internal v3
rotate1(v3 v, f32 theta)
{
    v3 res;
    res.X = v.X;
    res.Z = HMM_CosF(theta) * v.Z - HMM_SinF(theta) * v.Y;
    res.Y = HMM_SinF(theta) * v.Z + HMM_CosF(theta) * v.Y;
    
    return res;
}

internal void
init_planet(v3 center, f32 M, planet_t *planet, v3 *pos)
{
    planet_params_t *params = &planet->params;
    v3 *vel = &planet->v, *accel = &planet->a;
    
    f64 a = params->semi_major;
    f64 e = params->eccentricity;
    f64 i = params->inclination;
    f64 omega = params->ascending_longitude;
    f64 w = params->periapsis;
    f64 v = params->true_anomaly;
    
    f64 r = a * (1 - e*e) / (1 + e * HMM_CosF(v));
    f64 v_r = HMM_SqrtF(M / a) * e * HMM_SinF(v) / HMM_SqrtF(1 - e*e);
    f64 v_theta = HMM_SqrtF(M / a) * (1 + e * HMM_CosF(v)) / HMM_SqrtF(1 - e*e);
    
    v3 r_orb = HMM_V3(r * HMM_CosF(v), 0, r * HMM_SinF(v));
    v3 v_orb = HMM_V3(v_r * HMM_CosF(v) - v_theta * HMM_SinF(v), 
                      0, v_r * HMM_SinF(v) + v_theta * HMM_CosF(v));
    
    r_orb = rotate3(r_orb, w);
    r_orb = rotate1(r_orb, i);
    r_orb = rotate3(r_orb, omega);
    
    v_orb = rotate3(v_orb, w);
    v_orb = rotate1(v_orb, i);
    v_orb = rotate3(v_orb, omega);
    
    *pos = r_orb;
    *vel = v_orb;
    *accel = get_planet_acceleration(sun_mass, r_orb);
}

internal void
update_planet_pos(planet_t *planet, f32 M, f64 dt, mat4 *trans)
{
    v3 nv = HMM_AddV3(planet->v, HMM_MulV3F(planet->a, dt));
    
    v3 pos = HMM_V3(trans->Elements[3][0], trans->Elements[3][1], trans->Elements[3][2]);
    
    v3 np = HMM_AddV3(pos, HMM_MulV3F(planet->v, dt));
    
    trans->Elements[3][0] = np.X;
    trans->Elements[3][1] = np.Y;
    trans->Elements[3][2] = np.Z;
    
    planet->v = nv;
    planet->a = get_planet_acceleration(M, np);
}

internal u32
create_sun(app_t *app, v3 pos, f32 r)
{
    u32 sphere_id = create_sphere(&app->rb, 30, 30, make_color(255,0 ,255, 255));
    v3 color = color_to_v3(255, 255, 255);
    u32 mat_id = create_material(&app->rb, color, color, color, 10.0f);
    
    transform_info_t sun_trans = (transform_info_t) {
        pos,
        HMM_V3(0.0f, 0.0f, 0.0f),
        HMM_V3(r, r, r),
    };
    
    u32 id = create_entity(app, sun_trans, sphere_id, mat_id);
    
    return id;
}

internal void
create_planets(app_t *app, planet_t *p, u32 count, v3 sun_pos, f32 sun_mass,
               u32 *out_trans_id)
{
    transform_info_t planet_trans[ARRAY_COUNT(planets)];
    
    for (u32 i = 0; i < count; i++)
    {
        init_rand_planet_params(&p[i].params, 140, 150, 0.0f, 0.1f, 0.0f, 0.12f,
                                0.0f, 360.0f, 0.0f, 360.0f, 0.0f, 360.0f);
        init_planet(sun_pos, sun_mass, p+i, &planet_trans[i].pos);
        planet_trans[i].rot = HMM_V3(random_float(0.0f, 360.0f), random_float(0.0f, 360.0f), random_float(0.0f, 360.0f));
        planet_trans[i].scale = HMM_V3(1.0f, 1.0f, 1.0f);
    }
    
    *out_trans_id = create_transforms(&app->rb, planet_trans, planet_count);
}

internal u32
create_random_materials(renderer_t *rb, u32 count)
{
    u32 id = get_stack_count(rb->mats);
    
    v3 color;
    for (u32 i = 0; i < count; i++) {
        color = HMM_V3(random_float(0.0f, 1.0f), random_float(0.0f, 1.0f), random_float(0.0f, 1.0f));
        create_material(rb, color, color, color, random_float(10.0f, 32.0f));
    }
    
    return id;
}

internal v3
get_random_point_on_sphere(f32 r)
{
    f32 theta = random_float(0.0f, 2.0f*HMM_PI);
    f32 phi = acosf(2.0f * (f32)rand() / (f32)RAND_MAX - 1.0f);
    
    f32 x = r * HMM_SinF(phi) * HMM_CosF(theta);
    f32 z = r * HMM_SinF(phi) * HMM_SinF(theta);
    f32 y = r * HMM_CosF(phi);
    
    return HMM_V3(x, y, z);
}

internal void
get_random_star_transforms(transform_info_t *info, u32 count, u32 r)
{
    for (u32 i = 0; i < count; i++)
    {
        info->pos = get_random_point_on_sphere(r);
        info->rot = HMM_V3(0.0f, 0.0f, 0.0f);
        info->scale = HMM_V3(1.0f, 1.0f, 1.0f);
    }
}

INIT_APP(init_app)
{
    srand(time(NULL));
    
    set_camera_pos(&app->rb.cam, HMM_V3(0.0f, 0.0f, 150.0f));
    
    sun_shader_id = add_shader(&app->rb, &app->am, 2,
                               VERTEX_SHADER, "sun_vert.glsl",
                               FRAGMENT_SHADER, "sun_frag.glsl");
    
    planet_shader_id = add_shader(&app->rb, &app->am, 2,
                                  VERTEX_SHADER, "planet_vert.glsl",
                                  FRAGMENT_SHADER, "planet_frag.glsl");
    
    /*star_shader_id = add_shader(&app->rb, &app->am, 2,
                                VERTEX_SHADER, "star_vert.glsl",
                                FRAGMENT_SHADER, "star_frag.glsl");*/
    
    f32 sun_radius = 50.0f;
    v3 sun_pos = HMM_V3(0.0f, 0.0f, 0.0f);
    
    entity_id = create_sun(app, sun_pos, sun_radius);
    
    create_planets(app, planets, planet_count, sun_pos, sun_mass, &planet_trans_id);
    
    u32 planet_mats_id = create_random_materials(&app->rb, planet_count);
    
    u32 shape_count = planet_count/planet_shape_count;
    planet_ids[0] = create_icosahedron_instance(&app->rb, make_color(0, 0, 255, 255), shape_count);
    planet_ids[1] = create_cube_instance(&app->rb, make_color(0, 0, 255, 255), shape_count);
    
    /*transform_info_t star_trans_info[2];
    u32 star_count = ARRAY_COUNT(star_trans_info);
    get_random_star_transforms(star_trans_info, star_count, 100);
    u32 star_trans_id = create_transforms(&app->rb, star_trans_info, star_count);
    star_id = create_sphere_instance(&app->rb, 10, 10, 
                                     make_color(255, 255, 255, 255), star_count);*/
    
    //LOG("%d %d", planet_trans_id, star_trans_id);
    
    u32 trans_info_count = 0;
    u32 mat_info_count = 0;
    attrib_info_t *trans_attrib_info = get_attrib_info_by_type(ATTRIB_TYPE_MAT4, &trans_info_count);
    attrib_info_t *mat_attrib_info = get_attrib_info_by_type(ATTRIB_TYPE_MATERIAL, &mat_info_count);
    
    for (u32 i = 0; i < planet_shape_count; i++)
    {
        trans_attrib_ids[i] = add_attribute(&app->rb, planet_shader_id, planet_ids[i], 
                                            app->rb.transforms+planet_trans_id + i*planet_count/planet_shape_count,
                                            sizeof(mat4), planet_count, 1, 
                                            trans_attrib_info, trans_info_count, "inst_model");
        
        add_attribute(&app->rb, planet_shader_id, planet_ids[i], 
                      app->rb.mats+planet_mats_id + i*planet_count/planet_shape_count,
                      sizeof(material_t), planet_count, 1, 
                      mat_attrib_info, mat_info_count, NULL);
    }
    
    /*add_attribute(&app->rb, star_shader_id, star_id,
                  app->rb.transforms+star_trans_id, sizeof(mat4), 0,
                  trans_attrib_info, trans_info_count, "inst_model");*/
    
    free(trans_attrib_info);
    free(mat_attrib_info);
    
    
    init_dir_light(&app->rb.dir_light, HMM_V3(-0.2f, 1.0f, -0.3f),
                   HMM_V3(0.0f, 0.0f, 0.0f), 
                   HMM_V3(0.0f, 0.0f, 0.0f),
                   HMM_V3(0.0f, 0.0f, 0.0f));
    
    add_point_light(&app->rb, sun_pos, 1.0f, 0.0001f, 0.000001f, HMM_V3(0.45f, 0.45f, 0.45f), 
                    HMM_V3(0.8f, 0.8f, 0.8f), HMM_V3(1.0f, 1.0f, 1.0f));
}

UPDATE_AND_RENDER(update_and_render)
{
    process_input(app);
    
    u32 i;
    
    for (i = 0; i < planet_count; i++)
    {
        update_planet_pos(planets+i, sun_mass, 0.01, 
                          app->rb.transforms+planet_trans_id+i);
    }
    
    for (i = 0; i < planet_shape_count; i++)
    {
        flag_attribute(&app->rb, planet_ids[i], trans_attrib_ids[i]);
    }
    
    use_shader(&app->rb, sun_shader_id);
    
    add_entity(app, entity_id);
    
    
    /*use_shader(&app->rb, star_shader_id);
    
    add_instance(&app->rb, star_id);*/
    
    
    use_shader(&app->rb, planet_shader_id);
    
    for (i = 0; i < ARRAY_COUNT(planet_ids); i++)
        add_instance(&app->rb, planet_ids[i]);
}