
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

internal void
set_default_limits(WGPULimits *limits) {
    limits->maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
    limits->maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
    limits->minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
    limits->minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
    limits->maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
}

internal WGPURequiredLimits 
get_required_limits(WGPUAdapter adapter) 
{
    WGPUSupportedLimits supported_limits = {0};
    supported_limits.nextInChain = NULL;
    wgpuAdapterGetLimits(adapter, &supported_limits);
    
    WGPURequiredLimits required_limits = {0};
    set_default_limits(&required_limits.limits);
    
    required_limits.limits.maxVertexAttributes = 2;
    required_limits.limits.maxVertexBuffers = 1;
    required_limits.limits.maxBufferSize = 6 * 5 * sizeof(f32);
    required_limits.limits.maxVertexBufferArrayStride = 5 * sizeof(f32);
    required_limits.limits.maxInterStageShaderComponents = 3;
    
    required_limits.limits.minUniformBufferOffsetAlignment = supported_limits.limits.minUniformBufferOffsetAlignment;
    required_limits.limits.minStorageBufferOffsetAlignment = supported_limits.limits.minStorageBufferOffsetAlignment;
    
    return required_limits;
}

internal WGPUTextureView
get_next_surface_texture_view(WGPUSurface surface) 
{
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(surface, &surface_texture);
    
    if (surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_Success)
        return NULL;
    
    WGPUTextureViewDescriptor view_desc = {0};
    view_desc.nextInChain = NULL;
    view_desc.label = (WGPUStringView){ 
        .data = "Surface Texture View",
        .length = strlen("Surface Texture View")
    };
    view_desc.format = wgpuTextureGetFormat(surface_texture.texture);
    view_desc.dimension = WGPUTextureViewDimension_2D;
    view_desc.baseMipLevel = 0;
    view_desc.mipLevelCount = 1;
    view_desc.baseArrayLayer = 0;
    view_desc.arrayLayerCount = 1;
    view_desc.aspect = WGPUTextureAspect_All;
    
    WGPUTextureView texture_view = wgpuTextureCreateView(surface_texture.texture, &view_desc);
    
    return texture_view;
}

internal WGPUTextureView
get_texture_view(WGPUTexture texture, texture_info_t info)
{
    WGPUTextureViewDescriptor desc = {0};
    desc.aspect = WGPUTextureAspect_All;
    desc.baseArrayLayer = 0;
    desc.arrayLayerCount = 1;
    desc.baseMipLevel = 0;
    desc.mipLevelCount = info.mip_level_count;
    desc.dimension = get_wgpu_texture_format(info.dim_type);
    desc.format = get_wgpu_texture_format(info.format);
    
    return wgpuTextureCreateView(texture, &desc);
}

internal void
init_wgpu(wgpu_renderer_t *wgpu, GLFWwindow *window)
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
    
    // NOTE(ajeej): Get WebGPU Adapter
    
    WGPUSurface surface = glfwCreateWindowWGPUSurface(instance, window);
    
    WGPURequestAdapterOptions adapter_options = {0};
    adapter_options.nextInChain = NULL;
    adapter_options.powerPreference = WGPUPowerPreference_HighPerformance;
    adapter_options.compatibleSurface = surface;
    
    WGPUAdapter adapter = request_adapter(instance, &adapter_options);
    
    log_adapter_limits(adapter);
    //log_adapter_features(adapter);
    log_adapter_info(adapter);
    
    
    // NOTE(ajeej):  Get WebGPU Device
    
    WGPURequiredLimits required_limits = get_required_limits(adapter);
    WGPUDeviceDescriptor device_desc = {0};
    device_desc.nextInChain = NULL;
    device_desc.label = make_wgpu_str("WebGPU Device");
    device_desc.requiredFeatureCount = 0;
    device_desc.requiredLimits = &required_limits;
    device_desc.defaultQueue.nextInChain = NULL;
    device_desc.defaultQueue.label = make_wgpu_str("Default Queue");
    device_desc.deviceLostCallbackInfo = (WGPUDeviceLostCallbackInfo) { 
        .callback = device_lost_callback 
    };
    device_desc.uncapturedErrorCallbackInfo = (WGPUUncapturedErrorCallbackInfo){
        .callback = uncaptured_error_callback
    };
    
    WGPUDevice device = request_device(adapter, &device_desc);
    
    //log_device_features(device);
    log_device_limits(device);
    
    WGPUSupportedLimits device_limits = {0};
    device_limits.nextInChain = NULL;
    ASSERT_LOG(wgpuDeviceGetLimits(device, &device_limits) == WGPUStatus_Success, "Failed to aquire device limits.");
    wgpu->device_limits = device_limits.limits;
    
    
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    
    
    WGPUSurfaceConfiguration config = {0};
    config.nextInChain = NULL;
    config.width = 640;
    config.height = 480;
    
    WGPUSurfaceCapabilities capabilities = {0};
    wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);
    ASSERT_LOG(capabilities.formatCount > 0, "No capatable formats for surface!");
    
    wgpu->surface_format = capabilities.formats[0];
    config.format = capabilities.formats[0];
    config.viewFormatCount = 0;
    config.viewFormats = NULL;
    
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.device = device;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;
    
    wgpuSurfaceConfigure(surface, &config);
    
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    wgpuAdapterRelease(adapter);
    
    wgpu->instance = instance;
    wgpu->surface = surface;
    wgpu->device = device;
    wgpu->queue = queue;
}

