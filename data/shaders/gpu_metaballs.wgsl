
@group(0) @binding(0) var<storage, read> metaball_pos: array<vec4f>;
@group(0) @binding(1) var<storage, read> metaball_radius: array<f32>;
@group(0) @binding(2) var<uniform> metaball_count: u32;

@group(1) @binding(0) var framebuffer: texture_storage_2d<rgba8unorm, write>;

@group(2) @binding(0) var<uniform> res: vec4<u32>;


fn point_length2(p0: vec2f, p1: vec2f) -> f32 {
	let dx = p0.x - p1.x;
	let dy = p0.y - p1.y;

	return dx*dx + dy*dy;
}

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>) {
	
	let dim = textureDimensions(framebuffer);

	if (all(id.xy < dim.xy)) {

		let px = vec2u(ceil(vec2f(dim.xy)/vec2f(res.xy)));

		let res_pos = (floor(vec2f(id.xy)/vec2f(px)) + 0.5) * vec2f(px);

		var total_influence: f32 = 0.0;
		for (var i = 0u; i < metaball_count; i = i + 1u) {

			let pos = metaball_pos[i].xy;

			let len2 = point_length2(res_pos, pos);
			let r = metaball_radius[i];
			let r2 = r*r;

			total_influence += r2/len2;
		}


		let alpha = step(1.0, total_influence);
		let final_color = vec3f(0.0, 1.0, 0.0)*alpha;
		
		textureStore(framebuffer, id.xy, vec4f(final_color, alpha));
	}
}