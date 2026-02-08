#version 410


uniform mat4 camera2clip_matrix;
uniform float z;

layout(location=0) in vec2 position_in;
layout(location=1) in vec2 tex_coord_in;
layout(location=2) in float alpha_in;
layout(location=3) in float current_layer_in;

out vec2 tex_coord;
out float alpha;
out float current_layer;


void main() {
	gl_Position= camera2clip_matrix* vec4(position_in.rg, z, 1.0);
	tex_coord= tex_coord_in;
	alpha= alpha_in;
	current_layer= current_layer_in;
}