internal void
free_wgpu(wgpu_renderer_t *wgpu)
{
    if (wgpu->bind_groups)
    {
        for (u32 i = 0; i < wgpu->bg_count; i++)
            wgpuBindGroupRelease(wgpu->bind_groups[i]);
        
        free(wgpu->bind_groups);
        wgpu->bind_groups = NULL;
    }
    
    if (wgpu->render_pipelines)
    {
        for (u32 i = 0; i < wgpu->render_pipeline_count; i++)
            wgpuRenderPipelineRelease(wgpu->render_pipelines[i]);
        
        free(wgpu->render_pipelines);
        wgpu->render_pipelines = NULL;
    }
    
    if (wgpu->compute_pipelines)
    {
        for (u32 i = 0; i < wgpu->compute_pipeline_count; i++)
            wgpuComputePipelineRelease(wgpu->compute_pipelines[i]);
        
        free(wgpu->compute_pipelines);
        wgpu->compute_pipelines = NULL;
    }
    
    wgpuSurfaceRelease(wgpu->surface);
    wgpuDeviceRelease(wgpu->device);
    wgpuInstanceRelease(wgpu->instance);
}

internal WGPUVertexBufferLayout *
get_vertex_buffer_layouts(render_pipeline_t *pipeline, vertex_buffer_layout_t *vb_layouts, u32 vb_count)
{
    u32 vb_layout_count = vb_count;
    WGPUVertexBufferLayout *vertex_buffer_layouts = (WGPUVertexBufferLayout *)malloc(vb_layout_count*sizeof(*vertex_buffer_layouts));
    
    vertex_buffer_layout_t *layout;
    for (u32 vb_idx = 0; vb_idx < vb_layout_count; vb_idx++)
    {
        layout = vb_layouts + pipeline->vb_layout_ids[vb_idx];
        
        attribute_t *attrib;
        u32 attrib_count = layout->attrib_count;
        WGPUVertexAttribute *va = (WGPUVertexAttribute *)malloc(sizeof(*va)*attrib_count);
        memset(va, 0, sizeof(*va)*attrib_count);
        for (u32 a_idx = 0; a_idx < attrib_count; a_idx++)
        {
            attrib = layout->attribs + a_idx;
            
            va[a_idx].shaderLocation = attrib->loc;
            va[a_idx].format = get_wgpu_vertex_format(attrib->type);
            va[a_idx].offset = attrib->offset;
        }
        
        vertex_buffer_layouts[vb_idx] = (WGPUVertexBufferLayout) {
            .attributeCount = attrib_count,
            .attributes = va,
            .arrayStride = layout->stride,
            .stepMode = get_wgpu_step_mode(layout->mode)
        };
    }
    
    return vertex_buffer_layouts;
}

internal void
release_vertex_buffer_layouts(WGPUVertexBufferLayout *vb_layouts, u32 vb_count)
{
    if (vb_layouts == NULL)
        return;
    
    for (u32 i = 0; i < vb_count; i++)
    {
        if (vb_layouts[i].attributes == NULL)
            continue;
        
        free((WGPUVertexAttribute *)vb_layouts[i].attributes);
    }
    
    free(vb_layouts);
}

internal void
set_bind_group_layout_entry(renderer_t *rb, WGPUBindGroupLayoutEntry *entry, bind_layout_t *bind)
{
    switch(bind->type)
    {
        case BINDING_TYPE_BUFFER: {
            buffer_bind_layout_t b_layout = bind->buffer_layout;
            
            entry->buffer.type = get_wgpu_buffer_type(b_layout.buffer_type);
            
            if (b_layout.has_dynamic_offset)
                entry->buffer.minBindingSize = b_layout.aligned_size;
            else
                entry->buffer.minBindingSize = b_layout.aligned_size*b_layout.count;
            
            entry->buffer.hasDynamicOffset = b_layout.has_dynamic_offset;
        } break;
        
        case BINDING_TYPE_TEXTURE: {
            texture_bind_layout_t t_layout = bind->texture_layout;
            texture_info_t *info = rb->textures + t_layout.id;
            
            entry->texture.sampleType = get_wgpu_texture_sample_type(info->format);
            entry->texture.viewDimension = get_wgpu_texture_view_dim(info->dim_type);
        } break;
        
        case BINDING_TYPE_STORAGE_TEXTURE: {
            storage_texture_bind_layout_t st_layout = bind->storage_texture_layout;
            texture_info_t *info = rb->textures + st_layout.id;
            
            entry->storageTexture.access = get_wgpu_texture_access(st_layout.access);
            entry->storageTexture.format = get_wgpu_texture_format(info->format);
            entry->storageTexture.viewDimension = get_wgpu_texture_view_dim(info->dim_type);
        } break;
        
        case BINDING_TYPE_SAMPLER: {
            sampler_bind_layout_t s_layout = bind->sampler_layout;
            
            entry->sampler.type = get_wgpu_sampler_type(s_layout.type);
        } break;
    }
}

internal WGPUBindGroupLayoutEntry *
get_bind_group_layout_entries(renderer_t *rb, bind_layout_t *bindings, u32 count)
{
    WGPUBindGroupLayoutEntry *binding_layouts = (WGPUBindGroupLayoutEntry *)malloc(sizeof(*binding_layouts)*count);
    memset(binding_layouts, 0, sizeof(*binding_layouts)*count);
    
    WGPUBindGroupLayoutEntry *entry;
    bind_layout_t *bind;
    for (u32 b_idx = 0; b_idx < count; b_idx++)
    {
        entry = binding_layouts + b_idx;
        bind = bindings + b_idx;
        
        entry->binding = bind->binding;
        entry->visibility = get_wgpu_shader_visibility(bind->visibility);
        
        set_bind_group_layout_entry(rb, entry, bind);
    }
    
    return binding_layouts;
}

internal WGPUBindGroupLayout *
get_bind_group_layouts(renderer_t *rb, wgpu_renderer_t *wgpu, 
                       bind_group_layout_t *layouts, u32 count)
{
    WGPUBindGroupLayout *res = (WGPUBindGroupLayout *)malloc(sizeof(*res)*count);
    
    u32 bind_count;
    bind_group_layout_t *layout;
    WGPUBindGroupLayoutDescriptor desc = {0};
    for (u32 bg_idx = 0; bg_idx < count; bg_idx++)
    {
        layout = layouts + bg_idx;
        bind_count = layout->count;
        
        WGPUBindGroupLayoutEntry *binding_layouts = get_bind_group_layout_entries(rb, layout->binds, layout->count);
        
        desc.nextInChain = NULL;
        desc.entryCount = bind_count;
        desc.entries = binding_layouts;
        
        res[bg_idx] = wgpuDeviceCreateBindGroupLayout(wgpu->device, &desc);
        
        free(binding_layouts);
    }
    
    return res;
}

