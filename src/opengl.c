
internal void
init_vertex_array(vertex_array_t *va)
{
    glGenVertexArrays(1, &va->vao);
    glGenBuffers(1, &va->vbo);
    glGenBuffers(1, &va->ebo);
}

internal void
free_vertex_array(vertex_array_t *va)
{
    glDeleteVertexArrays(1, &va->vao);
    glDeleteBuffers(1, &va->vbo);
    glDeleteBuffers(1, &va->ebo);
    
    if (va->verts)
        stack_free(va->verts);
    if (va->indices)
        stack_free(va->indices);
}

internal void
init_renderer(renderer_t *rb, i32 width, i32 height)
{
    vertex_array_t *va = stack_push(&rb->va);
    memset(va, 0, sizeof(*va));
    
    init_camera_default(&rb->cam, HMM_V3(0.0f, 0.0f, 3.0f),
                        0.05f, 0.01f, width/(f32)height);
}

internal void
free_renderer(renderer_t *rb)
{
    u32 count = get_stack_count(rb->va);
    for (u32 i = 0; i < count; i++)
        free_vertex_array(rb->va+i);
    
    if (rb->shaders)
        stack_free(rb->shaders);
    
    if (rb->mats)
        stack_free(rb->mats);
    
    if (rb->cmds)
        stack_free(rb->cmds);
    
    if (rb->transforms)
        stack_free(rb->transforms);
}

internal void
update_renderer(renderer_t *rb)
{
    vertex_array_t *va;
    u32 count = get_stack_count(rb->va);
    
    for (u32 i = 0; i < count; i++)
    {
        va = rb->va+i;
        
        init_vertex_array(va);
        
        LOG("vao %d", va->vao);
        
        glBindVertexArray(va->vao);
        
        u64 vert_count = get_stack_count(va->verts);
        u64 index_count = get_stack_count(va->indices);
        LOG("Submitting %llu verts, %llu indices", vert_count, index_count);
        
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(*va->verts)*vert_count,
                     va->verts, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*va->indices)*index_count,
                     va->indices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vertex_t), (void *)offsetof(vertex_t, pos));
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vertex_t), (void *)offsetof(vertex_t, norm));
        glEnableVertexAttribArray(1);
        
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                              sizeof(vertex_t), (void *)offsetof(vertex_t, uv));
        glEnableVertexAttribArray(2);
        
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              sizeof(vertex_t), (void *)offsetof(vertex_t, color));
        glEnableVertexAttribArray(3);
        
        if (i > 0)
        {
            u32 attrib_count = get_stack_count(va->mesh_instance.attribs);
            attrib_t *attrib;
            shader_t *shader;
            for (u32 j = 0; j < attrib_count; j++)
            {
                attrib = va->mesh_instance.attribs+j;
                shader = rb->shaders+attrib->shader_id;
                
                glGenBuffers(1, &attrib->ibo);
                glBindBuffer(GL_ARRAY_BUFFER, attrib->ibo);
                glBufferData(GL_ARRAY_BUFFER, attrib->count*attrib->size,
                             attrib->data, (attrib->dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
                
                glUseProgram(shader->id);
                
                
                u32 loc = -1;
                u32 has_name = 0;
                if (attrib->name) {
                    loc = glGetAttribLocation(shader->id, attrib->name);
                    if (loc == -1) continue;
                    has_name = 1;
                }
                
                attrib_info_t *info;
                for (u32 k = 0; k < attrib->info_count; k++)
                {
                    info = attrib->info+k;
                    
                    if (!has_name) {
                        loc = glGetAttribLocation(shader->id, info->name);
                        if (loc == -1) continue;
                        has_name = 1;
                    }
                    
                    glEnableVertexAttribArray(loc+k);
                    glVertexAttribPointer(loc+k, info->count, info->type,
                                          info->normalize, attrib->size,
                                          (void *)info->offset);
                    
                    glVertexAttribDivisor(loc+k, attrib->divisor);
                }
                
                glUseProgram(0);
            }
        }
        
        glBindVertexArray(0);
    }
}


