#version 410


uniform mat4 camera2clip_matrix;

in vec2 position_in;
in vec2 tex_coord_in;
in float current_layer_in;
in float selection_in;

out vec2 tex_coord;
out float current_layer;
out float selection;


void main() {
	gl_Position= camera2clip_matrix* vec4(position_in.rg, -20.0, 1.0);
	tex_coord= tex_coord_in;
	current_layer= current_layer_in;
	selection= selection_in;
}
