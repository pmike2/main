#version 410 core

uniform mat4 world2clip_matrix;

in vec3 vertex_in;
in vec2 tex_in;
in vec4 color_in;
in float current_layer_in;

out vec2 tex_coords;
out vec4 color;
out float current_layer;


void main() {
	gl_Position= world2clip_matrix* vec4(vertex_in.xyz, 1.0);
	tex_coords= tex_in;
	color= color_in;
	current_layer= current_layer_in;
}
