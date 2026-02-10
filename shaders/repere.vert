#version 410 core


uniform mat4 world2clip;


layout(location=0) in vec3 position_in;
layout(location=1) in vec4 color_in;


flat out vec4 vertex_color;


void main(void) {
	vertex_color= color_in;

	gl_Position= world2clip * vec4(position_in, 1.0);
}
