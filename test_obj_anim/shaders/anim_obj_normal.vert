#version 410

uniform mat4 world2clip_matrix;
uniform vec3 light_position;
uniform vec3 view_position;

uniform samplerBuffer anim_buffer; // contient les matrices de transformation (action -> frame -> vertex)
uniform sampler2D idx_texture; // contient les indices de anim_buffer des débuts des frames

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 tangent_in;
layout(location=3) in vec3 bitangent_in;
layout(location=4) in vec3 tex_coord_in;
layout(location=5) in float shininess_in;
layout(location=6) in float idx_action;
layout(location=7) in float idx_frame;
layout(location=8) in mat4 model2world_matrix;

out vec3 tex_coord;
out vec3 tangent_light_position;
out vec3 tangent_view_position;
out vec3 tangent_vertex_position;
out float shininess;


void main(void) {
	tex_coord = tex_coord_in;
	shininess = shininess_in;

	int idx = int(texelFetch(idx_texture, ivec2(int(idx_frame), int(idx_action)), 0).r) + 16 * gl_VertexID;
	mat4 anim_matrix = mat4(
		texelFetch(anim_buffer, idx + 0).r, texelFetch(anim_buffer, idx + 1).r, texelFetch(anim_buffer, idx + 2).r, texelFetch(anim_buffer, idx + 3).r,
		texelFetch(anim_buffer, idx + 4).r, texelFetch(anim_buffer, idx + 5).r, texelFetch(anim_buffer, idx + 6).r, texelFetch(anim_buffer, idx + 7).r,
		texelFetch(anim_buffer, idx + 8).r, texelFetch(anim_buffer, idx + 9).r, texelFetch(anim_buffer, idx + 10).r, texelFetch(anim_buffer, idx + 11).r,
		texelFetch(anim_buffer, idx + 12).r, texelFetch(anim_buffer, idx + 13).r, texelFetch(anim_buffer, idx + 14).r, texelFetch(anim_buffer, idx + 15).r
	);

	mat4 m = model2world_matrix * anim_matrix;
	vec4 p = m * vec4(position_in, 1.0);

	mat3 normal_matrix = transpose(inverse(mat3(m)));

	vec3 normal = normalize(normal_matrix * normal_in);
	
	vec3 tangent = normalize(normal_matrix * tangent_in);

	// pas sur que ce soit utile; fait dans https://learnopengl.com/Advanced-Lighting/Normal-Mapping
	//tangent = normalize(tangent - dot(tangent, normal) * normal);
	
	vec3 bitangent = normalize(normal_matrix * bitangent_in);
	bitangent = cross(normal, tangent);
	
	mat3 tbn = transpose(mat3(tangent, bitangent, normal));

	tangent_light_position = tbn * light_position;
	tangent_view_position = tbn * view_position;
	tangent_vertex_position = tbn * vec3(p);

	gl_Position = world2clip_matrix * p;
}
