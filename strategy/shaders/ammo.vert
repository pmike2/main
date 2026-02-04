#version 410


uniform mat4 world2clip_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
//layout(location=2) in vec3 ambient_in;
layout(location=2) in vec3 diffuse_in;
//layout(location=4) in vec3 specular_in;
//layout(location=5) in float shininess_in;
//layout(location=6) in float opacity_in;
layout(location=3) in mat4 model2world_matrix;
//layout(location=11) in vec3 team_color_in;

out vec3 vertex_position;
out vec3 vertex_normal;
//out vec3 vertex_ambient_color;
out vec3 vertex_diffuse_color;
//out vec3 vertex_specular_color;
//out float vertex_shininess;
//out float vertex_opacity;
//out vec3 team_color;


void main(void) {
	vertex_position = position_in;
	//vertex_normal = normalize(normal_matrix* normal_in);
	vertex_normal = normalize(normal_in);
	//vertex_ambient_color = ambient_in;
	vertex_diffuse_color = diffuse_in;
	//vertex_specular_color = specular_in;
	//vertex_shininess = shininess_in;
	//vertex_opacity = opacity_in;
	//team_color = team_color_in;

	gl_Position = world2clip_matrix * model2world_matrix * vec4(position_in, 1.0);
}