internal void
release_bind_group_layouts(WGPUBindGroupLayout *bg_layouts, u32 bg_layout_count)
{
    if (bg_layouts == NULL);
    
    for (u32 i = 0; i < bg_layout_count; i++)
        wgpuBindGroupLayoutRelease(bg_layouts[i]);
    
    free(bg_layouts);
}

internal void
set_bind_group_entry(renderer_t *rb, wgpu_renderer_t *wgpu, 
                     WGPUBindGroupEntry *entry, bind_layout_t *bind)
{
    switch(bind->type)
    {
        case BINDING_TYPE_BUFFER: {
            buffer_bind_layout_t b_layout = bind->buffer_layout;
            
            entry->buffer = wgpu->buffers[b_layout.buffer_id];
            entry->offset = b_layout.offset;
            if (b_layout.has_dynamic_offset)
                entry->size = b_layout.aligned_size;
            else
                entry->size = b_layout.aligned_size*b_layout.count;
        } break;
        
        case BINDING_TYPE_TEXTURE: {
            texture_bind_layout_t t_layout = bind->texture_layout;
            texture_info_t info = rb->textures[t_layout.id];
            
            entry->textureView = get_texture_view(wgpu->textures[t_layout.id], info);
        } break;
        
        case BINDING_TYPE_STORAGE_TEXTURE: {
            storage_texture_bind_layout_t st_layout = bind->storage_texture_layout;
            texture_info_t info = rb->textures[st_layout.id];
            
            entry->textureView = get_texture_view(wgpu->textures[st_layout.id], info);;
        } break;
        
        case BINDING_TYPE_SAMPLER: {
            sampler_bind_layout_t s_layout = bind->sampler_layout;
            
            entry->sampler = wgpu->samplers[s_layout.id];
        } break;
    }
}

internal WGPUBindGroupEntry *
get_bind_group_entries(renderer_t *rb, wgpu_renderer_t *wgpu, 
                       bind_layout_t *bindings, u32 count)
{
    WGPUBindGroupEntry *entries = (WGPUBindGroupEntry *)malloc(sizeof(*entries)*count);
    memset(entries, 0, sizeof(*entries)*count);
    
    bind_layout_t *bind;
    WGPUBindGroupEntry *entry;
    for (u32 b_idx = 0; b_idx < count; b_idx++)
    {
        bind = bindings + b_idx;
        entry = entries + b_idx;
        
        entry->nextInChain = NULL;
        entry->binding = bind->binding;
        
        set_bind_group_entry(rb, wgpu, entry, bind);
    }
    
    return entries;
}

internal WGPUBindGroup *
get_bind_groups(renderer_t *rb, wgpu_renderer_t *wgpu,
                bind_group_layout_t *layouts, WGPUBindGroupLayout *bg_layouts,
                u32 count)
{
    WGPUBindGroup *bind_group = (WGPUBindGroup *)malloc(sizeof(*bind_group)*count);
    
    u32 bind_count;
    bind_group_layout_t *layout;
    for (u32 bg_idx = 0; bg_idx < count; bg_idx++)
    {
        layout = layouts + bg_idx;
        
        WGPUBindGroupEntry *bind_group_entries = get_bind_group_entries(rb, wgpu, layout->binds, layout->count);
        
        WGPUBindGroupDescriptor desc = {0};
        desc.nextInChain = NULL;
        desc.layout = bg_layouts[bg_idx];
        desc.entryCount = layout->count;
        desc.entries = bind_group_entries;
        
        bind_group[bg_idx] =  wgpuDeviceCreateBindGroup(wgpu->device, &desc);
        
        free(bind_group_entries);
    }
    
    return bind_group;
}

internal void
release_bind_groups(WGPUBindGroup *bind_groups, u32 bg_count)
{
    if (bind_groups == NULL)
        return;
    
    for (u32 i = 0; i < bg_count; i++)
        wgpuBindGroupRelease(bind_groups[i]);
    
    free(bind_groups);
}

internal WGPURenderPipeline
create_render_pipeline(renderer_t *rb,
                       wgpu_renderer_t *wgpu, 
                       render_pipeline_t *pipeline, 
                       vertex_buffer_layout_t *vb_layouts, 
                       WGPUBindGroupLayout *bind_group_layouts,
                       u32 bg_count,
                       WGPUShaderModule module)
{
    WGPURenderPipeline result;
    WGPURenderPipelineDescriptor desc = {0};
    
    u32 vb_layout_count = pipeline->vb_count;
    WGPUVertexBufferLayout *vertex_buffer_layouts = get_vertex_buffer_layouts(pipeline, vb_layouts, vb_layout_count);
    
    u32 p_bg_count = pipeline->bg_count;
    WGPUBindGroupLayout *p_bind_group_layouts = (WGPUBindGroupLayout *)malloc(sizeof(*p_bind_group_layouts)*p_bg_count);
    
    for (u32 bg_idx = 0; bg_idx < p_bg_count; bg_idx++)
        p_bind_group_layouts[bg_idx] = bind_group_layouts[pipeline->bg_layout_ids[bg_idx]];
    
    WGPUPipelineLayoutDescriptor layout_desc = {0};
    layout_desc.nextInChain = NULL;
    layout_desc.bindGroupLayoutCount = p_bg_count;
    layout_desc.bindGroupLayouts = p_bind_group_layouts;
    
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(wgpu->device, &layout_desc);
    
    free(p_bind_group_layouts);
    
    desc.layout = layout;
    
    desc.vertex = (WGPUVertexState){
        .bufferCount = vb_layout_count,
        .buffers = vertex_buffer_layouts,
        .module = module,
        .entryPoint = make_wgpu_str("vs_main"),
        .constantCount = 0,
        .constants = NULL
    };
    
    desc.primitive = (WGPUPrimitiveState) {
        .topology = WGPUPrimitiveTopology_TriangleList,
        .stripIndexFormat = WGPUIndexFormat_Undefined,
        .frontFace = WGPUFrontFace_CCW,
        .cullMode = WGPUCullMode_None
    };
    
    desc.fragment = &(WGPUFragmentState) {
        .module = module,
        .entryPoint = make_wgpu_str("fs_main"),
        .constantCount = 0,
        .constants = NULL,
        .targetCount = 1,
        .targets = &(WGPUColorTargetState) {
            .format =  get_wgpu_texture_format(rb->textures[pipeline->fb_id].format),
            .blend = &(WGPUBlendState) {
                .color = get_wgpu_blend_comp(pipeline->color_blend),
                .alpha = get_wgpu_blend_comp(pipeline->alpha_blend),
            },
            .writeMask = (pipeline->fb_id != 0) ? WGPUColorWriteMask_All : WGPUColorWriteMask_None,
        }
    };
    
    desc.depthStencil = NULL;
    
    desc.multisample = (WGPUMultisampleState) {
        .count = 1,
        .mask = ~0u,
        .alphaToCoverageEnabled = 0
    };
    
    result = wgpuDeviceCreateRenderPipeline(wgpu->device, &desc);
    
    release_vertex_buffer_layouts(vertex_buffer_layouts, vb_layout_count);
    
    wgpuPipelineLayoutRelease(layout);
    
    return result;
}

