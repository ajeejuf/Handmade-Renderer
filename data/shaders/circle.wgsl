
@group(0) @binding(0) var circle_texture: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(circle_texture);

	if (all(id.xy < dim.xy)) {
		let uv = vec2f(id.xy)/vec2f(dim.xy) * 2.0 - 1.0;;

		let dst = 1.0 - length(uv);
		let thickness = 0.05;
		let fade = 0.05;
		
		var color = smoothstep(0.0, fade, dst);
		color *= smoothstep(thickness + fade, thickness, dst);

		textureStore(circle_texture, id.xy, vec4f(color));
	}
}