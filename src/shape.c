
internal u32
push_mesh_info(renderer_t *rb, u32 idx_count, u32 vert_base, u32 prim_type)
{
    u32 id = get_stack_count(rb->meshes);
    
    mesh_info_t *info = stack_push(&rb->meshes);
    info->indices_idx = get_stack_count(rb->indices);
    info->indices_count = idx_count;
    info->vert_base = vert_base;
    info->prim_type = prim_type;
    
    return id;
}

internal u32
create_triangle(renderer_t *rb, v4 color)
{
    u32 vert_base = get_stack_count(rb->verts);
    
    vertex_t *verts = stack_push_array(&rb->verts, 3);
    verts[0] = (vertex_t) {
        HMM_V3(-0.5f, -0.5f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
        HMM_V2(0.0f, 0.0f),
        color,
    };
    
    verts[1] = (vertex_t) {
        HMM_V3(0.5f, -0.5f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
        HMM_V2(1.0f, 0.0f),
        color,
    };
    
    verts[2] = (vertex_t) {
        HMM_V3(0.0f, 0.5f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
        HMM_V2(0.5f, 1.0f),
        color,
    };
    
    u32 id = push_mesh_info(rb, 3, vert_base, PRIMITIVE_TRIANGLES);
    
    u32 *indices = stack_push_array(&rb->indices, 3);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    
    return id;
}

internal u32
create_textured_quad(renderer_t *rb, v3 center, v2 dim, v2 uv[4], v4 color) 
{
    u32 vert_base = get_stack_count(rb->verts);
    
    vertex_t *verts = stack_push_array(&rb->verts, 4);
    verts[0] = (vertex_t) {
        HMM_V3(center.X - dim.X/2.0f, center.Y - dim.Y/2.0f, center.Z),
        HMM_V3(0.0f, 0.0f, 1.0f),
        uv[0],
        color
    };
    verts[1] = (vertex_t) {
        HMM_V3(center.X + dim.X/2.0f, center.Y - dim.Y/2.0f, center.Z),
        HMM_V3(0.0f, 0.0f, 1.0f),
        uv[1],
        color
    };
    verts[2] = (vertex_t) {
        HMM_V3(center.X + dim.X/2.0f, center.Y + dim.Y/2.0f, center.Z),
        HMM_V3(0.0f, 0.0f, 1.0f),
        uv[2],
        color
    };
    verts[3] = (vertex_t) {
        HMM_V3(center.X - dim.X/2.0f, center.Y + dim.Y/2.0f, center.Z),
        HMM_V3(0.0f, 0.0f, 1.0f),
        uv[3],
        color
    };
    
    u32 id = push_mesh_info(rb, 6, vert_base, PRIMITIVE_TRIANGLES);
    
    u32 *indices = stack_push_array(&rb->indices, 6);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;
    
    return id;
}

internal u32
create_quad(renderer_t *rb, v3 center, v2 dim, v4 color)
{
    v2 uv[4] = {
        HMM_V2(0.0f, 1.0f),
        HMM_V2(1.0f, 1.0f),
        HMM_V2(1.0f, 0.0f),
        HMM_V2(0.0f, 0.0f)
    };
    
    return create_textured_quad(rb, center, dim, uv, color);
}

internal u32
create_cube(renderer_t *rb, v4 color)
{
    vertex_t nv[] = {
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, 0.5f), HMM_V3(0.0f, 0.0f, 1.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, 0.5f), HMM_V3(0.0f, 0.0f, 1.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, 0.5f), HMM_V3(0.0f, 0.0f, 1.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, 0.5f, 0.5f), HMM_V3(0.0f, 0.0f, 1.0f),
            HMM_V2(1.0f, 1.0f), color
        },
        
        (vertex_t){
            HMM_V3(0.5f, 0.5f, -0.5f), HMM_V3(0.0f, 0.0f, -1.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, -0.5f), HMM_V3(0.0f, 0.0f, -1.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, -0.5f), HMM_V3(0.0f, 0.0f, -1.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, -0.5f), HMM_V3(0.0f, 0.0f, -1.0f),
            HMM_V2(1.0f, 1.0f), color
        },
        
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, -0.5f), HMM_V3(-1.0f, 0.0f, 0.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, -0.5f), HMM_V3(-1.0f, 0.0f, 0.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, 0.5f), HMM_V3(-1.0f, 0.0f, 0.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, 0.5f), HMM_V3(-1.0f, 0.0f, 0.0f),
            HMM_V2(1.0f, 1.0f), color
        },
        
        (vertex_t){
            HMM_V3(0.5f, 0.5f, 0.5f), HMM_V3(1.0f, 0.0f, 0.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, 0.5f), HMM_V3(1.0f, 0.0f, 0.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, -0.5f), HMM_V3(1.0f, 0.0f, 0.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, 0.5f, -0.5f), HMM_V3(1.0f, 0.0f, 0.0f),
            HMM_V2(1.0f, 1.0f), color
        },
        
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, -0.5f), HMM_V3(0.0f, 1.0f, 0.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, 0.5f, 0.5f), HMM_V3(0.0f, 1.0f, 0.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, 0.5f, 0.5f), HMM_V3(0.0f, 1.0f, 0.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, 0.5f, -0.5f), HMM_V3(0.0f, 1.0f, 0.0f),
            HMM_V2(1.0f, 1.0f), color
        },
        
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, 0.5f), HMM_V3(0.0f, -1.0f, 0.0f),
            HMM_V2(0.0f, 1.0f), color
        },
        (vertex_t){
            HMM_V3(-0.5f, -0.5f, -0.5f), HMM_V3(0.0f, -1.0f, 0.0f),
            HMM_V2(0.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, -0.5f), HMM_V3(0.0f, -1.0f, 0.0f),
            HMM_V2(1.0f, 0.0f), color
        },
        (vertex_t){
            HMM_V3(0.5f, -0.5f, 0.5f), HMM_V3(0.0f, -1.0f, 0.0f),
            HMM_V2(1.0f, 1.0f), color
        },
    };
    
    u32 ni[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    
    u32 vert_base = get_stack_count(rb->verts);
    vertex_t *verts = stack_push_array(&rb->verts, 24);
    memcpy(verts, nv, sizeof(nv));
    
    u32 id = push_mesh_info(rb, 36, vert_base, PRIMITIVE_TRIANGLES);
    
    u32 *indices = stack_push_array(&rb->indices, 36);
    memcpy(indices, ni, sizeof(ni));
    
    return id;
}