internal WGPUComputePipeline
create_compute_pipeline(wgpu_renderer_t *wgpu, 
                        compute_pipeline_t *pipeline, 
                        WGPUBindGroupLayout *bind_group_layouts,
                        u32 bg_count,
                        WGPUShaderModule module)
{
    WGPUComputePipeline result;
    
    u32 p_bg_count = pipeline->bg_count;
    WGPUBindGroupLayout *p_bind_group_layouts = (WGPUBindGroupLayout *)malloc(sizeof(*p_bind_group_layouts)*p_bg_count);
    
    for (u32 bg_idx = 0; bg_idx < p_bg_count; bg_idx++)
        p_bind_group_layouts[bg_idx] = bind_group_layouts[pipeline->bg_layout_ids[bg_idx]];
    
    WGPUPipelineLayoutDescriptor layout_desc = {0};
    layout_desc.nextInChain = NULL;
    layout_desc.bindGroupLayoutCount = p_bg_count;
    layout_desc.bindGroupLayouts = p_bind_group_layouts;
    
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(wgpu->device, &layout_desc);
    
    WGPUComputePipelineDescriptor desc = {0};
    desc.compute.entryPoint = make_wgpu_str("cs_main");
    desc.compute.module = module;
    desc.layout = layout;
    
    result = wgpuDeviceCreateComputePipeline(wgpu->device, &desc);
    
    wgpuPipelineLayoutRelease(layout);
    
    return result;
}

internal WGPUBuffer
create_buffer(wgpu_renderer_t *wgpu, u64 size, u32 usage)
{
    WGPUBuffer buffer;
    
    WGPUBufferDescriptor buffer_desc = {0};
    buffer_desc.nextInChain = NULL;
    buffer_desc.size = size;
    buffer_desc.usage = get_wgpu_buffer_usage(usage);
    buffer_desc.mappedAtCreation = 0;
    
    buffer = wgpuDeviceCreateBuffer(wgpu->device, &buffer_desc);
    
    return buffer;
}

internal WGPUBuffer *
create_vertex_buffers(wgpu_renderer_t *wgpu, vertex_buffer_layout_t *vb_layouts, u32 count)
{
    WGPUBuffer *vb = (WGPUBuffer *)malloc(sizeof(*vb)*count);
    
    vertex_buffer_data_t vb_data;
    vertex_buffer_layout_t *layout;
    for (u32 i = 0; i < count; i++)
    {
        layout = vb_layouts + i;
        vb_data = layout->data;
        
        vb[i] = create_buffer(wgpu, vb_data.count * vb_data.el_size,
                              BUFFER_FLAG_COPY_DST | BUFFER_FLAG_VERTEX);
        wgpuQueueWriteBuffer(wgpu->queue, vb[i], 0, vb_data.data, vb_data.count * vb_data.el_size);
    }
    
    return vb;
}

// TODO(ajeej): this concept of dynamic uniforms require some modifications
//              it could be possible to pass in an array of struct and threat
//              it as multiple dynamic uniforms by modifying the offset
//              whilst keeping the same stride, this could be useful in creating
//              a monolithic uniform storage structure
//              More thought has to be put int the customiability of this renderer
//              I would like to give the application to be able to define the sort
//              of data that goes in and out of it; however, most of this information
//              particularly the layout is backed into the renderer struction
//              
//              A working concept for future refactorings will to have the user define
//              the layout of date withing the render, allowing for a default setting
//              when no custom layout is passed in
//              Another useful detail would be the parsing of shaders to determine the
//              bindings of certain attributes and uniforms. so the setting wouldn't be
//              reliant on the user. This would minimize errors from the user end, since
//              the shader will only compile if it is written properly, which would minimize
//              checks in the renderer.

internal WGPUBuffer *
create_buffers(wgpu_renderer_t *wgpu, buffer_info_t *info, u32 count)
{
    WGPUBuffer *res = (WGPUBuffer *)malloc(sizeof(*res)*count);
    
    for (u32 b_idx = 0; b_idx < count; b_idx++)
        res[b_idx] = create_buffer(wgpu, info[b_idx].size, info[b_idx].usage);
    
    return res;
}

