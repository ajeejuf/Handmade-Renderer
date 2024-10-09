
#include <webgpu\webgpu.h>
#include <dawn/dawn_proc.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    WGPUInstanceDescriptor desc = {0};
    desc.nextInChain = NULL;
    
    WGPUInstance instance = wgpuCreateInstance(&desc);
    
    if (!instance) {
        fprintf(stderr, "Could not initialize WebGPU!\n");
        return 1;
    }
    
    fprintf(stdout, "WGPU instance: %p\n", instance);
    
    return 0;
}