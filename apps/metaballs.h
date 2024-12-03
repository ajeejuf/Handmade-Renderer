
#ifndef METABALLS_H
#define METABALLS_H

typedef struct screen_grid_t {
    
    union {
        struct {
            u32 width;
            u32 height;
        };
        
        u32 dim[2];
    };
    
    u32 max_res;
    f32 thickness;
    f32 aspect;
    
    u32 res[2];
    
    u32 mesh_id;
    
    u32 line_count;
    
    u32 trans_count;
    mat4 *trans;
    
} screen_grid_t;

typedef struct metaballs_t {
    v2 *pos;
    v2 *vel;
    f32 *rad;
    
    u32 mesh_id;
    
    u32 count;
    u32 max;
    
    u32 width, height;
    v3 color, debug_color;
    
    mat4 *trans;
} metaballs_t;

internal f32
random_f32(f32 min, f32 max)
{
    return min + ((f32)rand() / (f32)RAND_MAX) * (max-min);
}

internal v2
random_v2(f32 min, f32 max)
{
    return HMM_V2(random_f32(min, max), random_f32(min, max));
}

internal void
random_f32_array(f32 *a, u32 count, f32 min, f32 max)
{
    for (u32 i = 0; i < count; i++)
        a[i] = random_f32(min, max);
}

internal void
random_v2_array(v2 *v, u32 count, f32 min, f32 max)
{
    for (u32 i = 0; i < count; i++)
        v[i] = random_v2(min, max);
}

internal void
update_screen_grid(screen_grid_t *grid, u32 res)
{
    if (res > grid->max_res) return;
    
    f32 aspect = grid->aspect;
    
    f32 thickness = grid->thickness;
    u32 x_res = res, y_res = (u32)((f32)res * aspect);
    
    grid->res[0] = x_res; grid->res[1] = y_res;
    
    grid->line_count = x_res + y_res - 2;
    
    f32 w = 2.0f/aspect, h = 2.0f;
    f32 scale_x = w/x_res, scale_y = h/y_res;
    mat4 *cur_trans = grid->trans;
    for (u32 x = 1; x < x_res; x++)
    {
        update_transform(cur_trans, 
                         HMM_V3(scale_x * x - w/2, 0.0f, 0.0f),
                         HMM_V3(0.0f, 0.0f, 0.0f),
                         HMM_V3(w*thickness/grid->width, h, 1.0f));
        cur_trans++;
    }
    
    for (u32 y = 1; y < y_res; y++)
    {
        update_transform(cur_trans, 
                         HMM_V3(0.0f, scale_y * y - h/2.0f, 0.0f),
                         HMM_V3(0.0f, 0.0f, 180.0f),
                         HMM_V3(w, h*thickness/grid->height, 1.0f));
        cur_trans++;
    }
}

internal screen_grid_t 
create_screen_grid(renderer_t *rb,
                   u32 width, u32 height, 
                   u32 res, u32 max_res, 
                   f32 line_thickness)
{
    screen_grid_t grid = {0};
    
    grid.width = width;
    grid.height = height;
    grid.max_res = max_res;
    grid.thickness = line_thickness;
    grid.aspect = height/(f32)width;
    
    grid.mesh_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(1.0f, 1.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    f32 aspect = grid.aspect;
    u32 line_count = max_res-1 + (u32)(max_res*aspect)-1;
    grid.trans = (mat4 *)malloc(sizeof(mat4)*line_count);
    grid.trans_count = line_count;
    
    update_screen_grid(&grid, res);
    
    return grid;
}

internal metaballs_t
create_metaballs(renderer_t *rb, v2 *pos, f32 *rad, v2 *vel, u32 count, u32 max,
                 v3 color, v3 debug_color, f32 width, f32 height)
{
    metaballs_t metaballs = {0};
    
    metaballs.pos = (v2 *)malloc(sizeof(v2)*max);
    metaballs.rad = (f32 *)malloc(sizeof(f32)*max);
    metaballs.vel = (v2 *)malloc(sizeof(v2)*max);
    metaballs.count = count;
    metaballs.max = max;
    metaballs.color = color;
    metaballs.debug_color = debug_color;
    
    metaballs.width = width;
    metaballs.height = height;
    
    metaballs.mesh_id = create_quad(rb, HMM_V3(0.0f, 0.0f, 0.0f), HMM_V2(2.0f, 2.0f), HMM_V4(1.0f, 1.0f, 1.0f, 1.0f));
    
    memcpy(metaballs.pos, pos, sizeof(v2)*count);
    memcpy(metaballs.rad, rad, sizeof(f32)*count);
    memcpy(metaballs.vel, vel, sizeof(v2)*count);
    
    return metaballs;
}

internal void
update_metaballs_physics(metaballs_t *metaballs, f32 dt)
{;
    f32 rad;
    v2 pos, vel;
    for (u32 i = 0; i < metaballs->count; i++)
    {
        pos = metaballs->pos[i];
        vel = metaballs->vel[i];
        rad = metaballs->rad[i];
        
        pos = HMM_AddV2(pos, HMM_MulV2F(vel, dt));
        
        if (pos.X - rad < 0) {
            pos.X = rad + 0.1;
            vel.X = -vel.X;
        } else if (pos.X + rad > (f32)metaballs->width) {
            pos.X = (f32)metaballs->width - rad - 0.1;
            vel.X = -vel.X;
        }
        
        if (pos.Y - rad < 0) {
            pos.Y = rad + 0.1;
            vel.Y = -vel.Y;
        } else if (pos.Y + rad > (f32)metaballs->height) {
            pos.Y = (f32)metaballs->height - rad - 0.1;
            vel.Y = -vel.Y;
        }
        
        metaballs->pos[i] = pos;
        metaballs->vel[i] = vel;
    }
}

internal void
render_screen_grid(renderer_t *rb, screen_grid_t grid)
{
    f32 aspect = grid.height/(f32)grid.width;
    u32 line_count = grid.res[0]-1 + grid.res[1]-1;
    
    push_render_cmd(rb, grid.mesh_id, 0, 0, line_count);
}

internal void
render_metaball_circles(renderer_t *rb, metaballs_t metaballs)
{
    push_render_cmd(rb, metaballs.mesh_id, 0, 0, metaballs.count);
}

#endif //METABALLS_H
