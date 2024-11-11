
@group(0) @binding(0) var distance_field: texture_storage_2d<rgba8unorm,write>;

fn compute_distance(x: f32, y: f32) -> f32 {
	let dist = inverseSqrt(x*x + y*y) * 10;

	return dist;
}

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(distance_field);

	if(all(id.xy < dim)) {

		let x = f32(id.x) - f32(dim.x/2);
		let y = f32(id.y) - f32(dim.y/2);

		let dist_value = clamp(compute_distance(x, y), 0.0, 1.0);

		let color = vec4f(dist_value, dist_value, dist_value, 1.0);

		textureStore(distance_field, id.xy, color);

	}
}