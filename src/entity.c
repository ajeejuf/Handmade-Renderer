
internal u32
create_transform(renderer_t *rb, transform_info_t info)
{
    u32 id = get_stack_count(rb->transforms);
    
    mat4 trans = HMM_M4D(1.0f);
    trans = HMM_MulM4(trans, HMM_Translate(info.pos));
    
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(info.rot.X), 
                                           HMM_V3(1.0f, 0.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(info.rot.Y), 
                                           HMM_V3(0.0f, 1.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(info.rot.Z), 
                                           HMM_V3(0.0f, 0.0f, 1.0f)));
    
    trans = HMM_MulM4(trans, HMM_Scale(info.scale));
    
    *(mat4 *)stack_push(&rb->transforms) = trans;
    
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
create_entity(app_t *app, transform_info_t trans, 
              u32 mesh_id, u32 mat_id)
{
    u32 id = get_stack_count(app->entities);
    entity_t *entity = stack_push(&app->entities);
    
    entity->trans_id = create_transform(&app->rb, trans);
    entity->mesh_id = mesh_id;
    entity->mat_id = mat_id;
    
    return id;
}

internal void
add_entity(app_t *app, u32 id)
{
    entity_t *entity = app->entities + id;
    
    push_render_cmd(&app->rb, entity->mesh_id, entity->mat_id, entity->trans_id);
}