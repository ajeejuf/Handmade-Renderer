
#include <webgpu\webgpu.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>
#include <glfw3webgpu.h>

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "glfw_renderer.h"
#include "wgpu_renderer.h"

#include "wgpu_utils.c"

#include "wgpu.c"



internal void
init_glfw_wgpu(wgpu_renderer_t *wgpu, GLFWwindow *window)
{
    // NOTE(ajeej): Create WebGPU Instance
    
    WGPUInstanceDescriptor desc = {0};
    desc.nextInChain = NULL;
    
#if __EMSCRIPTEN__
    WGPUInstance instance = wgpuCreateInstance(NULL);
#else
    
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.next = NULL;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;\
    toggles.enabledToggleCount = 1;
    const char *toggle_name = "enable_immediate_error_handling";
    toggles.enabledToggles = &toggle_name;
    
    desc.nextInChain = &toggles.chain;
    
    WGPUInstance instance = wgpuCreateInstance(&desc);
#endif
    
    ASSERT_LOG(instance, "Could not initialize WebGPU!");
    
    LOG("WGPU instance: %p\n", instance);
    
    WGPUSurface surface = glfwCreateWindowWGPUSurface(instance, window);
    
    wgpu->instance = instance;
    wgpu->surface = surface;
}


INIT_RENDERER(init_renderer)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)malloc(sizeof(*wgpu));
    
    // NOTE(ajeej): Initialize WGPU
    init_glfw_wgpu(wgpu, window);
    
    u32 w = 0, h = 0;
    glfwGetWindowSize(window, (i32 *)&w, (i32 *)&h);
    init_wgpu(wgpu, w, h,
              request_adapter_callback,
              request_device_callback);
    
    init_wgpu_renderer(rb, wgpu, am);
    
    rb->api = (void *)wgpu;
}


START_FRAME(start_frame)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)rb->api;
    
    start_wgpu_frame(wgpu);
}


END_FRAME(end_frame)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)rb->api;
    
    end_wgpu_frame(rb, wgpu);
}