internal WGPUTexture
create_texture(wgpu_renderer_t *wgpu, texture_info_t info)
{
    WGPUTextureDescriptor desc = {0};
    desc.nextInChain = NULL;
    desc.dimension = get_wgpu_texture_dim(info.dim_type);
    
    memcpy(&desc.size, info.size, sizeof(*info.size)*3);
    
    desc.mipLevelCount = info.mip_level_count;
    desc.sampleCount = info.sample_count;
    desc.format = get_wgpu_texture_format(info.format);
    desc.usage = get_wgpu_texture_usage(info.usage);
    
    return wgpuDeviceCreateTexture(wgpu->device, &desc);
}

internal WGPUSampler
create_sampler(wgpu_renderer_t *wgpu, sampler_info_t info)
{
    WGPUSamplerDescriptor desc = {0};
    desc.nextInChain = NULL;
    desc.addressModeU = get_wgpu_address_mode(info.am_u);
    desc.addressModeV = get_wgpu_address_mode(info.am_v);
    desc.addressModeW = get_wgpu_address_mode(info.am_w);
    
    desc.magFilter = get_wgpu_filter_mode(info.mag_filter);
    desc.minFilter = get_wgpu_filter_mode(info.min_filter);
    
    desc.mipmapFilter = get_wgpu_mipmap_filter_mode(info.mipmap_filter);
    
    desc.lodMinClamp = info.lod_min_clamp;
    desc.lodMaxClamp = info.lod_max_clamp;
    
    desc.compare = WGPUCompareFunction_Undefined;
    desc.maxAnisotropy = info.max_anisotropy;
    
    return wgpuDeviceCreateSampler(wgpu->device, &desc);
}

internal void
write_to_texture(wgpu_renderer_t *wgpu, WGPUTexture texture, texture_info_t info)
{
    WGPUImageCopyTexture dst;
    dst.texture = texture;
    
    // TODO(ajeej): dont hard code these values if they are useful
    dst.mipLevel = 0;
    dst.origin = (WGPUOrigin3D) { 0, 0, 0 };
    dst.aspect = WGPUTextureAspect_All;
    
    u32 bytes_per_pixel = get_wgpu_bytes_per_pixel(info.format);
    
    WGPUTextureDataLayout src;
    src.bytesPerRow =  bytes_per_pixel * info.size[0];
    src.rowsPerImage = info.size[1];
    
    u64 size = bytes_per_pixel * info.size[0] * info.size[1];
    wgpuQueueWriteTexture(wgpu->queue, &dst, info.data, size, 
                          &src, (WGPUExtent3D *)info.size);
}

internal WGPUTexture *
create_textures(wgpu_renderer_t *wgpu, texture_info_t *info, u32 count)
{
    WGPUTexture *res = (WGPUTexture *)malloc(sizeof(*res)*count);
    
    // NOTE(ajeej): the first texture will contains
    //              surface texture
    for (u32 t_idx = 0; t_idx < count; t_idx++) {
        if (t_idx == 1) continue;
        res[t_idx] = create_texture(wgpu, info[t_idx]);
        write_to_texture(wgpu, res[t_idx], info[t_idx]);
    }
    
    return res;
}

internal WGPUSampler *
create_samplers(wgpu_renderer_t *wgpu, sampler_info_t *infos, u32 count)
{
    WGPUSampler *res = (WGPUSampler *)malloc(sizeof(*res)*count);
    
    for (u32 s_idx = 0; s_idx < count; s_idx++)
        res[s_idx] = create_sampler(wgpu, infos[s_idx]);
    
    return res;
}

internal void
init_render_pipelines(renderer_t *rb, wgpu_renderer_t *wgpu, 
                      render_pipeline_t *pipelines, u32 pipeline_count,
                      vertex_buffer_layout_t *vb_layouts,
                      WGPUBindGroupLayout *bind_group_layouts, u32 bg_count,
                      WGPUShaderModule *modules, u32 module_count)
{
    wgpu->render_pipeline_count = pipeline_count;
    wgpu->render_pipelines = (WGPURenderPipeline *)malloc(sizeof(*wgpu->render_pipelines)*pipeline_count);
    
    vertex_buffer_layout_t *layout;
    render_pipeline_t *pipeline;
    for (u32 p_idx = 0; p_idx < pipeline_count; p_idx++)
    {
        pipeline = pipelines + p_idx;
        
        wgpu->render_pipelines[p_idx] = create_render_pipeline(rb, wgpu, pipeline, vb_layouts, 
                                                               bind_group_layouts, bg_count,
                                                               modules[pipeline->shader_id]);
    }
}

internal void
init_compute_pipelines(wgpu_renderer_t *wgpu,
                       compute_pipeline_t *pipelines, u32 pipeline_count,
                       WGPUBindGroupLayout *bind_group_layouts, u32 bg_count,
                       WGPUShaderModule *modules, u32 module_count)
{
    wgpu->compute_pipeline_count = pipeline_count;
    wgpu->compute_pipelines = (WGPUComputePipeline *)malloc(sizeof(*wgpu->compute_pipelines)*pipeline_count);
    
    vertex_buffer_layout_t *layout;
    compute_pipeline_t *pipeline;
    for (u32 p_idx = 0; p_idx < pipeline_count; p_idx++)
    {
        pipeline = pipelines + p_idx;
        
        wgpu->compute_pipelines[p_idx] = create_compute_pipeline(wgpu, pipeline,
                                                                 bind_group_layouts, bg_count,
                                                                 modules[pipeline->shader_id]);
    }
}

internal WGPUShaderModule *
create_shader_modules(wgpu_renderer_t *wgpu, 
                      shader_asset_t *assets, u32 count)
{
    WGPUShaderModule *modules = malloc(sizeof(*modules)*count);
    
    shader_asset_t *asset;
    for (u32 i = 0; i < count; i++)
    {
        asset = assets+i;
        
        WGPUShaderModuleDescriptor shader_desc = {0};
        WGPUShaderSourceWGSL wgsl_source = {0};
        
        wgsl_source.chain.next = NULL;
        wgsl_source.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgsl_source.code = make_wgpu_str(asset->code);
        
        shader_desc.nextInChain = &wgsl_source.chain;
        
        modules[i] = wgpuDeviceCreateShaderModule(wgpu->device, &shader_desc);
        
    }
    
    return modules;
}

