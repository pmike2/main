#version 410


uniform mat4 world2clip_matrix;
//uniform mat4 anim_matrices[100];
uniform samplerBuffer anim_buffer;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 color_in;
layout(location=3) in float matrix_idx;
layout(location=4) in mat4 model2world_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	int idx = 16 * int(matrix_idx);
	mat4 anim_matrix = mat4(
		texelFetch(anim_buffer, idx + 0).r, texelFetch(anim_buffer, idx + 1).r, texelFetch(anim_buffer, idx + 2).r, texelFetch(anim_buffer, idx + 3).r,
		texelFetch(anim_buffer, idx + 4).r, texelFetch(anim_buffer, idx + 5).r, texelFetch(anim_buffer, idx + 6).r, texelFetch(anim_buffer, idx + 7).r,
		texelFetch(anim_buffer, idx + 8).r, texelFetch(anim_buffer, idx + 9).r, texelFetch(anim_buffer, idx + 10).r, texelFetch(anim_buffer, idx + 11).r,
		texelFetch(anim_buffer, idx + 12).r, texelFetch(anim_buffer, idx + 13).r, texelFetch(anim_buffer, idx + 14).r, texelFetch(anim_buffer, idx + 15).r
	);

	//vec4 p = model2world_matrix * anim_matrices[int(matrix_idx)] * vec4(position_in, 1.0);
	vec4 p = model2world_matrix * anim_matrix * vec4(position_in, 1.0);

	vertex_position = vec3(p);
	//vertex_normal = mat3(anim_matrices[int(matrix_idx)]) * normalize(normal_in);
	vertex_normal = mat3(anim_matrix) * normalize(normal_in);
	vertex_color = color_in;

	gl_Position = world2clip_matrix * p;
}
