
internal void
wgpu_init_pipeline(wgpu_renderer_t *wgpu, const char *shader_src)
{
    WGPUShaderModuleDescriptor shader_desc = {0};
    WGPUShaderSourceWGSL wgsl_source = {0};
    wgsl_source.chain.next = NULL;
    wgsl_source.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgsl_source.code = (WGPUStringView){ 
        .data = shader_src,
        .length = strlen(shader_src)
    };
    
    shader_desc.nextInChain = &wgsl_source.chain;
    
    WGPUShaderModule shader_module = wgpuDeviceCreateShaderModule(wgpu->device, &shader_desc);
    
    WGPUVertexAttribute vert_attrib[2];
    vert_attrib[0] = (WGPUVertexAttribute) {
        .shaderLocation = 0,
        .format = WGPUVertexFormat_Float32x2,
        .offset = 0,
    };
    vert_attrib[1] = (WGPUVertexAttribute) {
        .shaderLocation = 1,
        .format = WGPUVertexFormat_Float32x3,
        .offset = 2 * sizeof(f32),
    };
    
    WGPUVertexBufferLayout vertex_buffer_layout = {
        .attributeCount = 2,
        .attributes = vert_attrib,
        .arrayStride = 5 * sizeof(f32),
        .stepMode = WGPUVertexStepMode_Vertex
    };
    
    WGPURenderPipelineDescriptor pipeline_desc = {0};
    pipeline_desc.nextInChain = NULL;
    
    pipeline_desc.vertex = (WGPUVertexState){
        .bufferCount = 1,
        .buffers = &vertex_buffer_layout,
        .module = shader_module,
        .entryPoint =  (WGPUStringView){ 
            .data = "vs_main",
            .length = strlen("vs_main")
        },
        .constantCount = 0,
        .constants = NULL
    };
    
    pipeline_desc.primitive = (WGPUPrimitiveState) {
        .topology = WGPUPrimitiveTopology_TriangleList,
        .stripIndexFormat = WGPUIndexFormat_Undefined,
        .frontFace = WGPUFrontFace_CCW,
        .cullMode = WGPUCullMode_None
    };
    
    pipeline_desc.fragment = &(WGPUFragmentState) {
        .module = shader_module,
        .entryPoint =  (WGPUStringView){ 
            .data = "fs_main",
            .length = strlen("fs_main")
        },
        .constantCount = 0,
        .constants = NULL,
        .targetCount = 1,
        .targets = &(WGPUColorTargetState) {
            .format = wgpu->surface_format,
            .blend = &(WGPUBlendState) {
                .color = (WGPUBlendComponent) {
                    .srcFactor = WGPUBlendFactor_SrcAlpha,
                    .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                    .operation = WGPUBlendOperation_Add
                },
                .alpha = (WGPUBlendComponent) {
                    .srcFactor = WGPUBlendFactor_SrcAlpha,
                    .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                    .operation = WGPUBlendOperation_Add
                }
            },
            .writeMask = WGPUColorWriteMask_All
        }
    };
    
    pipeline_desc.depthStencil = NULL;
    
    pipeline_desc.multisample = (WGPUMultisampleState) {
        .count = 1,
        .mask = ~0u,
        .alphaToCoverageEnabled = 0
    };
    
    wgpu->pipeline = wgpuDeviceCreateRenderPipeline(wgpu->device, &pipeline_desc);
    
    wgpuShaderModuleRelease(shader_module);
}

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

internal void
wgpu_init(wgpu_renderer_t *wgpu, const char *shader_path)
{
    ASSERT_LOG(glfwInit(), "Could not initialize GLFW!");
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "WebGPU Test", NULL, NULL);
    ASSERT_LOG(window, "Could not open window!");
    
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
    device_desc.label = (WGPUStringView){ 
        .data = "WebGPU Device",
        .length = strlen("WebGPU Device")
    };
    device_desc.requiredFeatureCount = 0;
    device_desc.requiredLimits = &required_limits;
    device_desc.defaultQueue.nextInChain = NULL;
    device_desc.defaultQueue.label = (WGPUStringView){ 
        .data = "Default Queue",
        .length = strlen("Default Queue")
    };
    device_desc.deviceLostCallbackInfo = (WGPUDeviceLostCallbackInfo) { 
        .callback = device_lost_callback 
    };
    device_desc.uncapturedErrorCallbackInfo = (WGPUUncapturedErrorCallbackInfo){
        .callback = uncaptured_error_callback
    };
    
    WGPUDevice device = request_device(adapter, &device_desc);
    
    //log_device_features(device);
    log_device_limits(device);
    
    
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
    
    wgpu->window = window;
    wgpu->instance = instance;
    wgpu->surface = surface;
    wgpu->device = device;
    wgpu->queue = queue;
    
    const char *src = read_file(shader_path, NULL);
    wgpu_init_pipeline(wgpu, src);
}

internal void
wgpu_free(wgpu_renderer_t *wgpu)
{
    glfwDestroyWindow(wgpu->window);
    
    wgpuRenderPipelineRelease(wgpu->pipeline);
    wgpuSurfaceRelease(wgpu->surface);
    wgpuDeviceRelease(wgpu->device);
    wgpuInstanceRelease(wgpu->instance);
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
    view_desc.label = make_wgpu_str("Surface Texture View");
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
