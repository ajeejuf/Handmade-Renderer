
struct VertexInput {
	@builtin(instance_index) inst_idx: u32,
	@location(0) pos: vec3f,
	@location(1) norm: vec3f,
	@location(2) uv: vec2f,
	@location(3) color: vec4f
};

struct VertexOutput {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f
};

@group(0) @binding(0) var<storage, read> models: array<mat4x4f>;

@group(1) @binding(0) var<uniform> view: mat4x4f;
@group(1) @binding(1) var<uniform> proj: mat4x4f;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.pos = proj*view*models[in.inst_idx]*vec4f(in.pos, 1.0);
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	return vec4f(1.0, 1.0, 1.0, 0.8);
}