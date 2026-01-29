#version 410


uniform mat4 camera2clip_matrix;

in vec3 position_in;
in vec4 color_in;

out vec4 color;


void main() {
	gl_Position = camera2clip_matrix * vec4(position_in.xyz, 1.0);
	color = color_in;
}
