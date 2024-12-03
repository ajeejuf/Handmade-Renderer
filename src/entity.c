
internal void
update_transform(mat4 *out_trans, v3 pos, v3 rot, v3 scale)
{
    mat4 trans = HMM_M4D(1.0f);
    trans = HMM_MulM4(trans, HMM_Translate(pos));
    
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.X), 
                                           HMM_V3(1.0f, 0.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.Y), 
                                           HMM_V3(0.0f, 1.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.Z), 
                                           HMM_V3(0.0f, 0.0f, 1.0f)));
    
    trans = HMM_MulM4(trans, HMM_Scale(scale));
    
    *out_trans = trans;
}

internal u32
create_transform(renderer_t *rb, transform_info_t info)
{
    u32 id = get_stack_count(rb->transforms);
    
    mat4 *trans = (mat4 *)stack_push(&rb->transforms);
    
    update_transform(trans, info.pos, info.rot, info.scale);
    
    return id;
}

internal u32
create_transforms(renderer_t *rb, transform_info_t *infos, u32 count)
{
    u32 id = get_stack_count(rb->transforms);
    
    for (u32 i = 0; i < count; i++)
        create_transform(rb, infos[i]);
    
    return id;
}

internal u32
add_physics(app_t *app, v3 v)
{
    u32 id = get_stack_count(app->physics);
    physic_comp_t *comp = stack_push(&app->physics);
    
    comp->a = HMM_V3(0.0f, 0.0f, 0.0f);
    comp->v = v;
    
    return id;
}

internal u32
add_entity(app_t *app, transform_info_t trans, 
           u32 mesh_id, u32 mat_id, u32 physic_id)
{
    u32 id = get_stack_count(app->entities);
    entity_t *entity = stack_push(&app->entities);
    
    entity->trans_id = create_transform(&app->rb, trans);
    entity->mesh_id = mesh_id;
    entity->mat_id = mat_id;
    entity->physic_id = physic_id;
    
    return id;
}

internal u32
add_text(app_t *app, u32 font_id, u32 count, v3 pos, f32 size)
{
    u32 id = get_stack_count(app->texts);
    text_t *text = stack_push(&app->texts);
    
    text->font_id = font_id;
    text->ch_count = count;
    text->size = size;
    text->pos = pos;
    text->update = 1;
    
    transform_info_t *info = (transform_info_t *)malloc(sizeof(*info)*count);
    for (u32 i = 0; i < count; i++)
    {
        info[i].pos = HMM_V3(0.0f, 0.0f, 0.0f);
        info[i].rot = HMM_V3(0.0f, 0.0f, 0.0f);
        info[i].scale = HMM_V3(1.0f, 1.0f, 1.0f);
    }
    
    text->trans_id = create_transforms(&app->rb, info, count);
    
    return id;
}

internal void
update_entity_physics(app_t *app, u32 id, f32 dt)
{
    entity_t entity = app->entities[id];
    physic_comp_t physics = app->physics[entity.physic_id];
    mat4 *trans = app->rb.transforms + entity.trans_id;
    
    v3 dp = HMM_MulV3F(physics.v, dt);
    
    trans->Elements[3][0] += dp.X;
    trans->Elements[3][1] += dp.Y;
    trans->Elements[3][2] += dp.Z;
}

internal void
render_entity(app_t *app, u32 id)
{
    entity_t *entity = app->entities + id;
    
    push_render_cmd(&app->rb, entity->mesh_id, entity->mat_id, entity->trans_id, 1);
}

internal void
render_text(app_t *app, u32 id, const char* str)
{
    text_t *text = app->texts + id;
    
    ASSERT_LOG(strlen(str) <= text->ch_count,
               "String is too larger for text.");
    
    char c;
    u32 c_idx, count = strlen(str);
    
    font_info_t info = app->rb.fonts[text->font_id];
    mat4 *trans = app->rb.transforms + text->trans_id;
    
    v3 pos = text->pos;
    f32 size = text->size;
    for (u32 i = 0; i < count; i++)
    {
        c = str[i];
        
        if (c == '\n') {
            // TODO(ajeej): handle new line case;
            continue;
        }
        
        
        ASSERT_LOG(c >= info.start && c < info.start + info.count,
                   "Character not in font.");
        
        c_idx = c - info.start;
        
        f32 kern = 0.0;
        if (i < count-1)
        {
            u32 nc_idx = str[i+1]-info.start;
            kern = info.kerning[c_idx][nc_idx];
        }
        
        update_transform(trans, 
                         HMM_AddV3(pos, 
                                   HMM_V3(info.xoff[c_idx]*size, 
                                          info.yoff[c_idx]*size, 
                                          0.0f)),
                         HMM_V3(0.0f, 0.0f, 0.0f),
                         HMM_V3(size, size, 1.0f));
        
        pos = HMM_AddV3(pos, HMM_V3((info.xadvance[c_idx]+kern)*size, 0.0f, 0.0f));
        
        push_render_cmd(&app->rb, info.mesh_start_id + c_idx, 0, text->trans_id + i, 1);
        
        trans++;
    }
}