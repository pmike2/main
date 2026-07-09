#version 410


uniform mat4 world2clip_matrix;
uniform samplerBuffer anim_buffer;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 color_in;
layout(location=3) in vec4 matrix_idx;
layout(location=4) in vec4 matrix_weight;
layout(location=5) in mat4 model2world_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	mat4 m = mat4(0.0);
	for (int i=0; i<4; ++i) {
		if (matrix_idx[i] < 0.0) {
			continue;
		}
		
		int idx = 16 * int(matrix_idx[i]);
		mat4 anim_matrix = mat4(
			texelFetch(anim_buffer, idx + 0).r, texelFetch(anim_buffer, idx + 1).r, texelFetch(anim_buffer, idx + 2).r, texelFetch(anim_buffer, idx + 3).r,
			texelFetch(anim_buffer, idx + 4).r, texelFetch(anim_buffer, idx + 5).r, texelFetch(anim_buffer, idx + 6).r, texelFetch(anim_buffer, idx + 7).r,
			texelFetch(anim_buffer, idx + 8).r, texelFetch(anim_buffer, idx + 9).r, texelFetch(anim_buffer, idx + 10).r, texelFetch(anim_buffer, idx + 11).r,
			texelFetch(anim_buffer, idx + 12).r, texelFetch(anim_buffer, idx + 13).r, texelFetch(anim_buffer, idx + 14).r, texelFetch(anim_buffer, idx + 15).r
		);
		m += matrix_weight[i] * anim_matrix;
	}

	m = model2world_matrix * m;

	vec4 p = m * vec4(position_in, 1.0);

	vertex_position = vec3(p);
	vertex_normal = mat3(m) * normalize(normal_in);
	vertex_color = color_in;

	gl_Position = world2clip_matrix * p;
}
