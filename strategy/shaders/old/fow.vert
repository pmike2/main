#version 410


uniform mat4 world2clip_matrix;
uniform float z;

in vec2 position_in;
in vec4 color_in;

out vec4 color;


void main() {
	gl_Position = world2clip_matrix * vec4(position_in.xy, z, 1.0);
	color = color_in;
}
