#version 410


uniform mat4 camera2clip_matrix;
uniform mat4 model2world_matrix;

in vec3 position_in;


void main(void) {
	gl_Position= camera2clip_matrix* model2world_matrix* vec4(position_in, 1.0);
}
