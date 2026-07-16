#version 410


uniform mat4 world2clip_matrix;
uniform samplerBuffer anim_buffer; // contient les matrices de transformation (action -> frame -> vertex)
uniform sampler2D idx_texture; // contient les indices de anim_buffer des débuts des frames

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 color_in;
layout(location=3) in float idx_action;
layout(location=4) in float idx_frame;
layout(location=5) in mat4 model2world_matrix;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 vertex_color;


void main(void) {
	// texelFetch permet de lire les textures avec des coordonnées entières (pas 0.0 - 1.0) et sans interpolation
	int idx = int(texelFetch(idx_texture, ivec2(int(idx_frame), int(idx_action)), 0).r) + 16 * gl_VertexID;
	mat4 anim_matrix = mat4(
		texelFetch(anim_buffer, idx + 0).r, texelFetch(anim_buffer, idx + 1).r, texelFetch(anim_buffer, idx + 2).r, texelFetch(anim_buffer, idx + 3).r,
		texelFetch(anim_buffer, idx + 4).r, texelFetch(anim_buffer, idx + 5).r, texelFetch(anim_buffer, idx + 6).r, texelFetch(anim_buffer, idx + 7).r,
		texelFetch(anim_buffer, idx + 8).r, texelFetch(anim_buffer, idx + 9).r, texelFetch(anim_buffer, idx + 10).r, texelFetch(anim_buffer, idx + 11).r,
		texelFetch(anim_buffer, idx + 12).r, texelFetch(anim_buffer, idx + 13).r, texelFetch(anim_buffer, idx + 14).r, texelFetch(anim_buffer, idx + 15).r
	);

	//anim_matrix = mat4(1.0);

	mat4 m = model2world_matrix * anim_matrix;
	vec4 p = m * vec4(position_in, 1.0);

	vertex_position = vec3(p);
	vertex_normal = mat3(m) * normalize(normal_in);
	vertex_color = color_in;

	gl_Position = world2clip_matrix * p;
}
