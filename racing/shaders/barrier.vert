#version 410


uniform mat4 camera2clip_matrix;
uniform mat4 world2camera_matrix;
uniform float z;

in vec2 position_in;
in vec4 color_in;
in float lambda_in;


out vec4 vertex_color;
out float lambda;


void main(void) {
	vertex_color= color_in;
	lambda= lambda_in;

	gl_Position= camera2clip_matrix* world2camera_matrix* vec4(position_in.rg, z, 1.0);
}
