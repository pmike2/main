#version 410 core


uniform mat4 world2camera_matrix;
uniform mat4 camera2clip_matrix;

layout(location=0) in vec3 position_in;

out vec3 text_coords;


void main() {
	text_coords= position_in;
	gl_Position= camera2clip_matrix* world2camera_matrix * vec4(position_in, 1.0);
}