internal u32
create_sphere(renderer_t *rb, u32 stacks, u32 slices, v4 color)
{
    f32 pi = HMM_PI;
    f32 h_pi = pi / 2.0f;
    
    u32 v_count = (stacks+1)*(slices+1);
    u32 i_count = stacks*slices*6;
    
    u32 vert_base  = get_stack_count(rb->verts);
    vertex_t *verts = stack_push_array(&rb->verts, v_count);
    
    u32 id = push_mesh_info(rb, i_count, vert_base, PRIMITIVE_TRIANGLES);
    
    u32 *indices = stack_push_array(&rb->indices, i_count);
    
    f32 sector_step = 2 * pi / slices;
    f32 stack_step = pi /stacks;
    
    u32 i, j;
    for (i = 0; i <= stacks; i++)
    {
        f32 phi = h_pi - i*stack_step;
        f32 xy = HMM_CosF(phi);
        f32 z = HMM_SinF(phi);
        
        for (j = 0; j <= slices; j++)
        {
            f32 theta = j * sector_step;
            f32 x = xy * HMM_CosF(theta);
            f32 y = xy * HMM_SinF(theta);
            
            verts->pos = HMM_V3(x, y, z);
            verts->norm = HMM_V3(x, y, z);
            verts->uv = HMM_V2((f32)j/slices, (f32)i/stacks);
            verts->color = color;
            
            verts++;
        }
    }
    
    u32 k1, k2;
    for (u32 i = 0; i < stacks; i++)
    {
        k1 = i * (slices + 1);
        k2 = k1 + slices + 1;
        
        for (u32 j = 0; j < slices; j++, k1++, k2++)
        {
            if (i != 0)
            {
                *(indices++) = k1;
                *(indices++) = k2;
                *(indices++) = k1+1;
            }
            
            if (i != (stacks-1))
            {
                *(indices++) = k1+1;
                *(indices++) = k2;
                *(indices++) = k2+1;
            }
        }
    }
    
    return id;
}

