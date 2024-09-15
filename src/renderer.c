
internal color_t
make_color(f32 r, f32 g, f32 b, f32 a) 
{
    return (color_t) {
        r, g, b, a
    };
}

#define init_camera_default(c, p, sp, sn, as) \
init_camera(c, p, sp, sn, as, 0.5f, 45.0f, 0.1f, 10000.0f);

internal void
update_camera(camera_t *cam, u32 update_flag)
{
    if (update_flag & CAMERA_UPDATE_PROJECTION)
    {
        cam->pers = HMM_Perspective_RH_NO(HMM_ToRad(cam->fov), cam->aspect_ratio,
                                          cam->n, cam->f);
        
        f32 interp_near = cam->n+(cam->f-cam->n)*cam->orth_interp;
        f32 h = 2.0f*tan(cam->fov/2.0f) * interp_near;
        f32 w = h * cam->aspect_ratio;
        f32 l = -w/2.0f, b = -h/2.0f;
        cam->orth = HMM_Orthographic_RH_NO(l, -l, b, -b, cam->n, cam->f);
    }
    
    if (update_flag & CAMERA_UPDATE_ORIENTATION)
    {
        v3 front = HMM_RotateV3Q(HMM_V3(0.0f, 0.0f, -1.0f), cam->rot);
        
        cam->front = HMM_NormV3(front);
        cam->right = HMM_NormV3(HMM_Cross(cam->front, cam->world_up));
        cam->up = HMM_NormV3(HMM_Cross(cam->right, cam->front));
    }
    
    if (update_flag & CAMERA_UPDATE_VIEW)
    {
        cam->view = HMM_LookAt_RH(cam->pos, HMM_AddV3(cam->pos, cam->front), cam->up);
    }
}

internal void
init_camera(camera_t *cam, v3 pos, f32 speed, f32 sens,
            f32 aspect_ratio, f32 orth_interp,
            f32 fov, f32 near_plane, f32 far_plane)
{
    cam->pos = pos;
    cam->world_up = HMM_V3(0.0f, 1.0f, 0.0f);
    
    {
        quat x = HMM_QFromAxisAngle_RH(HMM_V3(1.0f, 0.0f, 0.0f), HMM_ToRad(0.0f));
        quat y = HMM_QFromAxisAngle_RH(HMM_V3(0.0f, 1.0f, 0.0f), HMM_ToRad(0.0f));
        quat z = HMM_QFromAxisAngle_RH(HMM_V3(0.0f, 0.0f, 1.0f), HMM_ToRad(0.0f));
        cam->rot = HMM_MulQ(z, HMM_MulQ(y, x));
        
    }
    cam->speed = speed;
    cam->sens = sens;
    cam->orth_interp = orth_interp;
    cam->fov = fov;
    cam->aspect_ratio = aspect_ratio;
    cam->n = near_plane;
    cam->f = far_plane;
    
    update_camera(cam, CAMERA_UPDATE_ALL);
}

internal void
move_camera(camera_t *cam, v3 axis)
{
    cam->pos = HMM_AddV3(cam->pos, HMM_MulV3F(axis, cam->speed));
}

internal void
rotate_camera(camera_t *cam, v3 axis, f32 dir)
{
    quat dr = HMM_QFromAxisAngle_RH(axis, dir*cam->sens);
    cam->rot = HMM_MulQ(dr, cam->rot);
}

internal void
set_camera_pos(camera_t *cam, v3 pos)
{
    cam->pos = pos;
}

internal u32
push_mesh_info(renderer_t *rb, vertex_array_t *va, 
               u32 idx_count, u32 prim_type)
{
    u32 id = get_stack_count(rb->meshes);
    
    mesh_info_t *info = stack_push(&rb->meshes);
    info->indices_idx = get_stack_count(va->indices);
    info->indices_count = idx_count;
    info->prim_type = prim_type;
    
    return id;
}

internal void
push_render_cmd(renderer_t *rb, 
                u32 mesh_id, u32 mat_id, u32 trans_id)
{
    rb->shaders[rb->cur_shader].cmds.count++;
    
    render_cmd_t *cmd = stack_push(&rb->cmds);
    
    cmd->mesh_id = mesh_id;
    cmd->mat_id = mat_id;
    cmd->trans_id = trans_id;
}

internal void
init_dir_light(dir_light_t *light, v3 dir,
               v3 ambient, v3 diffuse, v3 specular)
{
    light->dir = dir;
    light->ambient = ambient;
    light->diffuse = diffuse;
    light->specular = specular;
}

internal void
add_point_light(renderer_t *rb, v3 pos, f32 constant, f32 linear, f32 quadratic,
                v3 ambient, v3 diffuse, v3 specular)
{
    if (rb->point_light_count == POINT_LIGHT_COUNT)
        return;
    
    point_light_t *light = rb->point_lights + rb->point_light_count++;
    
    light->pos = pos;
    
    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;
    
    light->ambient = ambient;
    light->diffuse = diffuse;
    light->specular = specular;
}

