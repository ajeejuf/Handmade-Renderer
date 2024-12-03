
internal color_t
make_color(f32 r, f32 g, f32 b, f32 a) 
{
    return (color_t) {
        r, g, b, a
    };
}

#define init_camera_default(c, p, sp, sn, w, h) \
init_camera(c, p, sp, sn, w, h, 1.0f, 45.0f, 1.0f, 100.0f);

internal void
update_camera(camera_t *cam, u32 update_flag)
{
    if (update_flag & CAMERA_UPDATE_PROJECTION)
    {
        f32 aspect_ratio = cam->width/(f32)cam->height;
        
        cam->pers = HMM_Perspective_RH_ZO(HMM_ToRad(cam->fov), aspect_ratio,
                                          cam->n, cam->f);
        
        f32 h = cam->orth_h;
        f32 w = h * aspect_ratio;
        f32 l = -w/2.0f, b = -h/2.0f;
        cam->orth = HMM_Orthographic_RH_ZO(l, -l, b, -b, cam->n, cam->f);
        
        cam->world_to_screen_or = (f32)cam->height/h;
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
            u32 width, u32 height, f32 orth_h,
            f32 fov, f32 near_plane, f32 far_plane)
{
    cam->pos = pos;
    cam->world_up = HMM_V3(0.0f, 1.0f, 0.0f);
    
    quat x = HMM_QFromAxisAngle_RH(HMM_V3(1.0f, 0.0f, 0.0f), HMM_ToRad(0.0f));
    quat y = HMM_QFromAxisAngle_RH(HMM_V3(0.0f, 1.0f, 0.0f), HMM_ToRad(0.0f));
    quat z = HMM_QFromAxisAngle_RH(HMM_V3(0.0f, 0.0f, 1.0f), HMM_ToRad(0.0f));
    cam->rot = HMM_MulQ(z, HMM_MulQ(y, x));
    
    cam->speed = speed;
    cam->sens = sens;
    cam->orth_h = orth_h;
    cam->fov = fov;
    cam->width = width;
    cam->height = height;
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
add_texture(renderer_t *rb, void *data, u32 width, u32 height, u32 format, u32 usage)
{
    u32 id = get_stack_count(rb->textures);
    
    texture_info_t *info = (texture_info_t *)stack_push(&rb->textures);
    
    info->data = data;
    info->dim_type = TEXTURE_DIM_2D;
    info->mip_level_count = 1;
    info->sample_count = 1;
    info->format = format;
    info->usage = usage;
    
    info->size[0] = width;
    info->size[1] = height;
    info->size[2] = 1;
    
    return id;
}

internal void
init_app_renderer(renderer_t *rb, i32 width, i32 height)
{
    memset(rb, 0, sizeof(*rb));
    
    rb->width = width; rb->height = height;
    
    u8 *data = (u8 *)malloc(width*height*4);
    add_texture(rb, data, width, height, TEXTURE_FORMAT_RGBA8U_NORM, TEXTURE_USAGE_RENDER_ATTACHMENT | TEXTURE_USAGE_COPY_DST);
    add_texture(rb, NULL, 0, 0, 0, 0);
}

internal void
free_render_pipeline(render_pipeline_t *rp)
{
    if (rp == NULL) return;
    
    if (rp->vb_layout_ids)
        free(rp->vb_layout_ids);
    
    if (rp->bg_layout_ids)
        free(rp->bg_layout_ids);
    
    if (rp->bg_const_ids)
        stack_free(rp->bg_const_ids);
    
    if (rp->bg_frame_ids)
        stack_free(rp->bg_frame_ids);
    
    if (rp->bg_draw_ids)
        stack_free(rp->bg_draw_ids);
}

internal void
free_compute_pipeline(compute_pipeline_t *cp)
{
    if (cp == NULL) return;
    
    if (cp->bg_layout_ids)
        free(cp->bg_layout_ids);
}

internal void
free_vertex_buffer_layout(vertex_buffer_layout_t *vbl)
{
    if (vbl == NULL) return;
    
    if (vbl->attribs)
        free(vbl->attribs);
}

internal void
free_font_info(font_info_t *fi)
{
    if (fi == NULL) return;
    
    if (fi->xoff)
        free(fi->xoff);
    
    if (fi->yoff)
        free(fi->yoff);
    
    if (fi->xadvance)
        free(fi->xadvance);
}

internal void
free_app_renderer(renderer_t *rb)
{
    u32 i;
    for (i = 0; i < get_stack_count(rb->render_pipelines); i++)
        free_render_pipeline(rb->render_pipelines+i);
    
    if (rb->render_pipelines)
        stack_free(rb->render_pipelines);
    
    
    for (i = 0; i < get_stack_count(rb->compute_pipelines); i++)
        free_compute_pipeline(rb->compute_pipelines+i);
    
    if (rb->compute_pipelines)
        stack_free(rb->compute_pipelines);
    
    
    if (rb->samplers)
        stack_free(rb->samplers);
    
    if (rb->textures)
        stack_free(rb->textures);
    
    if (rb->buffers)
        stack_free(rb->buffers);
    
    
    for (i = 0; i < get_stack_count(rb->vb_layouts); i++)
        free_vertex_buffer_layout(rb->vb_layouts+i);
    
    if (rb->vb_layouts)
        stack_free(rb->vb_layouts);
    
    if (rb->bg_layouts)
        stack_free(rb->vb_layouts);
    
    if (rb->cmds)
        stack_free(rb->cmds);
    
    if (rb->bind_updates)
        stack_free(rb->bind_updates);
    
    if (rb->verts)
        stack_free(rb->verts);
    
    if (rb->indices)
        stack_free(rb->indices);
    
    if (rb->render_cmds)
        stack_free(rb->render_cmds);
    
    if (rb->mats)
        stack_free(rb->mats);
    
    if (rb->meshes)
        stack_free(rb->meshes);
    
    
    for (i = 0; i < get_stack_count(rb->fonts); i++)
        free_font_info(rb->fonts+i);
    
    if (rb->fonts)
        stack_free(rb->fonts);
    
    if (rb->cams)
        stack_free(rb->cams);
    
    if (rb->transforms)
        stack_free(rb->transforms);
    
    if (rb->api)
        free(rb->api);
}

internal u32
process_font_asset(renderer_t *rb, asset_manager_t *am, u32 asset_id)
{
    font_asset_t asset = am->font_assets[asset_id];
    
    u32 id = get_stack_count(rb->fonts);
    font_info_t *info = stack_push(&rb->fonts);
    
    info->tex_id = add_texture(rb, asset.atlas_bitmap, 
                               asset.atlas_w, asset.atlas_h,
                               TEXTURE_FORMAT_R8U_NORM,
                               TEXTURE_USAGE_COPY_DST | 
                               TEXTURE_USAGE_TEXTURE_BINDING);
    
    info->mesh_start_id = get_stack_count(rb->meshes);
    
    info->start = asset.ch_start;
    info->count = asset.ch_count;
    
    info->xoff = (f32 *)malloc(sizeof(f32)*asset.ch_count);
    info->yoff = (f32 *)malloc(sizeof(f32)*asset.ch_count);
    info->xadvance = (f32 *)malloc(sizeof(f32)*asset.ch_count);
    info->kerning = (f32 **)malloc(sizeof(f32 *)*asset.ch_count);
    f32 *kern_data = malloc(sizeof(f32)*asset.ch_count*asset.ch_count);
    
    stbtt_packedchar *packed_chs = asset.packed_chs;
    stbtt_aligned_quad *aligned_quads = asset.aligned_quads;
    u32 **kerning = (u32 **)asset.kerning;
    
    f32 scale = 2.0f / rb->height;
    
    v2 glyph_size, center, uvs[4];
    stbtt_packedchar packed_ch;
    stbtt_aligned_quad aligned_quad;
    for (u32 i = 0; i < asset.ch_count; i++)
    {
        packed_ch = packed_chs[i];
        aligned_quad = aligned_quads[i];
        
        glyph_size = HMM_V2(packed_ch.x1 - packed_ch.x0,
                            packed_ch.y1 - packed_ch.y0);
        glyph_size = HMM_MulV2F(glyph_size, scale);
        
        uvs[0] = HMM_V2(aligned_quad.s0, aligned_quad.t1);
        uvs[1] = HMM_V2(aligned_quad.s1, aligned_quad.t1);
        uvs[2] = HMM_V2(aligned_quad.s1, aligned_quad.t0);
        uvs[3] = HMM_V2(aligned_quad.s0, aligned_quad.t0);
        
        center = HMM_DivV2F(glyph_size, 2.0f);
        create_textured_quad(rb, HMM_V3(center.X, center.Y, 0.0f), glyph_size, uvs, 
                             HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
        
        info->xoff[i] = packed_ch.xoff*scale*2;
        info->yoff[i] = packed_ch.yoff*scale*2 + glyph_size.Y;
        info->xadvance[i] = packed_ch.xadvance*scale*2;
        
        info->kerning[i] = kern_data + i*asset.ch_count;
        for (u32 j = 0; j < asset.ch_count; j++)
            kerning[i][j] = (f32)kerning[i][j]*scale*2;
    }
    
    return id;
}

internal void
push_render_cmd(renderer_t *rb, 
                u32 mesh_id, u32 mat_id, u32 trans_id,
                u32 inst_count)
{
    render_cmd_t *cmd = stack_push(&rb->render_cmds);
    
    cmd->mesh_id = mesh_id;
    cmd->mat_id = mat_id;
    cmd->trans_id = trans_id;
    cmd->inst_count = inst_count;
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


internal camera_t *
add_camera(renderer_t *rb, v3 pos, f32 orth_h, f32 fov, f32 n, f32 f)
{
    camera_t cam = {0};
    
    init_camera(&cam, pos, 0.05f, 0.01f, rb->width, rb->height, orth_h, fov, n, f);
    
    camera_t *new_cam = (camera_t *)stack_push(&rb->cams);
    
    *new_cam = cam;
    
    return new_cam;
}

internal u32
add_compute_pipeline(renderer_t *rb, u32 shader_id,
                     u32 *bg_ids, u32 bg_count,
                     u32 x, u32 y, u32 z)
{
    u32 id = get_stack_count(rb->compute_pipelines);
    
    compute_pipeline_t *pipeline = stack_push(&rb->compute_pipelines);
    
    pipeline->shader_id = shader_id;
    
    pipeline->bg_count = bg_count;
    pipeline->bg_layout_ids = (u32 *)malloc(sizeof(*bg_ids)*bg_count);
    memcpy(pipeline->bg_layout_ids, bg_ids, sizeof(*bg_ids)*bg_count);
    
    pipeline->workgroup_x = x;
    pipeline->workgroup_y = y;
    pipeline->workgroup_z = z;
    
    return id;
}

internal u32
add_render_pipeline(renderer_t *rb, u32 shader_id, u32 framebuffer_id, v3 clear, blend_comp_t color, blend_comp_t alpha,
                    u32 *vb_ids, u32 vb_count,
                    u32 *bg_ids, u32 bg_count)
{
    u32 id = get_stack_count(rb->render_pipelines);
    
    render_pipeline_t *pipeline = stack_push(&rb->render_pipelines);
    pipeline->shader_id = shader_id;
    pipeline->fb_id = framebuffer_id;
    pipeline->clear = clear;
    pipeline->color_blend = color;
    pipeline->alpha_blend = alpha;
    
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
        layout = rb->bg_layouts + bg_ids[bg_idx];
        
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
add_buffer(renderer_t *rb, u32 usage, u32 size)
{
    u32 id = get_stack_count(rb->buffers);
    
    buffer_info_t *info = (buffer_info_t *)stack_push(&rb->buffers);
    
    info->size = size;
    info->usage = usage;
    
    return id;
}

internal u32
add_sampler(renderer_t *rb, u32 u_mode, u32 v_mode, u32 w_mode,
            u32 mag_filter, u32 min_filter)
{
    u32 id = get_stack_count(rb->samplers);
    
    sampler_info_t *info = (sampler_info_t *)stack_push(&rb->samplers);
    
    info->am_u = u_mode;
    info->am_v = v_mode;
    info->am_w = w_mode;
    info->mag_filter = mag_filter;
    info->min_filter = min_filter;
    info->mipmap_filter = MIPMAP_FILTER_LINEAR;
    info->lod_min_clamp = 0.0f;
    info->lod_max_clamp = 1.0f;
    info->compare = 0;
    info->max_anisotropy = 1;
    
    return id;
}

#define get_buffer_bind_layout_for_struct(d, b, v, b_i, b_t) \
get_buffer_bind_layout(d, b, v, b_i, b_t, sizeof(*d), 1, 0, 0)

#define get_buffer_bind_layout_for_array(d, c, b, v, b_i, b_t) \
get_buffer_bind_layout(d, b, v, b_i, b_t, sizeof(*d), c, 0, 0)

#define get_dynamic_buffer_bind_layout_for_struct(d, c, b, v, b_i, b_t, i_o) \
get_buffer_bind_layout(d, b, v, b_i, b_t, sizeof(*d), c, 1, i_o)

#define get_dynamic_buffer_bind_layout_for_array(d, c, c_d, b, v, b_i, b_t, i_o) \
get_buffer_bind_layout(d, b, v, b_i, b_t, sizeof(*d)*c, c_d, 1, i_o)

internal bind_layout_t
get_buffer_bind_layout(void *data, u32 binding, u32 visibility, u32 buffer_id, u32 buffer_type,
                       u32 size, u32 count, u32 has_dynamic_offset, u32 id_offset)
{
    bind_layout_t res = {0};
    
    res.type = BINDING_TYPE_BUFFER;
    res.binding = binding;
    res.visibility = visibility;
    res.buffer_layout = (buffer_bind_layout_t) {
        .data = data,
        .buffer_id = buffer_id,
        .buffer_type = buffer_type,
        .offset = 0,
        .size = size,
        .aligned_size = 0,
        .has_dynamic_offset = has_dynamic_offset,
        .count = count,
        .stride = 0,
        .id_offset = id_offset,
    };
    
    return res;
}

internal bind_layout_t
get_texture_bind_layout(u32 binding, u32 visibility, u32 texture_id)
{
    bind_layout_t res = {0};
    
    res.type = BINDING_TYPE_TEXTURE;
    res.binding = binding;
    res.visibility = visibility;
    res.texture_layout = (texture_bind_layout_t) {
        .id = texture_id,
    };
    
    return res;
}

internal bind_layout_t
get_storage_texture_bind_layout(u32 binding, u32 visibility, u32 texture_id, u32 access)
{
    bind_layout_t res = {0};
    
    res.type = BINDING_TYPE_STORAGE_TEXTURE;
    res.binding = binding;
    res.visibility = visibility;
    res.storage_texture_layout = (storage_texture_bind_layout_t) {
        .id = texture_id,
        .access = access,
    };
    
    return res;
}

internal bind_layout_t
get_sampler_bind_layout(u32 binding, u32 visibility, u32 sampler_id, u32 type)
{
    bind_layout_t res = {0};
    
    res.type = BINDING_TYPE_SAMPLER;
    res.binding = binding;
    res.visibility = visibility;
    res.sampler_layout = (sampler_bind_layout_t) {
        .id = sampler_id,
        .type = type
    };
    
    return res;
}

internal u32
add_bind_group(renderer_t *rb, u32 type)
{
    u32 id = get_stack_count(rb->bg_layouts);
    
    bind_group_layout_t *layout = (bind_group_layout_t *)stack_push(&rb->bg_layouts);
    
    memset(layout->binds, 0, sizeof(*layout->binds)*ARRAY_COUNT(layout->binds));
    layout->count = 0;
    layout->type = type;
    
    return id;
}

internal void
add_bind_layouts(renderer_t *rb, u32 bg_id, bind_layout_t *binds, u32 count)
{
    bind_group_layout_t *layout = rb->bg_layouts + bg_id;
    
    ASSERT_LOG(layout->count + count <= ARRAY_COUNT(layout->binds), 
               "Bind Group %d cannot have more than 16 layouts.", bg_id);
    
    memcpy(layout->binds + layout->count, binds, sizeof(*binds)*count);
    layout->count += count;
}

internal void
update_bind_group_layout(renderer_t *rb, u32 bg_id, u32 b_id)
{
    bind_update_info_t *info = (bind_update_info_t *)stack_push(&rb->bind_updates);
    info->bg_id = bg_id;
    info->b_id = b_id;
}

internal void
read_buffer(renderer_t *rb, void *data, u32 b_id, u32 size)
{
    buffer_read_info_t *info = (buffer_read_info_t *)stack_push(&rb->buffer_reads);
    info->data = data;
    info->id = b_id;
    info->size = size;
}

internal void
use_render_pipeline(renderer_t *rb, u32 p_id)
{
    rb->cur_pipeline = p_id;
}

#define add_gpu_cmd(r, t, d) \
_add_gpu_cmd(r, t, d, sizeof(*d))

internal void
_add_gpu_cmd(renderer_t *rb, u32 type, void *data, u32 size)
{
    gpu_cmd_t *cmd = (gpu_cmd_t *)stack_push(&rb->cmds);
    
    cmd->type = type;
    memset(&cmd->data, 0, sizeof(cmd->data));
    memcpy(&cmd->data, data, size);
}

internal void
submit_compute_pipeline(renderer_t *rb, u32 id)
{
    compute_pipeline_submit_t submit = {0};
    submit.id = id;
    
    add_gpu_cmd(rb, GPU_CMD_COMPUTE_PIPELINE_SUBMIT, &submit);
}

internal void
start_render_pipeline(renderer_t *rb, u32 id)
{
    render_pipeline_submit_t submit = {0};
    submit.id = id;
    submit.cmd_start = get_stack_count(rb->render_cmds);
    submit.cmd_count = 0;
    submit.color = HMM_V3(0.0f, 0.0f, 0.0f);
    submit.clear = 0;
    
    add_gpu_cmd(rb, GPU_CMD_RENDER_PIPELINE_SUBMIT, &submit);
}

internal void
clear_color(renderer_t *rb, v3 color)
{
    render_pipeline_submit_t *submit = &get_stack_last(rb->cmds)->data.rp_submit;
    
    submit->color = color;
    submit->clear = 1;
}

internal void
end_render_pipeline(renderer_t *rb)
{
    gpu_cmd_t *cmd = get_stack_last(rb->cmds);
    cmd->data.rp_submit.cmd_count = get_stack_count(rb->render_cmds) - cmd->data.rp_submit.cmd_start;
}

internal void
copy_texture(renderer_t *rb,
             u32 src, u32 src_offset_x, u32 src_offset_y,
             u32 dst, u32 dst_offset_x, u32 dst_offset_y,
             u32 width, u32 height)
{
    texture_copy_t tex_copy = {0};
    tex_copy.src = src;
    tex_copy.dst = dst;
    tex_copy.src_offset_x = src_offset_x;
    tex_copy.src_offset_y = src_offset_y;
    tex_copy.dst_offset_x = dst_offset_x;
    tex_copy.dst_offset_y = dst_offset_y;
    tex_copy.width = width;
    tex_copy.height = height;
    
    add_gpu_cmd(rb, GPU_CMD_TEXTURE_COPY, &tex_copy);
}

internal void
copy_buffer(renderer_t *rb, 
            u32 src, u32 src_offset,
            u32 dst, u32 dst_offset,
            u32 size)
{
    buffer_copy_t buf_copy = {0};
    buf_copy.src = src;
    buf_copy.dst = dst;
    buf_copy.src_offset = src_offset;
    buf_copy.dst_offset = dst_offset;
    buf_copy.size = size;
    
    add_gpu_cmd(rb, GPU_CMD_BUFFER_COPY, &buf_copy);
}

