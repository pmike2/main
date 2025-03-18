#version 410


uniform mat4 camera2clip_matrix;
uniform mat4 world2camera_matrix;

in vec3 position_in;
in vec2 tex_coord_in;
in float alpha_in;
in float current_layer_in;

out vec2 tex_coord;
out float alpha;
out float current_layer;


void main() {
	gl_Position= camera2clip_matrix* world2camera_matrix* vec4(position_in.rgb, 1.0);
	tex_coord= tex_coord_in;
	alpha= alpha_in;
	current_layer= current_layer_in;
}
