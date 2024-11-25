
@group(0) @binding(0) var input: texture_2d<f32>;
@group(0) @binding(1) var output: texture_storage_2d<rgba8unorm,write>;

@compute @workgroup_size(16, 16, 1)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>)
{
	let dim = textureDimensions(input);

	if (all(id.xy < dim)) {
		
		let kernel = array<vec2<i32>, 9>(
			vec2<i32>(-1, -1), vec2<i32>( 0, -1), vec2<i32>( 1, -1),
			vec2<i32>(-1,  0), vec2<i32>( 0,  0), vec2<i32>( 1,  0),
			vec2<i32>(-1,  1), vec2<i32>( 0,  1), vec2<i32>( 1,  1) 
		);
		let weights = array<f32, 9>(1.0, 2.0, 1.0, 2.0, 4.0, 2.0, 1.0, 2.0, 1.0);
		var color: vec4f = vec4f(0.0);
		var total_weight: f32 = 0.0;

		for (var i = 0u; i < 9u; i = i + 1u)
		{

			let offset = kernel[i];
			let coord = vec2<i32>(id.xy) + offset;

			let clamped_coord = vec2<i32>(
				clamp(coord.x, 0, i32(dim.x - 1)),
				clamp(coord.y, 0, i32(dim.y - 1))
			);

			let sample_color = textureLoad(input, coord, 0);

			color = color + sample_color * weights[i];

			total_weight = total_weight + weights[i];		

		}

		let gaussian_color = color / total_weight;

		textureStore(output, id.xy, gaussian_color);
	}
}