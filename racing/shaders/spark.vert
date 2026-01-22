#version 410


uniform mat4 camera2clip_matrix;
uniform mat4 world2camera_matrix;
uniform float z;

in vec2 position_in;
in float alpha_in;

out float alpha;


void main(void) {
	alpha= alpha_in;
	gl_Position= camera2clip_matrix* world2camera_matrix* vec4(position_in.rg, z, 1.0);
}
