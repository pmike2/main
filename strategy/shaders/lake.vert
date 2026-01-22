#version 410


uniform mat4 world2clip_matrix;
//uniform float wave;

layout(location=0) in vec3 position_in;
layout(location=1) in vec4 color_in;

out vec3 vertex_position;
out vec4 vertex_color;
//out vec3 vertex_normal;


void main(void) {
	vertex_color= color_in;

	//float dist = length(vertex_position);
	//vertex_normal= normalize(vec3(0.1, 0.1, wave + fract(dist)));
	//vertex_normal= normalize(vec3(0.1, 0.1, wave));
	//vertex_normal= vec3(0.0, 0.0, wave);
	vertex_position= position_in;

	gl_Position= world2clip_matrix * vec4(position_in, 1.0);
}
