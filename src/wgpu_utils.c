
#if __EMSCRIPTEN__

#define WGPUStatus_Success WGPUWaitStatus_Success

#define WGPUStringView const char *

internal const char *
get_wgpu_str(WGPUStringView str)
{
    return str;
}

internal WGPUStringView
make_wgpu_str(const char *str)
{
    return str;
}

internal WGPUStringView
make_wgpu_str_n(const char *str, u64 n)
{
    return str;
}

#else

internal const char *
get_wgpu_str(WGPUStringView str)
{
    return str.data;
}

internal WGPUStringView
make_wgpu_str(const char *str)
{
    return (WGPUStringView) {
        .data = str,
        .length = strlen(str)
    };
}

internal WGPUStringView
make_wgpu_str_n(const char *str, u64 n)
{
    return (WGPUStringView) {
        .data = str,
        .length = n
    };
}

#endif

typedef struct request_adapter_callback_data_t {
    WGPUAdapter adapter;
    b8 request_ended;
} request_adapter_callback_data_t;

typedef struct request_device_callback_data_t {
    WGPUDevice device;
    b8 request_ended;
} request_device_callback_data_t;

typedef struct buffer_map_callback_data_t {
    WGPUBuffer buffer;
    void *data;
    u32 size;
} buffer_map_callback_data_t;

#if __EMSCRIPTEN__

internal void
device_lost_callback(WGPUDeviceLostReason reason, char const *message, void *user_data)
{
    LOG("Device lost: reason");
    if (message) {
        LOG("(%s)", message);
    }
}

internal void
uncaptured_error_callback(WGPUErrorType type, char const *message, void *user_data)
{
    LOG("Uncaptured device error: type %d", type);
    if (message) {
        LOG("(%s)", message);
    }
}

#else

void request_adapter_callback(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                              char const *message, void *user_data)
{
    request_adapter_callback_data_t *data = (request_adapter_callback_data_t *)user_data;
    
    if (status == WGPURequestAdapterStatus_Success) {
        data->adapter = adapter;
    } else {
        LOG("Could not get WebGPU adapter: %s", message);
    }
    
    data->request_ended = 1;
}


void request_device_callback(WGPURequestDeviceStatus status, WGPUDevice device,
                             char const *message, void *user_data)
{
    request_device_callback_data_t *data = (request_device_callback_data_t *)user_data;
    
    if (status == WGPURequestDeviceStatus_Success) {
        data->device = device;
    } else {
        LOG("Could not get WebGPU device: %s", message);
    }
    
    data->request_ended = 1;
}

void buffer_map_callback(WGPUMapAsyncStatus status, void *user_data)
{
    buffer_map_callback_data_t *info = (buffer_map_callback_data_t *)user_data;
    
    if (status == WGPUBufferMapAsyncStatus_Success) {
        
        void const *data = wgpuBufferGetConstMappedRange(info->buffer,
                                                         0, info->size);
        
        if (data) {
            memcpy(info->data, data, info->size);
            
            free(info);
        }
        
        wgpuBufferUnmap(info->buffer);
    }
}

internal void
device_lost_callback(WGPUDevice const *device, WGPUDeviceLostReason reason, char const *message, void *user_data)
{
    LOG("Device lost: reason");
    if (message) {
        LOG("(%s)", message);
    }
}

internal void
uncaptured_error_callback(WGPUErrorType type, char const *message, void *user_data)
{
    LOG("Uncaptured device error: type %d", type);
    if (message) {
        LOG("(%s)", message);
    }
}

#endif

internal WGPUAdapter
request_adapter(WGPUInstance instance, WGPURequestAdapterOptions const *options, void *request_adapter_callback)
{
    request_adapter_callback_data_t rac_data = {0};
    
    LOG("request_adapter_callback: %p", request_adapter_callback);
    
    wgpuInstanceRequestAdapter(instance, options,
                               request_adapter_callback,
                               (void *)&rac_data);
    
#if __EMSCRIPTEN__
    while (!rac_data.request_ended) {
        LOG("Requesting adapter...");
        emscripten_sleep(200);
    }
#endif
    
    while(!rac_data.request_ended)
    {
        ;
    }
    
    ASSERT_LOG(rac_data.request_ended, "Failed to acquire adapter!");
    
    return rac_data.adapter;
}

internal void
log_adapter_limits(WGPUAdapter adapter)
{
#ifndef __EMSCRIPTEN__
    
    WGPUSupportedLimits supported_limits = {0};
    supported_limits.nextInChain = NULL;
    
    b8 success = wgpuAdapterGetLimits(adapter, &supported_limits) == WGPUStatus_Success;
    
    if (success)
    {
        LOG("Adapter limits:");
        LOG(" - maxTextureDimension1D: %d", supported_limits.limits.maxTextureDimension1D);
        LOG(" - maxTextureDimension2D: %d", supported_limits.limits.maxTextureDimension2D);
        LOG(" - maxTextureDimension3D: %d", supported_limits.limits.maxTextureDimension3D);
        LOG(" - maxTextureArrayLayers: %d", supported_limits.limits.maxTextureArrayLayers);
    }
    
#endif
}

