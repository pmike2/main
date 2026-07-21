#version 410


uniform mat4 world2clip_matrix;
uniform mat4 model2world_matrix;
uniform vec3 light_position;
uniform vec3 view_position;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 tangent_in;
layout(location=3) in vec3 bitangent_in;
layout(location=4) in vec3 tex_coord_in;
layout(location=5) in float shininess_in;

out vec3 tex_coord;
out vec3 tangent_light_position;
out vec3 tangent_view_position;
out vec3 tangent_vertex_position;
out float shininess;


void main(void) {
	tex_coord = tex_coord_in;
	shininess = shininess_in;

	vec4 vertex_position = model2world_matrix * vec4(position_in, 1.0);

	mat3 normal_matrix = transpose(inverse(mat3(model2world_matrix)));

	vec3 normal = normalize(normal_matrix * normal_in);
	
	vec3 tangent = normalize(normal_matrix * tangent_in);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	
	vec3 bitangent = normalize(normal_matrix * bitangent_in);
	bitangent = cross(normal, tangent);
	
	mat3 tbn = transpose(mat3(tangent, bitangent, normal));

	tangent_light_position = tbn * light_position;
	tangent_view_position = tbn * view_position;
	tangent_vertex_position = tbn * vec3(vertex_position);

	gl_Position = world2clip_matrix * vertex_position;
}
