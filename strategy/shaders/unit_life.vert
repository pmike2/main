#version 410


uniform mat4 camera2clip_matrix;

layout(location=0) in vec3 position;
layout(location=1) in vec2 unit_position;
layout(location=2) in vec4 color;

out VS_OUT {
	vec2 unit_position;
	vec4 color;
} vs_out;


void main() {
	vs_out.unit_position = unit_position;
	vs_out.color = color;

	gl_Position = camera2clip_matrix * vec4(position.xyz, 1.0);
}
