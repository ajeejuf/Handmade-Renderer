
@group(0) @binding(0) var distance_field: texture_storage_2d<rgba8unorm,write>;

fn sigmoid_falloff(dist: f32, rad: f32, sharpness: f32) -> f32 {
	return 1.0 / (1.0 + exp(-sharpness * (dist - rad)));
}

fn compute_distance(x: f32, y: f32) -> f32 {
	let dist = inverseSqrt(x*x + y*y)*70;
	
	return sigmoid_falloff(dist, 1, 7.0);
}

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(distance_field);

	if(all(id.xy < dim)) {

		let x = f32(id.x) - f32(dim.x/2);
		let y = f32(id.y) - f32(dim.y/2);

		var dist_value = clamp(compute_distance(x, y), 0.0, 1.0);

		let mask_rad = length(vec2f(dim))/3;
		let center = vec2f(0.0, 0.0) / 2.0;
		let dist = distance(vec2(x, y), center);
		let value = smoothstep(mask_rad, mask_rad-70.0, dist);
		
		dist_value *= value;

		let color = vec4f(dist_value, dist_value, dist_value, dist_value);

		textureStore(distance_field, id.xy, color);

	}
}