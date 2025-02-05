#version 410


uniform mat4 camera2clip_matrix;

in vec3 position_in;
in vec2 tex_coord_in;
in float current_layer_in;
in vec4 color_in;


out vec2 tex_coord;
out float current_layer;
out vec4 vertex_color;


void main(void) {
	tex_coord= tex_coord_in;
	current_layer= current_layer_in;
	vertex_color= color_in;

	gl_Position= camera2clip_matrix* vec4(position_in.xyz, 1.0);
}