internal void
process_bind_group_layouts(wgpu_renderer_t *wgpu, bind_group_layout_t *layouts, u32 count, 
                           buffer_info_t *b_info, u32 b_count)
{
    u64 alignment = wgpu->device_limits.minUniformBufferOffsetAlignment;
    
    u64 *b_offsets = (u64 *)malloc(sizeof(*b_offsets)*b_count);
    memset(b_offsets, 0, sizeof(*b_offsets)*b_count);
    
    bind_group_layout_t *layout;
    bind_layout_t *bind;
    buffer_bind_layout_t *b_layout;
    for (u32 l_idx = 0; l_idx < count; l_idx++)
    {
        layout = layouts + l_idx;
        
        for (u32 b_idx = 0; b_idx < layout->count; b_idx++)
        {
            bind = layout->binds + b_idx;
            if (bind->type != BINDING_TYPE_BUFFER) continue;
            
            b_layout = &bind->buffer_layout;
            
            b_layout->offset = b_offsets[b_layout->buffer_id];
            b_layout->stride = align_offset(b_layout->size, alignment);
            b_layout->aligned_size = align_offset(b_layout->size, 16);
            
            if (b_layout->has_dynamic_offset)
                b_offsets[b_layout->buffer_id] = b_layout->offset + b_layout->stride*b_layout->count;
            else
                b_offsets[b_layout->buffer_id] = b_layout->offset + align_offset(b_layout->aligned_size*b_layout->count, alignment);
        }
    }
    
    for (u32 b_idx = 0; b_idx < b_count; b_idx++)
        b_info[b_idx].size = b_offsets[b_idx];
    
    free(b_offsets);;
}

internal void
write_buffer_bind_layout(wgpu_renderer_t *wgpu, buffer_bind_layout_t binding)
{
    WGPUBuffer buffer = wgpu->buffers[binding.buffer_id];
    
    u32 stride = (binding.has_dynamic_offset) ? binding.stride : binding.aligned_size;
    u8 *data = binding.data;
    
    if (binding.count == 1 || stride == binding.size)
    {
        wgpuQueueWriteBuffer(wgpu->queue, buffer, binding.offset,
                             data, binding.size*binding.count);
    }
    else
    {
        for (u32 i = 0; i < binding.count; i++)
        {
            wgpuQueueWriteBuffer(wgpu->queue, buffer,
                                 binding.offset + stride*i,
                                 data, binding.size);
            
            data += binding.size;
        }
    }
}

internal void
write_bind_layout_data(wgpu_renderer_t *wgpu, bind_group_layout_t *layouts, u32 count)
{
    bind_layout_t *bind;
    bind_group_layout_t *layout;
    for (u32 bg_idx = 0; bg_idx < count; bg_idx++)
    {
        layout = layouts + bg_idx;
        
        for (u32 b_idx = 0; b_idx < layout->count; b_idx++)
        {
            bind = layout->binds + b_idx;
            
            switch(bind->type)
            {
                case BINDING_TYPE_BUFFER: {
                    write_buffer_bind_layout(wgpu, bind->buffer_layout);
                } break;
                
                default: {
                    continue;//ASSERT_LOG(0, "Invalid binding type for writing data");
                } break;
            }
        }
    }
}

internal void
update_bind_layout_data(renderer_t *rb, wgpu_renderer_t *wgpu, bind_update_info_t *infos, u32 count)
{
    bind_layout_t *bind;
    bind_update_info_t *info;
    for (u32 i = 0; i < count; i++)
    {
        info = infos + i;
        
        bind = rb->bg_layouts[info->bg_id].binds + info->b_id;
        
        switch(bind->type)
        {
            case BINDING_TYPE_BUFFER: {
                write_buffer_bind_layout(wgpu, bind->buffer_layout);
            } break;
            
            default: {
                continue;//ASSERT_LOG(0, "Invalid binding type for writing data");
            } break;
        }
    }
}


