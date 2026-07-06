#version 410


uniform mat4 world2clip_matrix;
uniform mat4 anim_matrices[100];

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in float matrix_idx;
layout(location=3) in vec3 color_in;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	float x = matrix_idx;
	mat4 m = anim_matrices[0];
	vertex_position = vec3(anim_matrices[int(matrix_idx)] * vec4(position_in, 1.0));
	//vertex_normal = vec3(anim_matrices[int(matrix_idx)] * vec4(normalize(normal_in), 1.0));
	vertex_normal = mat3(anim_matrices[int(matrix_idx)]) * normalize(normal_in);

	//vertex_position = vec3(anim_matrices[0] * vec4(position_in, 1.0));
	//vertex_normal = vec3(anim_matrices[0] * vec4(normalize(normal_in), 1.0));

	//vertex_position = position_in;
	//vertex_normal = normalize(normal_in);

	vertex_color = color_in;
	//vertex_color = vec3(anim_matrices[0][0][0], 0.0, 0.0);

	gl_Position = world2clip_matrix * vec4(vertex_position, 1.0);
}
