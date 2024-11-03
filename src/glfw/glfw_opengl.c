
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "glfw_renderer.h"

#include "shader.c"
#include "opengl.c"

INIT_RENDERER(init_renderer)
{
    glfwMakeContextCurrent(window);
    glewInit();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    update_assets(rb, am);
    
    update_renderer(rb);
}

START_FRAME(start_frame)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

END_FRAME(end_frame)
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