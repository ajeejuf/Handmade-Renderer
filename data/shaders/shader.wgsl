
struct VertexInput {
	@location(0) pos: vec3f,
	@location(1) norm: vec3f,
	@location(2) uv: vec2f,
	@location(3) color: vec4f
};

struct VertexOutput {
	@builtin(position) pos: vec4f,
	@location(0) color: vec4f,
};

@group(0) @binding(0) var<uniform> dummy0: vec3f;
@group(0) @binding(1) var<uniform> dummy1: vec3f;
@group(1) @binding(0) var<uniform> offsetx: f32;
@group(1) @binding(1) var<uniform> offsety: f32;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.pos = vec4f(in.pos.x + offsetx, in.pos.y + offsety, 0.0, 1.0);
	out.color = vec4f(dummy0 + dummy1, 1.0);
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	return in.color;
}