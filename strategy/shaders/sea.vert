#version 410


uniform mat4 world2clip_matrix;
uniform float sea_level;

layout(location=0) in vec2 position_in;
layout(location=1) in vec4 color_in;

out VS_OUT {
	vec2 vertex_position;
	vec4 vertex_diffuse_color;
} vs_out;


void main(void) {
	vs_out.vertex_position= position_in;
	vs_out.vertex_diffuse_color= color_in;


	gl_Position= world2clip_matrix * vec4(position_in.xy, sea_level, 1.0);
}
