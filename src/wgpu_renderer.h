

#ifndef WGPU_RENDERER_H
#define WGPU_RENDERER_H

typedef struct wgpu_renderer_t {
    WGPUInstance instance;
    WGPUDevice device;
    WGPUQueue queue;
    
    WGPULimits device_limits;
    
    WGPURenderPipeline *render_pipelines;
    u32 render_pipeline_count;
    
    WGPUComputePipeline *compute_pipelines;
    u32 compute_pipeline_count;
    
    WGPUBuffer *vertex_buffers;
    u32 vb_count;
    
    WGPUBuffer index_buffer;
    
    WGPUBuffer *buffers;
    u32 b_count;
    
    WGPUTexture *textures;
    u32 t_count;
    
    WGPUBindGroup *bind_groups;
    u32 bg_count;
    
    WGPUSampler *samplers;
    u32 s_count;
    
    WGPUSurface surface;
    WGPUTextureFormat surface_format;
    
    WGPUTextureView target_view;
    WGPUCommandEncoder encoder;
} wgpu_renderer_t;

#endif //WGPU_RENDERER_H
