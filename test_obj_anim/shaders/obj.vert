#version 410


uniform mat4 world2clip_matrix;
uniform mat4 anim_matrices[100];

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 color_in;
layout(location=3) in int matrix_idx;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	vertex_position = vec3(anim_matrices[matrix_idx] * vec4(position_in, 1.0));
	vertex_normal = vec3(anim_matrices[matrix_idx] * vec4(normalize(normal_in), 1.0));
	vertex_color = color_in;

	gl_Position = world2clip_matrix * vec4(vertex_position, 1.0);
}
