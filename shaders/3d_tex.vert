#version 410

uniform mat4 world2clip_matrix;

in vec3 position_in;
in vec2 tex_coord_in;
in float current_layer_in;

out vec2 tex_coord;
out float current_layer;


void main(void) {
	gl_Position= world2clip_matrix* vec4(position_in, 1.0);
	tex_coord= tex_coord_in;
	current_layer= current_layer_in;
}
