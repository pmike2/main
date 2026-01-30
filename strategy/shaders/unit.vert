#version 410


uniform mat4 world2clip_matrix;


layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 ambient_in;
layout(location=3) in vec3 diffuse_in;
layout(location=4) in vec3 specular_in;
layout(location=5) in float shininess_in;
layout(location=6) in float opacity_in;
layout(location=7) in mat4 model2world_matrix;
layout(location=11) in vec3 team_color_in;
layout(location=12) in float hit_in;


out VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_ambient_color;
	vec3 vertex_diffuse_color;
	vec3 vertex_specular_color;
	float vertex_shininess;
	float vertex_opacity;
	vec3 team_color;
	float hit;
} vs_out;


void main(void) {
	vs_out.vertex_position = position_in;
	vs_out.vertex_normal = normalize(normal_in);
	vs_out.vertex_ambient_color = ambient_in;
	vs_out.vertex_diffuse_color = diffuse_in;
	vs_out.vertex_specular_color = specular_in;
	vs_out.vertex_shininess = shininess_in;
	vs_out.vertex_opacity = opacity_in;
	vs_out.team_color = team_color_in;
	vs_out.hit = hit_in;

	//gl_Position = world2clip_matrix * model2world_matrices[gl_InstanceID] * vec4(position_in, 1.0);
	gl_Position = world2clip_matrix * model2world_matrix * vec4(position_in, 1.0);
}
