
@group(0) @binding(0) var framebuffer: texture_storage_2d<rgba8unorm,write>;

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(framebuffer);

	if(all(id.xy < dim)) {
		textureStore(framebuffer, id.xy, vec4<f32>(0.0, 0.0, 0.0, 0.0));
	}
}