internal void
log_adapter_features(WGPUAdapter adapter)
{
    WGPUFeatureName *features = NULL;
    size_t feature_count = wgpuAdapterEnumerateFeatures(adapter, NULL);
    
    features = (WGPUFeatureName *)malloc(sizeof(*features)*feature_count);
    
    wgpuAdapterEnumerateFeatures(adapter, features);
    
    LOG("Adapter features:");
    for (u32 i = 0; i < feature_count; i++) {
        LOG(" - %#x", features[i]);
    }
    
    free(features);
    features = NULL;
}

internal void
log_adapter_info(WGPUAdapter adapter)
{
    
    WGPUAdapterInfo info = {0};
    info.nextInChain = NULL;
    
    wgpuAdapterGetInfo(adapter, &info);
    
    LOG("Adapter properties:");
    LOG(" - vendorID: %d", info.vendorID);
    if (get_wgpu_str(info.vendor)) {
        LOG(" - vendorName: %s", get_wgpu_str(info.vendor));
    }
    if (get_wgpu_str(info.architecture)) {
        LOG(" - architecture: %s", get_wgpu_str(info.architecture));
    }
    LOG(" - deviceID: %d", info.deviceID);
    if (get_wgpu_str(info.device)) {
        LOG(" - name: %s", get_wgpu_str(info.device));
    }
    if (get_wgpu_str(info.description)) {
        LOG(" - driverDescription: %s", get_wgpu_str(info.description));
    }
    LOG(" - adapterType: %#x", info.adapterType);
    LOG(" - backendType: %#x", info.backendType);
}


internal void
log_device_features(WGPUDevice device)
{
    WGPUFeatureName *features = NULL;
    size_t feature_count = wgpuDeviceEnumerateFeatures(device, NULL);
    
    features = (WGPUFeatureName *)malloc(sizeof(*features)*feature_count);
    
    wgpuDeviceEnumerateFeatures(device, features);
    
    LOG("Device features:");
    for (u32 i = 0; i < feature_count; i++) {
        LOG(" - %#x", features[i]);
    }
    
    free(features);
    features = NULL;
}

internal void
log_device_limits(WGPUDevice device)
{
    WGPUSupportedLimits device_limits = {0};
    device_limits.nextInChain = NULL;
    
    b8 success = wgpuDeviceGetLimits(device, &device_limits) == WGPUStatus_Success;
    
    if (success) {
        LOG("Device limits:");
        LOG(" - maxTextureDimension1D: %d", device_limits.limits.maxTextureDimension1D);
        LOG(" - maxTextureDimension2D: %d", device_limits.limits.maxTextureDimension2D);
        LOG(" - maxTextureDimension3D: %d", device_limits.limits.maxTextureDimension3D);
        LOG(" - maxTextureArrayLayers: %d", device_limits.limits.maxTextureArrayLayers);
        LOG(" - maxBindGroups: %d", device_limits.limits.maxBindGroups);
        LOG(" - maxDynamicUniformBuffersPerPipelineLayout: %d", device_limits.limits.maxDynamicUniformBuffersPerPipelineLayout);
        LOG(" - maxDynamicStorageBuffersPerPipelineLayout: %d", device_limits.limits.maxDynamicStorageBuffersPerPipelineLayout);
        LOG(" - maxSampledTexturesPerShaderStage: %d", device_limits.limits.maxSampledTexturesPerShaderStage);
        LOG(" - maxSamplersPerShaderStage: %d", device_limits.limits.maxSamplersPerShaderStage);
        LOG(" - maxStorageBuffersPerShaderStage: %d", device_limits.limits.maxStorageBuffersPerShaderStage);
        LOG(" - maxStorageTexturesPerShaderStage: %d", device_limits.limits.maxStorageTexturesPerShaderStage);
        LOG(" - maxUniformBuffersPerShaderStage: %d", device_limits.limits.maxUniformBuffersPerShaderStage);
        LOG(" - maxUniformBufferBindingSize: %lld", device_limits.limits.maxUniformBufferBindingSize);
        LOG(" - maxStorageBufferBindingSize: %lld", device_limits.limits.maxStorageBufferBindingSize);
        LOG(" - minUniformBufferOffsetAlignment: %d", device_limits.limits.minUniformBufferOffsetAlignment);
        LOG(" - minStorageBufferOffsetAlignment: %d", device_limits.limits.minStorageBufferOffsetAlignment);
        LOG(" - maxVertexBuffers: %d", device_limits.limits.maxVertexBuffers);
        LOG(" - maxVertexAttributes: %d", device_limits.limits.maxVertexAttributes);
        LOG(" - maxVertexBufferArrayStride: %d", device_limits.limits.maxVertexBufferArrayStride);
        LOG(" - maxInterStageShaderComponents: %d", device_limits.limits.maxInterStageShaderComponents);
        LOG(" - maxComputeWorkgroupStorageSize: %d", device_limits.limits.maxComputeWorkgroupStorageSize);
        LOG(" - maxComputeInvocationsPerWorkgroup: %d", device_limits.limits.maxComputeInvocationsPerWorkgroup);
        LOG(" - maxComputeWorkgroupSizeX: %d", device_limits.limits.maxComputeWorkgroupSizeX);
        LOG(" - maxComputeWorkgroupSizeY: %d", device_limits.limits.maxComputeWorkgroupSizeY);
        LOG(" - maxComputeWorkgroupSizeZ: %d", device_limits.limits.maxComputeWorkgroupSizeZ);
        LOG(" - maxComputeWorkgroupsPerDimension: %d", device_limits.limits.maxComputeWorkgroupsPerDimension);
    }
}

