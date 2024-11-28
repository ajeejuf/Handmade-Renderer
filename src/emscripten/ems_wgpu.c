
#include <webgpu\webgpu.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "ems_renderer.h"
#include "wgpu_renderer.h"

#include "wgpu_utils.c"

#include "wgpu.c"

internal void
init_ems_wgpu(wgpu_renderer_t *wgpu, const char *canvas)
{
    WGPUInstance instance = wgpuCreateInstance(NULL);
    
    ASSERT_LOG(instance, "Could not initialize WebGPU!");
    
    LOG("WGPU instance: %p\n", instance);
    
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canv_desc = (WGPUSurfaceDescriptorFromCanvasHTMLSelector) {
        .chain = {
            .sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector,
            .next = NULL,
        },
        
        .selector = canvas
    };
    
    WGPUSurfaceDescriptor surf_desc = (WGPUSurfaceDescriptor) {
        .nextInChain = (const WGPUChainedStruct *)&canv_desc
    };
    
    WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surf_desc);
    
    wgpu->instance = instance;
    wgpu->surface = surface;
}


INIT_RENDERER(init_renderer)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)malloc(sizeof(*wgpu));
    
    // NOTE(ajeej): Initialize WGPU
    init_ems_wgpu(wgpu, canvas);
    
    u32 w = 0, h = 0;
    emscripten_get_canvas_element_size(canvas, (i32 *)&w, (i32 *)&h);
    init_wgpu(wgpu, w, h, request_adapter_callback, request_device_callback);
    
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