internal u32
create_material(renderer_t *rb, v3 ambient, v3 diffuse, v3 specular,
                f32 shininess)
{
    u32 id = get_stack_count(rb->mats);
    
    material_t *mat = stack_push(&rb->mats);
    mat->ambient = ambient;
    mat->diffuse = diffuse;
    mat->specular = specular;
    mat->shininess = shininess;
    
    return id;
}

internal u32
get_attrib_data_type_size(u32 attrib_type)
{
    switch (attrib_type)
    {
        case ATTRIB_DATA_TYPE_BYTE: { return 1; } break;
        case ATTRIB_DATA_TYPE_UNSIGNED_BYTE: { return 1; } break;
        case ATTRIB_DATA_TYPE_SHORT: { return 2; } break;
        case ATTRIB_DATA_TYPE_UNSIGNED_SHORT: { return 2; } break;
        case ATTRIB_DATA_TYPE_INT: { return 4; } break;
        case ATTRIB_DATA_TYPE_UNSIGNED_INT: { return 4; } break;
        case ATTRIB_DATA_TYPE_FLOAT: { return 4; } break;
    }
    
    return 0;
}

internal attrib_info_t
get_attrib_info(u32 count, u32 type, u32 normalize, u32 offset, const char *name)
{
    attrib_info_t info = {0};
    
    info.count = count;
    info.type = type;
    info.normalize = normalize;
    info.offset = offset;
    
    if (name)
        info.name = cstr_dup((char *)name);
    
    return info;
}

internal attrib_info_t *
get_attrib_info_by_type(u32 type, u32 *out_count)
{
    attrib_info_t *res = NULL;
    *out_count = 0;
    
    switch (type)
    {
        case ATTRIB_TYPE_MAT4: {
            res = malloc(sizeof(attrib_info_t)*4);
            for (u32 i = 0; i < 4; i++)
                res[i] = get_attrib_info(4, ATTRIB_DATA_TYPE_FLOAT, 0, sizeof(v4)*i, NULL);
            
            *out_count = 4;
        } break;
        
        case ATTRIB_TYPE_MATERIAL: {
            res = malloc(sizeof(attrib_info_t)*4);
            res[0] = get_attrib_info(3, ATTRIB_DATA_TYPE_FLOAT, 0, offsetof(material_t, ambient), "inst_mat_ambient");
            res[1] = get_attrib_info(3, ATTRIB_DATA_TYPE_FLOAT, 0, offsetof(material_t, diffuse), "inst_mat_diffuse");
            res[2] = get_attrib_info(3, ATTRIB_DATA_TYPE_FLOAT, 0, offsetof(material_t, specular), "inst_mat_specular");
            res[3] = get_attrib_info(1, ATTRIB_DATA_TYPE_FLOAT, 0, offsetof(material_t, shininess), "inst_mat_shininess");
            
            *out_count = 4;
        } break;
    }
    
    return res;
}

internal u32
add_attribute(renderer_t *rb, u32 shader_id, u32 inst_id,
              void *data, u32 size, u32 dynamic,
              attrib_info_t *infos, u32 i_count, const char *name)
{
    vertex_array_t *va = rb->va+inst_id;
    
    u32 id = get_stack_count(va->mesh_instance.attribs);
    attrib_t *attrib = stack_push(&va->mesh_instance.attribs);
    memset(attrib, 0, sizeof(*attrib));
    
    attrib->inst_id = inst_id;
    attrib->shader_id = shader_id;
    
    attrib->data = data;
    
    attrib->count = va->mesh_instance.count;
    attrib->size = size;
    attrib->dynamic = dynamic;
    attrib->update = 0;
    
    attrib->info = malloc(sizeof(*infos)*i_count);
    memcpy(attrib->info, infos, sizeof(*infos)*i_count);
    
    attrib->info_count = i_count;
    
    if(name)
        attrib->name = cstr_dup((char *)name);
    
    return id;
}

internal void
flag_attribute(renderer_t *rb, u32 inst_id, u32 attrib_id)
{
    vertex_array_t *va = rb->va + inst_id;
    attrib_t *attrib = va->mesh_instance.attribs + attrib_id;
    
    attrib->update = 1;
}

internal void
init_instance(instance_t *instance, u32 mesh_id, u32 count)
{
    memset(instance, 0, sizeof(*instance));
    instance->mesh_id = mesh_id;
    instance->count = count;
}

internal void
add_instance(renderer_t *rb, u32 inst_id)
{
    u32 count = get_stack_count(rb->va);
    ASSERT(inst_id > 0 && inst_id < count);
    
    shader_t *shader = rb->shaders+rb->cur_shader;
    *(u32 *)stack_push(&shader->inst_ids) = inst_id;
}

internal void
use_shader(renderer_t *rb, u32 id)
{
    ASSERT(id >= 0 && id < get_stack_count(rb->shaders));
    rb->shaders[id].cmds.idx = get_stack_count(rb->cmds);
    
    rb->cur_shader = id;
}