internal void
update_assets(renderer_t *rb, asset_manager_t *am)
{
    u32 count = get_stack_count(am->entries);
    
    asset_entry_t *entry;
    for (u32 i = 0; i < count; i++)
    {
        entry = am->entries+i;
        
        switch(entry->type)
        {
            case ASSET_SHADER: {
                shader_entry_t shader = entry->shader;
                
                load_shader(rb, shader);
                
                free(entry->shader.fns);
                free(entry->shader.types);
            } break;
        }
    }
}

internal void
update_attribute(instance_t *inst, attrib_t *attrib)
{
    if (!attrib->dynamic)
        return;
    
    glBindBuffer(GL_ARRAY_BUFFER, attrib->ibo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, inst->count * attrib->size, attrib->data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

internal void
set_uniform(shader_t *shader, u32 loc_type, void *data)
{
    u32 loc = shader->loc[loc_type];
    //LOG("loc %d name %s", loc, shader_loc_names[loc_type]);
    if (loc == -1) {
        LOG("Invalid shader location: %s", shader_loc_names[loc_type]);
        return;
    }
    
    if (loc_type >= SHADER_LOC_FLOAT_POINT_LIGHT0_CONSTANT &&
        loc_type <= SHADER_LOC_FLOAT_POINT_LIGHT3_QUADRATIC)
    {
        f32 *f = (f32 *)data;
        glUniform1f(loc, *f);
    }
    else if (loc_type >= SHADER_LOC_VEC3_DIR_LIGHT_DIR &&
             loc_type <= SHADER_LOC_VEC3_MAT_SPECULAR) {
        v3 *v = (v3 *)data; 
        glUniform3f(loc, v->X, v->Y, v->Z);
    }
    else if (loc_type >= SHADER_LOC_MATRIX_VIEW &&
             loc_type <= SHADER_LOC_MATRIX_MODEL) {
        mat4 *mat = (mat4 *)data; 
        glUniformMatrix4fv(loc, 1, GL_FALSE, (f32 *)mat->Elements);
    }
    else
        ASSERT_LOG(0, "Unvalid shader location type: %s", shader_loc_names[loc_type]);
}

internal void
set_dir_light(renderer_t *rb, shader_t *shader)
{
    dir_light_t light = rb->dir_light;
    
    set_uniform(shader, SHADER_LOC_VEC3_DIR_LIGHT_DIR, (void *)&light.dir);
    set_uniform(shader, SHADER_LOC_VEC3_DIR_LIGHT_AMBIENT, (void *)&light.ambient);
    set_uniform(shader, SHADER_LOC_VEC3_DIR_LIGHT_DIFFUSE, (void *)&light.diffuse);
    set_uniform(shader, SHADER_LOC_VEC3_DIR_LIGHT_SPECULAR, (void *)&light.specular);
}

internal void
set_point_lights(renderer_t *rb, shader_t *shader)
{
    point_light_t *light;
    
    for (u32 i = 0; i < rb->point_light_count; i++) {
        light = rb->point_lights + i;
        
        set_uniform(shader, SHADER_LOC_VEC3_POINT_LIGHT0_POS + i*4, (void *)&light->pos);
        
        set_uniform(shader, SHADER_LOC_FLOAT_POINT_LIGHT0_CONSTANT + i*3, (void *)&light->constant);
        set_uniform(shader, SHADER_LOC_FLOAT_POINT_LIGHT0_LINEAR + i*3, (void *)&light->linear);
        set_uniform(shader, SHADER_LOC_FLOAT_POINT_LIGHT0_QUADRATIC + i*3, (void *)&light->quadratic);
        
        set_uniform(shader, SHADER_LOC_VEC3_POINT_LIGHT0_AMBIENT + i*4, (void *)&light->ambient);
        set_uniform(shader, SHADER_LOC_VEC3_POINT_LIGHT0_DIFFUSE + i*4, (void *)&light->diffuse);
        set_uniform(shader, SHADER_LOC_VEC3_POINT_LIGHT0_SPECULAR + i*4, (void *)&light->specular);
    }
}

internal void
set_material(renderer_t *rb, shader_t *shader, u32 mat_id)
{
    material_t mat = rb->mats[mat_id];
    
    set_uniform(shader, SHADER_LOC_VEC3_MAT_AMBIENT, (void *)&mat.ambient);
    set_uniform(shader, SHADER_LOC_VEC3_MAT_DIFFUSE, (void *)&mat.diffuse);
    set_uniform(shader, SHADER_LOC_VEC3_MAT_SPECULAR, (void *)&mat.specular);
}

internal void
submit_renderer(renderer_t *rb, shader_t *shader)
{
    render_cmd_t *cmd;
    vertex_array_t *vas = rb->va;
    
    glBindVertexArray(vas->vao);
    
    int i;
    for (i = 0; i < shader->cmds.count; i++)
    {
        cmd = rb->cmds+shader->cmds.idx+i;
        
        mesh_info_t *m_info = rb->meshes+cmd->mesh_id;
        mat4 *trans = rb->transforms+cmd->trans_id;
        
        set_uniform(shader, SHADER_LOC_MATRIX_MODEL, trans);
        set_material(rb, shader, cmd->mat_id);
        
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vas->ebo);
        glDrawElements(m_info->prim_type, m_info->indices_count,
                       GL_UNSIGNED_INT, (void *)m_info->indices_idx);
    }
    
    glBindVertexArray(0);
    
    shader->cmds.idx = 0;
    shader->cmds.count = 0;
    stack_clear(rb->cmds);
    
    u32 inst_count = get_stack_count(shader->inst_ids);
    vertex_array_t *va;
    for (i = 0; i < inst_count; i++)
    {
        va = vas+shader->inst_ids[i];
        
        glBindVertexArray(va->vao);
        
        instance_t inst = va->mesh_instance;
        mesh_info_t *m_info = rb->meshes+inst.mesh_id;
        
        glDrawElementsInstanced(m_info->prim_type, m_info->indices_count,
                                GL_UNSIGNED_INT, (void *)m_info->indices_idx,
                                inst.count);
        
        glBindVertexArray(0);
    }
    
    stack_clear(shader->inst_ids);
}

internal void
start_frame(renderer_t *rb)
{
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

internal void
end_frame(renderer_t *rb)
{
    shader_t *shader;
    u32 shader_count = get_stack_count(rb->shaders);
    
    for (u32 s_idx = 0; s_idx < shader_count; s_idx++)
    {
        shader = rb->shaders+s_idx;
        
        u32 inst_count = get_stack_count(shader->inst_ids);
        for (u32 i = 0; i < inst_count; i++)
        {
            instance_t *inst = &rb->va[i+1].mesh_instance;
            attrib_t *attrib;
            u32 attrib_count = get_stack_count(inst->attribs);
            
            for (u32 j = 0; j < attrib_count; j++)
            {
                attrib = inst->attribs + j;
                if (attrib->update) {
                    update_attribute(inst, attrib);
                    attrib->update = 0;
                }
            }
        }
        
        glUseProgram(shader->id);
        set_dir_light(rb, shader);
        set_point_lights(rb, shader);
        set_uniform(shader, SHADER_LOC_VEC3_VIEW_POS, (void *)&rb->cam.pos);
        set_uniform(shader, SHADER_LOC_MATRIX_VIEW, (void *)&rb->cam.view);
        set_uniform(shader, SHADER_LOC_MATRIX_PROJECTION, (void *)&rb->cam.pers);
        
        submit_renderer(rb, shader);
        
        glUseProgram(0);
    }
}
