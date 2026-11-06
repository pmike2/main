#version 410


uniform mat4 world2clip_matrix;

uniform samplerBuffer anim_buffer; // contient les matrices de transformation (action -> frame -> vertex)
uniform sampler2D anim_idx; // contient les indices de anim_buffer des débuts des frames


layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 tex_coord_in;
layout(location=3) in mat4 model2world_matrix;
layout(location=7) in vec3 team_color_in;
layout(location=8) in float hit_in;
layout(location=9) in float alpha_in;
layout(location=10) in float idx_action;
layout(location=11) in float idx_frame;


out VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 tex_coord;
	vec3 team_color;
	float hit;
	float alpha;
} vs_out;


void main(void) {
	// texelFetch permet de lire les textures avec des coordonnées entières (pas 0.0 - 1.0) et sans interpolation
	int idx = int(texelFetch(anim_idx, ivec2(int(idx_frame), int(idx_action)), 0).r) + 16 * gl_VertexID;

	// récupération de la matrice d'animation à appliquer à ce vertex
	mat4 anim_matrix = mat4(
		texelFetch(anim_buffer, idx + 0).r, texelFetch(anim_buffer, idx + 1).r, texelFetch(anim_buffer, idx + 2).r, texelFetch(anim_buffer, idx + 3).r,
		texelFetch(anim_buffer, idx + 4).r, texelFetch(anim_buffer, idx + 5).r, texelFetch(anim_buffer, idx + 6).r, texelFetch(anim_buffer, idx + 7).r,
		texelFetch(anim_buffer, idx + 8).r, texelFetch(anim_buffer, idx + 9).r, texelFetch(anim_buffer, idx + 10).r, texelFetch(anim_buffer, idx + 11).r,
		texelFetch(anim_buffer, idx + 12).r, texelFetch(anim_buffer, idx + 13).r, texelFetch(anim_buffer, idx + 14).r, texelFetch(anim_buffer, idx + 15).r
	);

	anim_matrix = mat4(1.0);

	mat4 m = model2world_matrix * anim_matrix;
	vec4 p = m * vec4(position_in, 1.0);

	vs_out.vertex_position = vec3(p);
	vs_out.vertex_normal = mat3(m) * normalize(normal_in);
	vs_out.tex_coord = tex_coord_in;
	vs_out.team_color = team_color_in;
	vs_out.hit = hit_in;
	vs_out.alpha = alpha_in;

	gl_Position = world2clip_matrix * p;
}
