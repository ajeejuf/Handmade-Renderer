
typedef struct request_adapter_callback_data_t {
    WGPUAdapter adapter;
    b8 request_ended;
} request_adapter_callback_data_t;

typedef struct request_device_callback_data_t {
    WGPUDevice device;
    b8 request_ended;
} request_device_callback_data_t;

internal void
request_adapter_callback(WGPURequestAdapterStatus status, WGPUAdapter adapter,
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

internal void
request_device_callback(WGPURequestDeviceStatus status, WGPUDevice device,
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

internal WGPUAdapter
request_adapter(WGPUInstance instance, WGPURequestAdapterOptions const *options)
{
    request_adapter_callback_data_t rac_data = {0};
    
    wgpuInstanceRequestAdapter(instance, options,
                               request_adapter_callback,
                               (void *)&rac_data);
    
#if __EMSCRIPTEN__
    while (!rac_data.request_ended) {
        LOG("Requesting adapter...");
        emscripten_sleep(100);
    }
#endif
    
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
    if (info.vendor) {
        LOG(" - vendorName: %s", info.vendor);
    }
    if (info.architecture) {
        LOG(" - architecture: %s", info.architecture);
    }
    LOG(" - deviceID: %d", info.deviceID);
    if (info.device) {
        LOG(" - name: %s", info.device);
    }
    if (info.description) {
        LOG(" - driverDescription: %s", info.description);
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
request_device(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor)
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