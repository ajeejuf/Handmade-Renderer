
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

@group(0) @binding(0) var fb: texture_2d<f32>;
@group(0) @binding(1) var tex_sampler: sampler;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.pos = vec4f(in.pos, 1.0);
	out.uv = in.uv;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let color = textureSample(fb, tex_sampler, in.uv);

	return color;
}