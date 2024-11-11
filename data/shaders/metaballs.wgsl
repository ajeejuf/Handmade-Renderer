
struct VertexInput {
	@location(0) pos: vec3f,
	@location(1) norm: vec3f,
	@location(2) uv: vec2f,
	@location(3) color: vec4f
};

struct VertexOutput {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f
};

@group(0) @binding(0) var distance_field: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.pos = vec4f(in.pos.x, in.pos.y, 0.0, 1.0);
	out.uv = in.uv;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

	let texel_coords = vec2i(in.uv * vec2f(textureDimensions(distance_field)));

	let color = textureLoad(distance_field, texel_coords, 0);

	return vec4f(color);
}