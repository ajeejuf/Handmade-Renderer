
@group(0) @binding(0) var<storage, read> metaball_pos: array<vec3f>;
@group(0) @binding(1) var<storage, read> metaball_radius: array<f32>;
@group(0) @binding(2) var<uniform> metaball_count: u32;
@group(0) @binding(3) var<uniform> res: vec2<u32>;

@group(1) @binding(0) var framebuffer: texture_storage_2d<rgba8unorm, write>;

@group(2) @binding(0) var view: mat4f;
@group(2) @binding(1) var proj: mat4f;
@group(3) @binding(2) var color: vec4f;


fn clip_to_screen(clip_pos: vec4f, dim: vec2<u32>) {
	var ndc = clip_pos.xyz / clip_pos.w;
	return (ndc.xy * 0.5 + 0.5) * vec2f(dim);
}

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>) {
	let dim = textureDimensions(framebuffer);

	if (all(id.xy < dim)) {
		
		var final_color: vec3f = vec3f(0.0);

		for (var i = 0u; i < metaball_count; i = i + 1u) {

			let clip_space_pos = proj * view * vec4f(metaball_pos[i], 1.0);

			let screen_pos = clip_to_screen(clip_space_pos);

			let dist = distance(vec3f(id.xy), screen_pos.xy);

			let influence = max(0.0, 1.0 - dist / metaball_radius[i]);

			final_color += metaball_color[i] * influence;

		}

		final_color = saturate(final_color);
		
		textureStore(framebuffer, id.xy, vec4f(final_color, 1.0));
	}
}