#version 410


uniform mat4 camera2clip_matrix;
uniform float z;

layout(location=0) in vec2 position_in;
layout(location=1) in vec4 color_in;

out vec4 color;


void main() {
	gl_Position = camera2clip_matrix * vec4(position_in, z, 1.0);
	color = color_in;
}
