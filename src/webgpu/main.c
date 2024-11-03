
#include <webgpu\webgpu.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>
#include <glfw3webgpu.h>

#include "utils.h"

typedef struct wgpu_renderer_t {
    GLFWwindow *window;
    WGPUInstance instance;
    WGPUDevice device;
    WGPUQueue queue;
    
    WGPURenderPipeline pipeline;
    
    WGPUSurface surface;
    WGPUTextureFormat surface_format;
} wgpu_renderer_t;

const char *shader_src = "\
@vertex  \n\
fn vs_main(@location(0) in_vertex_pos: vec2f) -> @builtin(position) vec4f { \n \
\treturn vec4f(in_vertex_pos, 0.0, 1.0);  \n\
}  \n\
  \n\
@fragment  \n\
fn fs_main() -> @location(0) vec4f {  \n\
\treturn vec4f(0.2, 0.3, 0.4, 1.0);  \n\
}  \n\
";

#include "wgpu_utils.c"
#include "wgpu_renderer.c"

int main(int argc, char *argv[])
{
    wgpu_renderer_t wgpu = {0};
    wgpu_init(&wgpu, "..\\data\\shaders\\shader.wgsl");
    
    f32 vertex_data[] = {
        -0.5, -0.5, 1.0, 0.0, 0.0,
        +0.5, -0.5, 0.0, 1.0, 0.0,
        +0.5, +0.5, 0.0, 0.0, 1.0,
        -0.5, +0.5, 1.0, 1.0, 0.0
    };
    
    u16 index_data[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    WGPUBufferDescriptor buffer_desc = {0};
    buffer_desc.nextInChain = NULL;
    buffer_desc.size = sizeof(vertex_data);
    buffer_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    buffer_desc.mappedAtCreation = 0;
    
    WGPUBuffer vertex_buffer = wgpuDeviceCreateBuffer(wgpu.device, &buffer_desc);
    
    wgpuQueueWriteBuffer(wgpu.queue, vertex_buffer, 0, vertex_data, buffer_desc.size);
    
    buffer_desc.size = sizeof(index_data);
    buffer_desc.size = (buffer_desc.size + 3) & ~3;
    buffer_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    
    WGPUBuffer index_buffer = wgpuDeviceCreateBuffer(wgpu.device, &buffer_desc);
    
    wgpuQueueWriteBuffer(wgpu.queue, index_buffer, 0, index_data,
                         buffer_desc.size);
    
    while (!glfwWindowShouldClose(wgpu.window)) 
    {
        glfwPollEvents();
        
        WGPUTextureView target_view = get_next_surface_texture_view(wgpu.surface);
        ASSERT_LOG(target_view, "Failed to get target view!");
        
        WGPUCommandEncoderDescriptor encoder_desc = {0};
        encoder_desc.nextInChain = NULL;
        encoder_desc.label = (WGPUStringView){ 
            .data = "Command Encoder",
            .length = strlen("Command Encoder")
        };
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpu.device, &encoder_desc);
        
        
        WGPURenderPassDescriptor render_pass_desc = {0};
        render_pass_desc.nextInChain = NULL;
        
        WGPURenderPassColorAttachment render_pass_color_attachment = {0};
        render_pass_color_attachment.view = target_view;
        render_pass_color_attachment.resolveTarget = NULL;
        render_pass_color_attachment.loadOp = WGPULoadOp_Clear;
        render_pass_color_attachment.storeOp = WGPUStoreOp_Store;
        render_pass_color_attachment.clearValue = (WGPUColor){ 0.8, 0.4, 0.1 };
        render_pass_color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        
        
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &render_pass_color_attachment;
        render_pass_desc.depthStencilAttachment = NULL;
        render_pass_desc.timestampWrites = NULL;
        
        WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
        
        wgpuRenderPassEncoderSetPipeline(render_pass, wgpu.pipeline);
        
        wgpuRenderPassEncoderSetVertexBuffer(render_pass, 0, vertex_buffer, 0,
                                             wgpuBufferGetSize(vertex_buffer));
        wgpuRenderPassEncoderSetIndexBuffer(render_pass, index_buffer, WGPUIndexFormat_Uint16,
                                            0, wgpuBufferGetSize(index_buffer));
        
        wgpuRenderPassEncoderDrawIndexed(render_pass, ARRAY_COUNT(index_data), 1, 0, 0, 0);
        
        wgpuRenderPassEncoderEnd(render_pass);
        wgpuRenderPassEncoderRelease(render_pass);
        
        
        WGPUCommandBufferDescriptor cmd_buffer_desc = {0};
        cmd_buffer_desc.nextInChain = NULL;
        cmd_buffer_desc.label = (WGPUStringView){ 
            .data = "Command Buffer",
            .length = strlen("Command Buffer")
        };
        WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
        wgpuCommandEncoderRelease(encoder);
        
        LOG("Submitting command...");
        wgpuQueueSubmit(wgpu.queue, 1, &cmd);
        wgpuCommandBufferRelease(cmd);
        LOG("Command submitted.");
        
        
        wgpuTextureViewRelease(target_view);
        wgpuSurfacePresent(wgpu.surface);
        
        wgpuDeviceTick(wgpu.device);
    }
    
    wgpuBufferRelease(vertex_buffer);
    wgpu_free(&wgpu);
    
    return 0;
}