internal void
submit_render_pipeline(renderer_t *rb, wgpu_renderer_t *wgpu, u32 p_id,
                       render_cmd_t *cmds, u32 cmd_count)
{
    render_pipeline_t *p_info = rb->render_pipelines + p_id;
    
    WGPURenderPassDescriptor render_pass_desc = {0};
    render_pass_desc.nextInChain = NULL;
    
    WGPURenderPassColorAttachment render_pass_color_attachment = {0};
    WGPUTextureView target_view;
    
    if (p_info->fb_id != 0)
    {
        target_view = get_texture_view(wgpu->textures[p_info->fb_id],
                                       rb->textures[p_info->fb_id]);
        render_pass_color_attachment.view = target_view;
        render_pass_color_attachment.resolveTarget = NULL;
        render_pass_color_attachment.loadOp = (p_id == 0) ? WGPULoadOp_Load : WGPULoadOp_Clear;
        render_pass_color_attachment.storeOp = WGPUStoreOp_Store;
        render_pass_color_attachment.clearValue = (WGPUColor){ p_info->clear.R, p_info->clear.G, p_info->clear.B};
        render_pass_color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &render_pass_color_attachment;
        render_pass_desc.depthStencilAttachment = NULL;
        render_pass_desc.timestampWrites = NULL;
    }
    else
    {
        target_view = get_texture_view(wgpu->textures[0], rb->textures[0]);
        
        render_pass_color_attachment.view = target_view;
        render_pass_color_attachment.resolveTarget = NULL;
        render_pass_color_attachment.loadOp = WGPULoadOp_Clear;
        render_pass_color_attachment.storeOp = WGPUStoreOp_Store;
        render_pass_color_attachment.clearValue = (WGPUColor){ 0, 0, 0};
        render_pass_color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &render_pass_color_attachment;
        render_pass_desc.depthStencilAttachment = NULL;
        render_pass_desc.timestampWrites = NULL;
    }
    
    WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(wgpu->encoder, &render_pass_desc);
    
    wgpuRenderPassEncoderSetPipeline(render_pass, wgpu->render_pipelines[p_id]);
    
    // NOTE(ajeej): Set Vertex Buffers and Index Buffer;
    
    WGPUBuffer *vertex_buffer;
    u32 vertex_buffer_count = p_info->vb_count;
    for (u32 v_idx = 0; v_idx < vertex_buffer_count; v_idx++) {
        vertex_buffer = wgpu->vertex_buffers + p_info->vb_layout_ids[v_idx];
        wgpuRenderPassEncoderSetVertexBuffer(render_pass, v_idx,
                                             *vertex_buffer, 0,
                                             wgpuBufferGetSize(*vertex_buffer));
    }
    
    wgpuRenderPassEncoderSetIndexBuffer(render_pass, wgpu->index_buffer,
                                        WGPUIndexFormat_Uint32, 0,
                                        wgpuBufferGetSize(wgpu->index_buffer));
    
    
    // NOTE(ajeej): Set bind groups that update every frame
    
    u32 bg_frame_count = get_stack_count(p_info->bg_frame_ids);
    for (u32 bg_idx = 0; bg_idx < bg_frame_count; bg_idx++)
    {
        u32 group_id  = p_info->bg_frame_ids[bg_idx];
        u32 idx = p_info->bg_layout_ids[group_id];
        wgpuRenderPassEncoderSetBindGroup(render_pass, group_id,
                                          wgpu->bind_groups[idx],
                                          0, NULL);
    }
    
    
    u32 strides[16];
    u32 stride_count = 0;
    
    render_cmd_t *cmd = NULL;
    for (u32 c_idx = 0; c_idx < cmd_count; c_idx++) {
        cmd = cmds + c_idx;
        
        
        // NOTE(ajeej): Set bind groups that update every draw call
        //              practically these are only dynamic in nature
        
        u32 bg_count = get_stack_count(p_info->bg_draw_ids);
        bind_group_layout_t *layout;
        bind_layout_t *bind;
        buffer_bind_layout_t *buffer_bind;
        for (u32 bg_idx = 0; bg_idx < bg_count; bg_idx++)
        {
            u32 group_id  = p_info->bg_draw_ids[bg_idx];
            u32 idx = p_info->bg_layout_ids[group_id];
            layout = rb->bg_layouts + idx;
            
            for (u32 b_idx = 0; b_idx < layout->count; b_idx++)
            {
                bind = layout->binds + b_idx;
                if (bind->type != BINDING_TYPE_BUFFER) continue;
                buffer_bind = &bind->buffer_layout;
                
                u32 id = *(u32 *)((u8 *)cmd + buffer_bind->id_offset);
                strides[stride_count++] = buffer_bind->stride * id;
            }
            
            wgpuRenderPassEncoderSetBindGroup(render_pass, group_id,
                                              wgpu->bind_groups[idx],
                                              stride_count, strides);
            
            stride_count = 0;
        }
        
        mesh_info_t m_info = rb->meshes[cmd->mesh_id];
        wgpuRenderPassEncoderDrawIndexed(render_pass, m_info.indices_count, 1,
                                         m_info.indices_idx, 0, 0);
    }
    
    wgpuRenderPassEncoderEnd(render_pass);
    
    wgpuRenderPassEncoderRelease(render_pass);
    
    wgpuTextureViewRelease(target_view);
}

internal void
submit_compute_pipeline(renderer_t *rb, wgpu_renderer_t *wgpu, u32 p_id)
{
    compute_pipeline_t *p_info = rb->compute_pipelines + p_id;
    
    WGPUComputePassDescriptor desc = {0};
    desc.nextInChain = NULL;
    desc.timestampWrites = NULL;
    WGPUComputePassEncoder compute_pass = wgpuCommandEncoderBeginComputePass(wgpu->encoder, &desc);
    
    wgpuComputePassEncoderSetPipeline(compute_pass, wgpu->compute_pipelines[p_id]);
    
    for (u32 bg_idx = 0; bg_idx < p_info->bg_count; bg_idx++)
    {
        u32 idx = p_info->bg_layout_ids[bg_idx];
        wgpuComputePassEncoderSetBindGroup(compute_pass, bg_idx,
                                           wgpu->bind_groups[idx],
                                           0, NULL);
    }
    
    wgpuComputePassEncoderDispatchWorkgroups(compute_pass,
                                             p_info->workgroup_x,
                                             p_info->workgroup_y,
                                             p_info->workgroup_z);
    
    wgpuComputePassEncoderEnd(compute_pass);
    wgpuComputePassEncoderRelease(compute_pass);
}

internal void
submit_texture_copy(wgpu_renderer_t *wgpu, 
                    u32 src, u32 src_offset_x, u32 src_offset_y,
                    u32 dst, u32 dst_offset_x, u32 dst_offset_y,
                    u32 width, u32 height)
{
    WGPUImageCopyTexture src_tex = {0};
    WGPUImageCopyTexture dst_tex = {0};
    WGPUExtent3D copy_size = {0};
    
    src_tex.texture = wgpu->textures[src];
    src_tex.mipLevel = 0;
    src_tex.origin = (WGPUOrigin3D){ src_offset_x, src_offset_y, 0 };
    src_tex.aspect = WGPUTextureAspect_All;
    
    dst_tex.texture = wgpu->textures[dst];
    dst_tex.mipLevel = 0;
    dst_tex.origin = (WGPUOrigin3D){ dst_offset_x, dst_offset_y, 0 };
    dst_tex.aspect = WGPUTextureAspect_All;
    
    copy_size.width = width;
    copy_size.height = height;
    copy_size.depthOrArrayLayers = 1;
    
    wgpuCommandEncoderCopyTextureToTexture(wgpu->encoder,
                                           &src_tex, &dst_tex, &copy_size);
}

