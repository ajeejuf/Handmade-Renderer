
@group(0) @binding(0) var<storage, read> metaball_pos: array<vec4f>;
@group(0) @binding(1) var<storage, read> metaball_radius: array<f32>;
@group(0) @binding(2) var<uniform> metaball_count: u32;

@group(1) @binding(0) var framebuffer: texture_storage_2d<rgba8unorm, write>;

@group(2) @binding(0) var<uniform> color: vec3f;

fn point_length2(p0: vec2f, p1: vec2f) -> f32 {
	let dx = p0.x - p1.x;
	let dy = p0.y - p1.y;

	return dx*dx + dy*dy;
}

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(framebuffer);

	if (all(id.xy < dim.xy)) {
		let px_pos = vec2f(id.xy) + 0.5;

		var final_alpha = 0.0;
		for (var i: u32 = 0; i < metaball_count; i++) {
			let pos = metaball_pos[i].xy;

			let thickness = 2.0;
			let smoothness = 1.0;

			let len2 = point_length2(px_pos, pos);
			let r = metaball_radius[i];

			let r_outer2 = (r + smoothness) * (r + smoothness);
			let r_inner2 = (r - smoothness) * (r - smoothness);
			let r_thick_outer2 = (r - thickness + smoothness) * (r - thickness + smoothness);
			let r_thick_inner2 = (r - thickness - smoothness) * (r - thickness - smoothness);

			let outer = smoothstep(r_outer2, r_inner2, len2);
			let inner = smoothstep(r_thick_outer2, r_thick_inner2, len2);

			let alpha = outer*(1.0 - inner);

			final_alpha = max(final_alpha, alpha);
			
		}

		if (final_alpha > 0.0) {
			textureStore(framebuffer, id.xy, vec4f(color, final_alpha));
		}
	}
}