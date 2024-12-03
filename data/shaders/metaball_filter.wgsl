
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

@group(0) @binding(0) var input: texture_2d<f32>;
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

	let color = textureSample(input, tex_sampler, in.uv);

	let distance: f32 = color.a;
	let alpha: f32 = smoothstep(1.0-0.2, 1.0+0.2, distance); 

	
	return vec4f(mix(vec3f(0.0, 0.2, 0.5), color.rgb, pow(alpha, 0.1)), 1.0);

	//return color;
}