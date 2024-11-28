
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
@group(0) @binding(1) var tex_sampler: sampler;

@group(1) @binding(0) var<uniform> proj: mat4x4f;
@group(1) @binding(1) var<uniform> view: mat4x4f;
@group(1) @binding(2) var<uniform> metaball_color: vec3f;

@group(2) @binding(0) var<uniform> model: mat4x4f;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.pos = proj*view*model*vec4f(in.pos, 1.0);
	out.uv = in.uv;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

	let color: vec4f = textureSample(distance_field, tex_sampler, in.uv);

	return vec4f(metaball_color, color.a);

	//return vec4f(0.0, 0.0, 0.0, 1.0);
}