internal WGPUDevice
request_device(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor,
               void *request_device_callback)
{
    request_device_callback_data_t rdc_data = {0};
    
    wgpuAdapterRequestDevice(adapter, descriptor,
                             request_device_callback,
                             (void *)&rdc_data);
    
#ifdef __EMSCRIPTEN__
    while (!rdc_data.request_ended) {
        LOG("Requesting device...");
        emscripten_sleep(100);
    }
#endif
    
    ASSERT_LOG(rdc_data.request_ended, "Failed to acquire device!");
    
    return rdc_data.device;
}



internal u32
get_wgpu_vertex_format(u32 type)
{
    switch(type)
    {
        case ATTRIBUTE_FORMAT_FLOAT32x2: {
            return WGPUVertexFormat_Float32x2;
        } break;
        
        case ATTRIBUTE_FORMAT_FLOAT32x3: {
            return WGPUVertexFormat_Float32x3;
        } break;
        
        case ATTRIBUTE_FORMAT_FLOAT32x4: {
            return WGPUVertexFormat_Float32x4;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid attribute format");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_step_mode(u32 mode)
{
    switch(mode)
    {
        case MODE_VERTEX: {
            return WGPUVertexStepMode_Vertex;
        } break;
        
        case MODE_INSTANCE: {
            return WGPUVertexStepMode_Instance;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid step mode");
        } break;
    }
    
    return WGPUVertexStepMode_Undefined;
}

internal u32
get_wgpu_shader_visibility(u32 visibility)
{
    u32 res = 0;
    if (visibility & SHADER_VISIBILITY_VERTEX)
        res |= WGPUShaderStage_Vertex;
    
    if (visibility & SHADER_VISIBILITY_FRAGMENT)
        res |= WGPUShaderStage_Fragment;
    
    if (visibility & SHADER_VISIBILITY_COMPUTE)
        res |= WGPUShaderStage_Compute;
    
    return res;
}

internal u32
get_wgpu_buffer_type(u32 type)
{
    switch(type)
    {
        case BUFFER_TYPE_UNIFORM: {
            return WGPUBufferBindingType_Uniform;
        } break;
        
        case BUFFER_TYPE_STORAGE: {
            return WGPUBufferBindingType_Storage;
        } break;
        
        case BUFFER_TYPE_READ_ONLY_STORAGE: {
            return WGPUBufferBindingType_ReadOnlyStorage;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid buffer type");
        } break;
    }
}

internal u32
get_wgpu_buffer_usage(u32 usage)
{
    u32 res = 0;
    
    if (usage & BUFFER_FLAG_MAP_READ)
        res |= WGPUBufferUsage_MapRead;
    
    if (usage & BUFFER_FLAG_MAP_WRITE)
        res |= WGPUBufferUsage_MapWrite;
    
    if (usage & BUFFER_FLAG_COPY_SRC)
        res |= WGPUBufferUsage_CopySrc;
    
    if (usage & BUFFER_FLAG_COPY_DST)
        res |= WGPUBufferUsage_CopyDst;
    
    if (usage & BUFFER_FLAG_INDEX)
        res |= WGPUBufferUsage_Index;
    
    if (usage & BUFFER_FLAG_VERTEX)
        res |= WGPUBufferUsage_Vertex;
    
    if (usage & BUFFER_FLAG_UNIFORM)
        res |= WGPUBufferUsage_Uniform;
    
    if (usage & BUFFER_FLAG_STORAGE)
        res |= WGPUBufferUsage_Storage;
    
    if (usage & BUFFER_FLAG_INDIRECT)
        res |= WGPUBufferUsage_Indirect;
    
    if (usage & BUFFER_FLAG_QUERY)
        res |= WGPUBufferUsage_QueryResolve;
    
    return res;
}

internal u32
get_wgpu_bytes_per_pixel(u32 format)
{
    switch(format)
    {
        case TEXTURE_FORMAT_R8U_NORM: 
        case TEXTURE_FORMAT_R8S_NORM:
        case TEXTURE_FORMAT_R8U_INT:
        case TEXTURE_FORMAT_R8S_INT: {
            return 1;
        } break;
        
        case TEXTURE_FORMAT_R16U_INT :
        case TEXTURE_FORMAT_R16S_INT :
        case TEXTURE_FORMAT_R16_FLOAT :
        case TEXTURE_FORMAT_RG8U_NORM :
        case TEXTURE_FORMAT_RG8S_NORM :
        case TEXTURE_FORMAT_RG8U_INT :
        case TEXTURE_FORMAT_RG8S_INT : {
            return 2;
        } break;
        
        case TEXTURE_FORMAT_R32U_INT :
        case TEXTURE_FORMAT_R32S_INT :
        case TEXTURE_FORMAT_R32_FLOAT :
        case TEXTURE_FORMAT_RG16U_INT :
        case TEXTURE_FORMAT_RG16S_INT :
        case TEXTURE_FORMAT_RG16_FLOAT :
        case TEXTURE_FORMAT_RGBA8U_NORM :
        case TEXTURE_FORMAT_RGBA8S_NORM :
        case TEXTURE_FORMAT_RGBA8U_INT :
        case TEXTURE_FORMAT_RGBA8S_INT : {
            return 4;
        } break;
        
        case TEXTURE_FORMAT_RG32U_INT :
        case TEXTURE_FORMAT_RG32S_INT :
        case TEXTURE_FORMAT_RG32_FLOAT :
        case TEXTURE_FORMAT_RGBA16U_INT :
        case TEXTURE_FORMAT_RGBA16S_INT :
        case TEXTURE_FORMAT_RGBA16_FLOAT : {
            return 8;
        } break;
        
        case TEXTURE_FORMAT_RGBA32U_INT :
        case TEXTURE_FORMAT_RGBA32S_INT :
        case TEXTURE_FORMAT_RGBA32_FLOAT : {
            return 16;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid Texture format.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_texture_usage(u32 usage)
{
    u32 res = 0;
    
    if (usage & TEXTURE_USAGE_COPY_SRC)
        res |= WGPUTextureUsage_CopySrc;
    if (usage & TEXTURE_USAGE_COPY_DST)
        res |= WGPUTextureUsage_CopyDst;
    if (usage & TEXTURE_USAGE_TEXTURE_BINDING)
        res |= WGPUTextureUsage_TextureBinding;
    if (usage & TEXTURE_USAGE_STORAGE_BINDING)
        res |= WGPUTextureUsage_StorageBinding;
    if (usage & TEXTURE_USAGE_RENDER_ATTACHMENT)
        res |= WGPUTextureUsage_RenderAttachment;
    
    return res;
}

internal u32
get_wgpu_texture_dim(u32 type)
{
    switch (type)
    {
        case TEXTURE_DIM_1D: {
            return WGPUTextureDimension_1D;
        } break;
        
        case TEXTURE_DIM_2D: {
            return WGPUTextureDimension_2D;
        } break;
        
        case TEXTURE_DIM_3D: {
            return WGPUTextureDimension_3D;
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_sampler_type(u32 type)
{
    switch (type)
    {
        case SAMPLER_TYPE_FILTERING: {
            return WGPUSamplerBindingType_Filtering;
        } break;
        
        case SAMPLER_TYPE_NONFILTERING: {
            return WGPUSamplerBindingType_NonFiltering;
        } break;
        
        case SAMPLER_TYPE_COMPARISON: {
            return WGPUSamplerBindingType_Comparison;
        } break;
    }
    
    return 0;
}

internal u32
get_texture_format_from_wgpu(u32 format)
{
    switch(format)
    {
        case WGPUTextureFormat_R8Unorm: {
            return TEXTURE_FORMAT_R8U_NORM;
        } break;
        
        case WGPUTextureFormat_R8Snorm: {
            return TEXTURE_FORMAT_R8S_NORM;
        } break;
        
        case WGPUTextureFormat_R8Uint: {
            return TEXTURE_FORMAT_R8U_INT;
        } break;
        
        case WGPUTextureFormat_R8Sint: {
            return TEXTURE_FORMAT_R8S_INT;
        } break;
        
        case  WGPUTextureFormat_R16Uint: {
            return TEXTURE_FORMAT_R16U_INT;
        } break;
        
        case WGPUTextureFormat_R16Sint: {
            return TEXTURE_FORMAT_R16S_INT;
        } break;
        
        case  WGPUTextureFormat_R16Float: {
            return TEXTURE_FORMAT_R16_FLOAT;
        } break;
        
        case WGPUTextureFormat_RG8Unorm: {
            return TEXTURE_FORMAT_RG8U_NORM;
        } break;
        
        case WGPUTextureFormat_RG8Snorm: {
            return TEXTURE_FORMAT_RG8S_NORM;
        } break;
        
        case  WGPUTextureFormat_RG8Uint: {
            return TEXTURE_FORMAT_RG8U_INT;
        } break;
        
        case WGPUTextureFormat_RG8Sint: {
            return TEXTURE_FORMAT_RG8S_INT;
        } break;
        
        case WGPUTextureFormat_R32Uint: {
            return TEXTURE_FORMAT_R32U_INT;
        } break;
        
        case WGPUTextureFormat_R32Sint: {
            return TEXTURE_FORMAT_R32S_INT;
        } break;
        
        case WGPUTextureFormat_R32Float: {
            return TEXTURE_FORMAT_R32_FLOAT;
        } break;
        
        case WGPUTextureFormat_RG16Uint: {
            return TEXTURE_FORMAT_RG16U_INT;
        } break;
        
        case WGPUTextureFormat_RG16Sint: {
            return TEXTURE_FORMAT_RG16S_INT;
        } break;
        
        case WGPUTextureFormat_RG16Float: {
            return TEXTURE_FORMAT_RG16_FLOAT;
        } break;
        
        case WGPUTextureFormat_RGBA8Unorm: {
            return TEXTURE_FORMAT_RGBA8U_NORM;
        } break;
        
        case WGPUTextureFormat_RGBA8Snorm: {
            return TEXTURE_FORMAT_RGBA8S_NORM;
        } break;
        
        case WGPUTextureFormat_RGBA8Uint: {
            return TEXTURE_FORMAT_RGBA8U_INT;
        } break;
        
        case WGPUTextureFormat_RGBA8Sint: {
            return TEXTURE_FORMAT_RGBA8S_INT;
        } break;
        
        case WGPUTextureFormat_RG32Uint: {
            return TEXTURE_FORMAT_RG32U_INT;
        } break;
        
        case WGPUTextureFormat_RG32Sint: {
            return TEXTURE_FORMAT_RG32S_INT;
        } break;
        
        case WGPUTextureFormat_RG32Float: {
            return TEXTURE_FORMAT_RG32_FLOAT;
        } break;
        
        case WGPUTextureFormat_RGBA16Uint: {
            return TEXTURE_FORMAT_RGBA16U_INT;
        } break;
        
        case WGPUTextureFormat_RGBA16Sint: {
            return TEXTURE_FORMAT_RGBA16S_INT;
        } break;
        
        case WGPUTextureFormat_RGBA16Float: {
            return TEXTURE_FORMAT_RGBA16_FLOAT;
        } break;
        
        case WGPUTextureFormat_RGBA32Uint: {
            return TEXTURE_FORMAT_RGBA32U_INT;
        } break;
        
        case WGPUTextureFormat_RGBA32Sint: {
            return TEXTURE_FORMAT_RGBA32S_INT;
        } break;
        
        case WGPUTextureFormat_RGBA32Float: {
            return TEXTURE_FORMAT_RGBA32_FLOAT;
        } break;
        
        case WGPUTextureFormat_BGRA8Unorm: {
            return TEXTURE_FORMAT_BGRA8U_NORM;
        } break;
        
        case WGPUTextureFormat_BGRA8UnormSrgb: {
            return TEXTURE_FORMAT_BGRA8U_NORM_SRGB;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid Texture format.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_texture_format(u32 format)
{
    switch(format)
    {
        case TEXTURE_FORMAT_R8U_NORM: {
            return WGPUTextureFormat_R8Unorm;
        } break;
        
        case TEXTURE_FORMAT_R8S_NORM: {
            return WGPUTextureFormat_R8Snorm;
        } break;
        
        case TEXTURE_FORMAT_R8U_INT: {
            return WGPUTextureFormat_R8Uint;
        } break;
        
        case TEXTURE_FORMAT_R8S_INT: {
            return WGPUTextureFormat_R8Sint;
        } break;
        
        case TEXTURE_FORMAT_R16U_INT : {
            return WGPUTextureFormat_R16Uint;
        } break;
        
        case TEXTURE_FORMAT_R16S_INT : {
            return WGPUTextureFormat_R16Sint;
        } break;
        
        case TEXTURE_FORMAT_R16_FLOAT : {
            return WGPUTextureFormat_R16Float;
        } break;
        
        case TEXTURE_FORMAT_RG8U_NORM : {
            return WGPUTextureFormat_RG8Unorm;
        } break;
        
        case TEXTURE_FORMAT_RG8S_NORM : {
            return WGPUTextureFormat_RG8Snorm;
        } break;
        
        case TEXTURE_FORMAT_RG8U_INT : {
            return WGPUTextureFormat_RG8Uint;
        } break;
        
        case TEXTURE_FORMAT_RG8S_INT : {
            return WGPUTextureFormat_RG8Sint;
        } break;
        
        case TEXTURE_FORMAT_R32U_INT : {
            return WGPUTextureFormat_R32Uint;
        } break;
        
        case TEXTURE_FORMAT_R32S_INT : {
            return WGPUTextureFormat_R32Sint;
        } break;
        
        case TEXTURE_FORMAT_R32_FLOAT : {
            return WGPUTextureFormat_R32Float;
        } break;
        
        case TEXTURE_FORMAT_RG16U_INT : {
            return WGPUTextureFormat_RG16Uint;
        } break;
        
        case TEXTURE_FORMAT_RG16S_INT : {
            return WGPUTextureFormat_RG16Sint;
        } break;
        
        case TEXTURE_FORMAT_RG16_FLOAT : {
            return WGPUTextureFormat_RG16Float;
        } break;
        
        case TEXTURE_FORMAT_RGBA8U_NORM : {
            return WGPUTextureFormat_RGBA8Unorm;
        } break;
        
        case TEXTURE_FORMAT_RGBA8S_NORM : {
            return WGPUTextureFormat_RGBA8Snorm;
        } break;
        
        case TEXTURE_FORMAT_RGBA8U_INT : {
            return WGPUTextureFormat_RGBA8Uint;
        } break;
        
        case TEXTURE_FORMAT_RGBA8S_INT : {
            return WGPUTextureFormat_RGBA8Sint;
        } break;
        
        case TEXTURE_FORMAT_RG32U_INT : {
            return WGPUTextureFormat_RG32Uint;
        } break;
        
        case TEXTURE_FORMAT_RG32S_INT : {
            return WGPUTextureFormat_RG32Sint;
        } break;
        
        case TEXTURE_FORMAT_RG32_FLOAT : {
            return WGPUTextureFormat_RG32Float;
        } break;
        
        case TEXTURE_FORMAT_RGBA16U_INT : {
            return WGPUTextureFormat_RGBA16Uint;
        } break;
        
        case TEXTURE_FORMAT_RGBA16S_INT : {
            return WGPUTextureFormat_RGBA16Sint;
        } break;
        
        case TEXTURE_FORMAT_RGBA16_FLOAT : {
            return WGPUTextureFormat_RGBA16Float;
        } break;
        
        case TEXTURE_FORMAT_RGBA32U_INT : {
            return WGPUTextureFormat_RGBA32Uint;
        } break;
        
        case TEXTURE_FORMAT_RGBA32S_INT : {
            return WGPUTextureFormat_RGBA32Sint;
        } break;
        
        case TEXTURE_FORMAT_RGBA32_FLOAT : {
            return WGPUTextureFormat_RGBA32Float;
        } break;
        
        case TEXTURE_FORMAT_BGRA8U_NORM: {
            return WGPUTextureFormat_BGRA8Unorm;
        } break;
        
        case TEXTURE_FORMAT_BGRA8U_NORM_SRGB: {
            return WGPUTextureFormat_BGRA8UnormSrgb;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid Texture format.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_texture_access(u32 access)
{
    switch(access)
    {
        case TEXTURE_ACCESS_WRITEONLY: {
            return WGPUStorageTextureAccess_WriteOnly;
        } break;
        
        case TEXTURE_ACCESS_READONLY: {
            return WGPUStorageTextureAccess_ReadOnly;
        } break;
        
        case TEXTURE_ACCESS_READWRITE: {
            return WGPUStorageTextureAccess_ReadWrite;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid texture access type.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_texture_view_dim(u32 type)
{
    switch (type)
    {
        case TEXTURE_VIEW_DIM_1D: {
            return WGPUTextureViewDimension_1D;
        } break;
        
        case TEXTURE_VIEW_DIM_2D: {
            return WGPUTextureViewDimension_2D;
        } break;
        
        case TEXTURE_VIEW_DIM_2DARRAY: {
            return WGPUTextureViewDimension_2DArray;
        } break;
        
        case TEXTURE_VIEW_DIM_CUBE: {
            return WGPUTextureViewDimension_Cube;
        } break;
        
        case TEXTURE_VIEW_DIM_CUBEARRAY: {
            return WGPUTextureViewDimension_CubeArray;
        } break;
        
        case TEXTURE_VIEW_DIM_3D: {
            return WGPUTextureViewDimension_3D;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid texture view dimension.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_texture_sample_type(u32 format)
{
    switch (format)
    {
        case TEXTURE_FORMAT_R8U_NORM :
        case TEXTURE_FORMAT_R8S_NORM :
        case TEXTURE_FORMAT_R16_FLOAT :
        case TEXTURE_FORMAT_RG8U_NORM :
        case TEXTURE_FORMAT_RG8S_NORM :
        case TEXTURE_FORMAT_RG16_FLOAT :
        case TEXTURE_FORMAT_RGBA8U_NORM :
        case TEXTURE_FORMAT_RGBA8S_NORM :
        case TEXTURE_FORMAT_RGBA16_FLOAT :
        case TEXTURE_FORMAT_BGRA8U_NORM:
        case TEXTURE_FORMAT_BGRA8U_NORM_SRGB: {
            return WGPUTextureSampleType_Float;
        } break;
        
        case TEXTURE_FORMAT_R8U_INT :
        case TEXTURE_FORMAT_R16U_INT :
        case TEXTURE_FORMAT_RG8U_INT :
        case TEXTURE_FORMAT_R32U_INT :
        case TEXTURE_FORMAT_RG16U_INT :
        case TEXTURE_FORMAT_RGBA8U_INT :
        case TEXTURE_FORMAT_RG32U_INT :
        case TEXTURE_FORMAT_RGBA16U_INT :
        case TEXTURE_FORMAT_RGBA32U_INT : {
            return WGPUTextureSampleType_Uint;
        } break;
        
        case TEXTURE_FORMAT_R8S_INT :
        case TEXTURE_FORMAT_R16S_INT :
        case TEXTURE_FORMAT_RG8S_INT :
        case TEXTURE_FORMAT_R32S_INT :
        case TEXTURE_FORMAT_RG16S_INT :
        case TEXTURE_FORMAT_RGBA8S_INT :
        case TEXTURE_FORMAT_RG32S_INT :
        case TEXTURE_FORMAT_RGBA16S_INT :
        case TEXTURE_FORMAT_RGBA32S_INT : {
            return WGPUTextureSampleType_Sint;
        } break;
        
        case TEXTURE_FORMAT_R32_FLOAT :
        case TEXTURE_FORMAT_RG32_FLOAT :
        case TEXTURE_FORMAT_RGBA32_FLOAT : {
            return WGPUTextureSampleType_UnfilterableFloat;
        } break;
        
        
        default: {
            ASSERT_LOG(0, "Invalid sampler type.");
        } break;
    }
    
    return 0;
}

internal u32
get_wgpu_address_mode(u32 mode)
{
    switch (mode)
    {
        case ADDRESS_MODE_CLAMPTOEDGE: {
            return WGPUAddressMode_ClampToEdge;
        } break;
        
        case ADDRESS_MODE_REPEAT: {
            return WGPUAddressMode_Repeat;
        } break;
        
        case ADDRESS_MODE_MIRRORREPEAT: {
            return WGPUAddressMode_MirrorRepeat;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid sampler address mode.");
        } break;
    }
}

internal u32
get_wgpu_filter_mode(u32 mode)
{
    switch (mode)
    {
        case FILTER_NEAREST: {
            return WGPUFilterMode_Nearest;
        } break;
        
        case FILTER_LINEAR: {
            return WGPUFilterMode_Linear;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid sampler filter mode.");
        } break;
    }
}

internal u32
get_wgpu_mipmap_filter_mode(u32 mode)
{
    switch (mode)
    {
        case MIPMAP_FILTER_NEAREST: {
            return WGPUMipmapFilterMode_Nearest;
        } break;
        
        case MIPMAP_FILTER_LINEAR: {
            return WGPUMipmapFilterMode_Linear;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid sampler mipmap filter mode.");
        } break;
    }
}

#if __EMSCRIPTEN__

internal u32
get_wgpu_blend_factor(u32 factor)
{
    switch (factor)
    {
        case BLEND_FACTOR_ZERO: {
            return WGPUBlendFactor_Zero;
        } break;
        
        case BLEND_FACTOR_ONE: {
            return WGPUBlendFactor_One;
        } break;
        
        case BLEND_FACTOR_SRC: {
            return WGPUBlendFactor_Src;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC: {
            return WGPUBlendFactor_OneMinusSrc;
        } break;
        
        case BLEND_FACTOR_SRCALPHA: {
            return WGPUBlendFactor_SrcAlpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRCALPHA: {
            return WGPUBlendFactor_OneMinusSrcAlpha;
        } break;
        
        case BLEND_FACTOR_DST: {
            return WGPUBlendFactor_Dst;
        } break;
        
        case BLEND_FACTOR_ONEMINUSDST: {
            return WGPUBlendFactor_OneMinusDst;
        } break;
        
        case BLEND_FACTOR_DSTALPHA: {
            return WGPUBlendFactor_DstAlpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSDSTALPHA: {
            return WGPUBlendFactor_OneMinusDstAlpha;
        } break;
        
        case BLEND_FACTOR_SRCALPHASATURATED: {
            return WGPUBlendFactor_SrcAlphaSaturated;
        } break;
        
        case BLEND_FACTOR_CONSTANT: {
            return WGPUBlendFactor_Constant;
        } break;
        
        case BLEND_FACTOR_ONEMINUSCONSTANT: {
            return WGPUBlendFactor_OneMinusConstant;
        } break;
        
        /*case BLEND_FACTOR_SRC1: {
            return WGPUBlendFactor_Src1;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC1: {
            return WGPUBlendFactor_OneMinusSrc1;
        } break;
        
        case BLEND_FACTOR_SRC1ALPHA: {
            return WGPUBlendFactor_Src1Alpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC1ALPHA: {
            return WGPUBlendFactor_OneMinusSrc1Alpha;
        } break;*/
        
        default: {
            ASSERT_LOG(0, "Invalid Blend Factor.");
        } break;
    }
    
    return 0;
}

#else

internal u32
get_wgpu_blend_factor(u32 factor)
{
    switch (factor)
    {
        case BLEND_FACTOR_ZERO: {
            return WGPUBlendFactor_Zero;
        } break;
        
        case BLEND_FACTOR_ONE: {
            return WGPUBlendFactor_One;
        } break;
        
        case BLEND_FACTOR_SRC: {
            return WGPUBlendFactor_Src;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC: {
            return WGPUBlendFactor_OneMinusSrc;
        } break;
        
        case BLEND_FACTOR_SRCALPHA: {
            return WGPUBlendFactor_SrcAlpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRCALPHA: {
            return WGPUBlendFactor_OneMinusSrcAlpha;
        } break;
        
        case BLEND_FACTOR_DST: {
            return WGPUBlendFactor_Dst;
        } break;
        
        case BLEND_FACTOR_ONEMINUSDST: {
            return WGPUBlendFactor_OneMinusDst;
        } break;
        
        case BLEND_FACTOR_DSTALPHA: {
            return WGPUBlendFactor_DstAlpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSDSTALPHA: {
            return WGPUBlendFactor_OneMinusDstAlpha;
        } break;
        
        case BLEND_FACTOR_SRCALPHASATURATED: {
            return WGPUBlendFactor_SrcAlphaSaturated;
        } break;
        
        case BLEND_FACTOR_CONSTANT: {
            return WGPUBlendFactor_Constant;
        } break;
        
        case BLEND_FACTOR_ONEMINUSCONSTANT: {
            return WGPUBlendFactor_OneMinusConstant;
        } break;
        
        case BLEND_FACTOR_SRC1: {
            return WGPUBlendFactor_Src1;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC1: {
            return WGPUBlendFactor_OneMinusSrc1;
        } break;
        
        case BLEND_FACTOR_SRC1ALPHA: {
            return WGPUBlendFactor_Src1Alpha;
        } break;
        
        case BLEND_FACTOR_ONEMINUSSRC1ALPHA: {
            return WGPUBlendFactor_OneMinusSrc1Alpha;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid Blend Factor.");
        } break;
    }
    
    return 0;
}

#endif

internal u32
get_wgpu_blend_op(u32 op)
{
    switch (op)
    {
        case BLEND_OP_ADD: {
            return WGPUBlendOperation_Add;
        } break;
        
        case BLEND_OP_SUBTRACT: {
            return WGPUBlendOperation_Subtract;
        } break;
        
        case BLEND_OP_REVERSESUBTRACT: {
            return WGPUBlendOperation_ReverseSubtract;
        } break;
        
        case BLEND_OP_MIN: {
            return WGPUBlendOperation_Min;
        } break;
        
        case BLEND_OP_MAX: {
            return WGPUBlendOperation_Max;
        } break;
        
        default: {
            ASSERT_LOG(0, "Invalid Blend Operation.");
        } break;
    }
    
    return 0;
}

internal WGPUBlendComponent
get_wgpu_blend_comp(blend_comp_t comp)
{
    return (WGPUBlendComponent) {
        .srcFactor = get_wgpu_blend_factor(comp.src_factor),
        .dstFactor = get_wgpu_blend_factor(comp.dst_factor),
        .operation = get_wgpu_blend_op(comp.op),
    };
}