internal v3 *
get_icosahedron_verts()
{
    f32 h_angle = HMM_PI / 180.0f * 72.0f;
    f32 v_angle = atanf(1.0f / 2.0f);
    
    u32 vert_count = 12;
    v3 *verts = malloc(vert_count*sizeof(*verts));
    i32 i1, i2;
    f32 z, xy;
    f32 h_angle1 = -HMM_PI/2 - h_angle/2;
    f32 h_angle2 = -HMM_PI/2;
    
    verts[0] = HMM_V3(0, 0, 1);
    
    for (u32 i = 1; i <= 5; i++)
    {
        i1 = i;
        i2 = (i+5);
        
        z = HMM_SinF(v_angle);
        xy = HMM_CosF(v_angle);
        
        verts[i1] = HMM_V3(xy * HMM_CosF(h_angle1),
                           xy * HMM_SinF(h_angle1),
                           z);
        
        verts[i2] = HMM_V3(xy * HMM_CosF(h_angle2),
                           xy * HMM_SinF(h_angle2),
                           -z);
        
        h_angle1 += h_angle;
        h_angle2 += h_angle;
    }
    
    i1 = 11;
    verts[i1] = HMM_V3(0, 0, -1);
    
    return verts;
}

internal v3
compute_face_norm(v3 v_1, v3 v_2, v3 v_3)
{
    v3 e1 = HMM_SubV3(v_2, v_1);
    v3 e2 = HMM_SubV3(v_3, v_1);
    
    v3 n = HMM_Cross(e1, e2);
    f32 len = HMM_LenV3(n);
    
    if (len > 0.000001f)
        n = HMM_MulV3F(n, 1.0f/len);
    
    return n;
}

internal void
build_icosahedron(v3 *verts, v4 color,
                  vertex_t **out_v, u32 *out_v_count,
                  u32 **out_i, u32 *out_i_count)
{
    u32 i_count;
    u32 v_count = i_count = 5*4*3;
    vertex_t *v = malloc(sizeof(*v)*v_count);
    u32 *i = malloc(sizeof(*i)*i_count);
    
    f32 s_step = 186 / 2048.0f;
    f32 t_step = 322 / 1024.0f;
    
    v3 v_0, v_1, v_2, v_3, v_4, v_11;
    v3 n;
    v2 t0, t1, t2, t3, t4, t11;
    u32 idx = 0;
    
    v_0 = verts[0];
    v_11 = verts[11];
    for (u32 j = 1; j <= 5; j++)
    {
        v_1 = verts[j];
        v_3 = verts[j+5];
        if (j < 5) {
            v_2 = verts[j+1];
            v_4 = verts[j+6];
        }
        else {
            v_2 = verts[1];
            v_4 = verts[6];
        }
        
        t0 = HMM_V2((2*j-1)*s_step, 0);
        t1 = HMM_V2((2*j-2)*s_step, t_step);
        t2 = HMM_V2((2*j-0)*s_step, t_step);
        t3 = HMM_V2((2*j-1)*s_step, t_step*2);
        t4 = HMM_V2((2*j+1)*s_step, t_step*2);
        t11 = HMM_V2((2*j*s_step)*s_step, t_step*3);
        
        n = compute_face_norm(v_0, v_1, v_2);
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_0, n, t0, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_1, n, t1, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_2, n, t2, color
        };
        
        n = compute_face_norm(v_1, v_3, v_2);
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_1, n, t1, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_3, n, t3, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_2, n, t2, color
        };
        
        n = compute_face_norm(v_2, v_3, v_4);
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_2, n, t2, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_3, n, t3, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_4, n, t4, color
        };
        
        n = compute_face_norm(v_3, v_11, v_4);
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_3, n, t3, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_11, n, t11, color
        };
        
        i[idx] = idx;
        v[idx++] = (vertex_t) {
            v_4, n, t4, color
        };
    }
    
    *out_v = v;
    *out_v_count = v_count;
    *out_i = i;
    *out_i_count = i_count;
}


internal u32
create_icosahedron(renderer_t *rb, v4 color)
{
    v3 *temp_v = get_icosahedron_verts();
    
    vertex_t *v;
    u32 *i;
    u32 v_count, i_count;
    build_icosahedron(temp_v, color, &v, &v_count, &i, &i_count);
    
    u32 vert_base = get_stack_count(rb->verts);
    vertex_t *verts = stack_push_array(&rb->verts, v_count);
    memcpy(verts, v, v_count*sizeof(*verts));
    
    u32 id = push_mesh_info(rb, i_count, vert_base, PRIMITIVE_TRIANGLES);
    
    u32 *indices = stack_push_array(&rb->indices, i_count);
    
    free(i); free(v);
    free(temp_v);
    
    return id;
}