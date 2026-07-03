#version 410


uniform mat4 world2clip_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 color_in;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	vertex_position = position_in;
	vertex_normal = normalize(normal_in);
	vertex_color = color_in;

	gl_Position = world2clip_matrix * vec4(vertex_position, 1.0);
}