INIT_RENDERER(init_renderer)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)malloc(sizeof(*wgpu));
    
    // NOTE(ajeej): Initialize WGPU
    init_wgpu(wgpu, window);
    
    rb->textures[1].format = get_texture_format_from_wgpu(wgpu->surface_format);
    
    // NOTE(ajeej): Compile shaders
    u32 module_count = get_stack_count(am->shader_assets);
    WGPUShaderModule *shader_modules = create_shader_modules(wgpu, am->shader_assets, 
                                                             module_count);
    
    
    // NOTE(ajeej): Process the bind group layouts
    wgpu->bg_count = get_stack_count(rb->bg_layouts);
    wgpu->b_count = get_stack_count(rb->buffers);
    process_bind_group_layouts(wgpu, rb->bg_layouts, wgpu->bg_count,
                               rb->buffers, wgpu->b_count);
    
    
    WGPUBindGroupLayout *bind_group_layouts = get_bind_group_layouts(rb, wgpu, rb->bg_layouts, wgpu->bg_count);
    
    
    // NOTE(ajeej): Create render pipelines
    u32 render_pipeline_count = get_stack_count(rb->render_pipelines);
    init_render_pipelines(rb, wgpu, rb->render_pipelines, render_pipeline_count,
                          rb->vb_layouts,
                          bind_group_layouts, wgpu->bg_count,
                          shader_modules, module_count);
    
    // NOTE(ajeej): Create compute pipelines
    u32 compute_pipeline_count = get_stack_count(rb->compute_pipelines);
    init_compute_pipelines(wgpu, rb->compute_pipelines, compute_pipeline_count,
                           bind_group_layouts, wgpu->bg_count,
                           shader_modules, module_count);
    
    
    // NOTE(ajeej): Create buffers
    wgpu->vb_count = get_stack_count(rb->vb_layouts);
    wgpu->vertex_buffers = create_vertex_buffers(wgpu, rb->vb_layouts, wgpu->vb_count);
    
    wgpu->index_buffer = create_buffer(wgpu, get_stack_size(rb->indices),
                                       WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
    wgpuQueueWriteBuffer(wgpu->queue, wgpu->index_buffer, 0, rb->indices, get_stack_size(rb->indices));
    
    wgpu->buffers = create_buffers(wgpu,rb->buffers, wgpu->b_count);
    
    wgpu->t_count = get_stack_count(rb->textures);
    wgpu->textures = create_textures(wgpu, rb->textures, wgpu->t_count);
    
    wgpu->s_count = get_stack_count(rb->samplers);
    wgpu->samplers = create_samplers(wgpu, rb->samplers, wgpu->s_count);
    
    // NOTE(ajeej): Create bind groups
    
    // TODO(ajeej): temp solution
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(wgpu->surface, &surface_texture);
    wgpu->textures[1] = surface_texture.texture;
    
    wgpu->bind_groups = get_bind_groups(rb, wgpu, rb->bg_layouts, bind_group_layouts, wgpu->bg_count);
    
    write_bind_layout_data(wgpu, rb->bg_layouts, wgpu->bg_count);
    
    rb->api = (void *)wgpu;
}

START_FRAME(start_frame)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)rb->api;
    
    WGPUCommandEncoderDescriptor encoder_desc = {0};
    encoder_desc.nextInChain = NULL;
    encoder_desc.label = make_wgpu_str("Command Encoder");
    
    /*wgpu->target_view = get_next_surface_texture_view(wgpu->surface);
    ASSERT_LOG(wgpu->target_view, "Failed to get target view!");*/
    
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(wgpu->surface, &surface_texture);
    wgpu->textures[1] = surface_texture.texture;
    
    wgpu->encoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &encoder_desc);
}

END_FRAME(end_frame)
{
    wgpu_renderer_t *wgpu = (wgpu_renderer_t *)rb->api;
    
    
    u32 update_count = get_stack_count(rb->bind_updates);
    update_bind_layout_data(rb, wgpu, rb->bind_updates, update_count);
    stack_clear(rb->bind_updates);
    
    
    u32 cmd_count = get_stack_count(rb->cmds);
    gpu_cmd_t *gpu_cmd;
    for (u32 c_idx = 0; c_idx < cmd_count; c_idx++)
    {
        gpu_cmd = rb->cmds + c_idx;
        
        switch(gpu_cmd->type)
        {
            case GPU_CMD_RENDER_PIPELINE_SUBMIT: {
                render_pipeline_submit_t submit = gpu_cmd->data.rp_submit;
                
                submit_render_pipeline(rb, wgpu, submit.id,
                                       rb->render_cmds + submit.cmd_start,
                                       submit.cmd_count);
            } break;
            
            case GPU_CMD_COMPUTE_PIPELINE_SUBMIT: {
                compute_pipeline_submit_t submit = gpu_cmd->data.cp_submit;
                
                submit_compute_pipeline(rb, wgpu, submit.id);
            } break;
            
            case GPU_CMD_TEXTURE_COPY: {
                texture_copy_t tex_copy = gpu_cmd->data.tex_copy;
                
                submit_texture_copy(wgpu, 
                                    tex_copy.src,
                                    tex_copy.src_offset_x, tex_copy.src_offset_y,
                                    tex_copy.dst,
                                    tex_copy.dst_offset_x, tex_copy.dst_offset_y,
                                    tex_copy.width, tex_copy.height);
            } break;
            
            default: {
                ASSERT_LOG(0, "Invalid GPU command type.");
            } break;
        }
    }
    
    stack_clear(rb->cmds);
    stack_clear(rb->render_cmds);
    
    WGPUCommandBufferDescriptor cmd_buffer_desc = {0};
    cmd_buffer_desc.nextInChain = NULL;
    cmd_buffer_desc.label = make_wgpu_str("Command Buffer");
    
    WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(wgpu->encoder, &cmd_buffer_desc);
    wgpuCommandEncoderRelease(wgpu->encoder);
    
    LOG("Submitting command...");
    wgpuQueueSubmit(wgpu->queue, 1, &cmd);
    wgpuCommandBufferRelease(cmd);
    LOG("Command submitted.");
    
    wgpuSurfacePresent(wgpu->surface);
    
    wgpuDeviceTick(wgpu->device);
}