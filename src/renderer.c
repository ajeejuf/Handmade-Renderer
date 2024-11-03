
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

internal void
init_app_renderer(renderer_t *rb, i32 width, i32 height)
{
    init_camera_default(&rb->cam, HMM_V3(0.0f, 0.0f, 3.0f),
                        0.05f, 0.01f, width/(f32)height);
}

internal u32
push_mesh_info(renderer_t *rb, u32 idx_count, u32 prim_type)
{
    u32 id = get_stack_count(rb->meshes);
    
    mesh_info_t *info = stack_push(&rb->meshes);
    info->indices_idx = get_stack_count(rb->indices);
    info->indices_count = idx_count;
    info->prim_type = prim_type;
    
    return id;
}

internal void
push_render_cmd(renderer_t *rb, 
                u32 mesh_id, u32 mat_id, u32 trans_id)
{
    render_cmd_t **cmds = &rb->render_pipelines[rb->cur_pipeline].cmds;
    render_cmd_t *cmd = stack_push(cmds);
    
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
add_compute_pipeline(renderer_t *rb, u32 shader_id,
                     u32 *bg_ids, u32 bg_count)
{
    u32 id = get_stack_count(rb->compute_pipelines);
    
    compute_pipeline_t *pipeline = stack_push(&rb->compute_pipelines);
    
    pipeline->bg_count = bg_count;
    pipeline->bg_layout_ids = (u32 *)malloc(sizeof(*bg_ids)*bg_count);
    memcpy(pipeline->bg_layout_ids, bg_ids, sizeof(*bg_ids)*bg_count);
    
    return id;
}

internal u32
add_render_pipeline(renderer_t *rb, u32 shader_id,
                    u32 *vb_ids, u32 vb_count,
                    u32 *bg_ids, u32 bg_count)
{
    u32 id = get_stack_count(rb->render_pipelines);
    
    render_pipeline_t *pipeline = stack_push(&rb->render_pipelines);
    pipeline->shader_id = shader_id;
    pipeline->cmds = NULL;
    
    pipeline->vb_count = vb_count;
    pipeline->vb_layout_ids = (u32 *)malloc(sizeof(*vb_ids)*vb_count);
    memcpy(pipeline->vb_layout_ids, vb_ids, sizeof(*vb_ids)*vb_count);
    
    pipeline->bg_count = bg_count;
    pipeline->bg_layout_ids = (u32 *)malloc(sizeof(*bg_ids)*bg_count);
    memcpy(pipeline->bg_layout_ids, bg_ids, sizeof(*bg_ids)*bg_count);
    
    u32 *bg_layout_id;
    bind_group_layout_t *layout;
    for (u32 bg_idx = 0; bg_idx < bg_count; bg_idx++)
    {
        layout = rb->bg_layouts + bg_idx;
        
        switch (layout->type)
        {
            case BIND_GROUP_TYPE_CONSTANT: {
                bg_layout_id = stack_push(&pipeline->bg_const_ids);
            } break;
            
            case BIND_GROUP_TYPE_FRAME: {
                bg_layout_id = stack_push(&pipeline->bg_frame_ids);
            } break;
            
            case BIND_GROUP_TYPE_DRAW: {
                bg_layout_id = stack_push(&pipeline->bg_draw_ids);
            } break;
            
            default: {
                ASSERT_LOG(0, "Invalid bind group type");
            };
        }
        
        *bg_layout_id = bg_idx;
    }
    
    return id;
}

internal void
get_attribute_info_from_type(u32 type, u32 start_loc, 
                             u32 *out_stride,
                             attribute_t **out_attribs, u32 *out_attrib_count)
{
    attribute_t *attribs;
    u32 stride, attrib_count;
    switch(type)
    {
        case ATTRIBUTE_VERTEX: {
            attribs = (attribute_t *)malloc(sizeof(attribute_t)*4);
            attribs[0].loc = start_loc;
            attribs[0].type = ATTRIBUTE_FORMAT_FLOAT32x3;
            attribs[0].offset = 0;
            
            attribs[1].loc = start_loc+1;
            attribs[1].type = ATTRIBUTE_FORMAT_FLOAT32x3;
            attribs[1].offset = offsetof(vertex_t, norm);
            
            attribs[2].loc = start_loc+2;
            attribs[2].type = ATTRIBUTE_FORMAT_FLOAT32x2;
            attribs[2].offset = offsetof(vertex_t, uv);
            
            attribs[3].loc = start_loc+3;
            attribs[3].type = ATTRIBUTE_FORMAT_FLOAT32x4;
            attribs[3].offset = offsetof(vertex_t, color);
            
            stride = sizeof(vertex_t);
            attrib_count = 4;
        } break;
        
        default: {
            ASSERT("Invalid attribute type");
        } break;
    }
    
    *out_stride = stride;
    *out_attribs = attribs;
    *out_attrib_count = attrib_count;
    
    return;
}

internal u32
add_custom_vertex_buffer(renderer_t *rb,
                         u32 stride, u32 mode,
                         vertex_buffer_data_t data,
                         attribute_t *attribs, u32 attrib_count)
{
    u32 id = get_stack_count(rb->vb_layouts);
    
    vertex_buffer_layout_t *vb_layout = (vertex_buffer_layout_t *)stack_push(&rb->vb_layouts);
    
    vb_layout->stride = stride;
    vb_layout->mode = mode;
    vb_layout->data = data;
    vb_layout->attribs = attribs;
    vb_layout->attrib_count = attrib_count;
    
    return id;
}

internal u32
add_vertex_buffer(renderer_t *rb, 
                  void *data, u64 count, 
                  u32 type, u32 loc, u32 mode)
{
    u32 stride = 0, attrib_count = 0;
    attribute_t *attribs = NULL;
    get_attribute_info_from_type(type, loc, &stride, &attribs, &attrib_count);
    
    vertex_buffer_data_t vb_data = (vertex_buffer_data_t) {
        .data = data,
        .count = count,
        .el_size = stride,
    };
    
    return add_custom_vertex_buffer(rb, stride, mode, vb_data, attribs, attrib_count);
}

internal u32
add_buffer(renderer_t *rb, u32 usage)
{
    u32 id = get_stack_count(rb->buffers);
    
    buffer_info_t *info = (buffer_info_t *)stack_push(&rb->buffers);
    
    info->size = 0;
    info->usage = usage;
    
    return id;
}

#define get_bind_layout_for_struct(d, b, v, b_i, b_t) \
get_bind_layout(d, b, v, b_i, b_t, sizeof(*d), 1, 0, 0)

#define get_bind_layout_for_array(d, c, b, v, b_i, b_t) \
get_bind_layout(d, b, v, b_i, b_t, sizeof(*d), c, 0, 0)

#define get_dynamic_bind_layout_for_struct(d, c, b, v, b_i, b_t, i_o) \
get_bind_layout(d, b, v, b_i, b_t, sizeof(*d), c, 1, i_o)

#define get_dynamic_bind_layout_for_array(d, c, c_d, b, v, b_i, b_t, i_o) \
get_bind_layout(d, b, v, b_i, b_t, sizeof(*d)*c, c_d, 1, i_o)

internal bind_layout_t
get_bind_layout(void *data, u32 binding, u32 visibility, u32 buffer_id, u32 buffer_type,
                u32 size, u32 count, u32 has_dynamic_offset, u32 id_offset)
{
    return (bind_layout_t) {
        .data = data,
        .binding = binding,
        .visibility = visibility,
        .buffer_id = buffer_id,
        .buffer_type = buffer_type,
        .offset = 0,
        .size = size,
        .aligned_size = 0,
        .has_dynamic_offset = has_dynamic_offset,
        .count = count,
        .stride = 0,
        .id_offset = id_offset
    };
}

internal u32
add_bind_group(renderer_t *rb, bind_layout_t *binds, u32 count, u32 type)
{
    u32 id = get_stack_count(rb->bg_layouts);
    
    bind_group_layout_t *layout = (bind_group_layout_t *)stack_push(&rb->bg_layouts);
    
    memcpy(layout->binds, binds, sizeof(*binds)*count);
    layout->count = count;
    layout->type = type;
    
    return id;
}

internal void
use_render_pipeline(renderer_t *rb, u32 p_id)
{
    rb->cur_pipeline = p_id;
}

internal void
submit_pipeline(renderer_t *rb, u32 type, u32 id)
{
    pipeline_submission_t *p_submit = (pipeline_submission_t *)stack_push(&rb->p_submit);
    
    p_submit->type = type;
    p_submit->id = id;
}