
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
global u32 model_id;
global u32 mat_id;
global u32 entity_id;

INIT_APP(init_app)
{
    shader_id = add_shader(&app->rb, &app->am, 2,
                           VERTEX_SHADER, "vert.glsl",
                           FRAGMENT_SHADER, "frag.glsl");
    
    model_id = create_triangle(&app->rb, make_color(255, 0, 0, 255));
    
    v3 color = HMM_V3(1.0f, 0.0f, 0.0f);
    mat_id = create_material(&app->rb, color, color, color, 10.0f);
    
    transform_info_t trans = (transform_info_t){
        .pos = HMM_V3(0.0f, 0.0f, 0.0f),
        .rot = HMM_V3(0.0f, 25.0f, 0.0f),
        .scale = HMM_V3(1.0f, 1.0f, 1.0f),
    };
    entity_id = create_entity(app, trans, model_id, mat_id);
}

UPDATE_AND_RENDER(update_and_render)
{
    add_entity(app, entity_id);
}