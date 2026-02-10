#version 410 core

uniform mat4 world2clip_matrix;

layout(location=0) in vec3 vertex_in;
layout(location=1) in vec2 tex_in;
layout(location=2) in vec4 color_in;
layout(location=3) in float current_layer_in;

out vec2 tex_coords;
out vec4 color;
out float current_layer;


void main() {
	gl_Position= world2clip_matrix* vec4(vertex_in.xyz, 1.0);
	tex_coords= tex_in;
	color= color_in;
	current_layer= current_layer_in;
}
