#version 410


uniform mat4 world2clip_matrix;
/*uniform sampler2DArray fow_texture_array;
uniform float idx_team;
uniform vec2 elevation_size;
uniform vec2 elevation_origin;
uniform float z_fow;
uniform float fow_active;*/

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 diffuse_in;
layout(location=3) in mat4 model2world_matrix;
layout(location=7) in vec3 team_color_in;
layout(location=8) in float hit_in;


out VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
	vec3 team_color;
	float hit;
} vs_out;


void main(void) {
	//vs_out.vertex_position = position_in;
	vs_out.vertex_position = vec3(model2world_matrix * vec4(position_in.xyz, 1.0));
	vs_out.vertex_normal = normalize(normal_in);
	vs_out.vertex_diffuse_color = diffuse_in;
	vs_out.team_color = team_color_in;
	vs_out.hit = hit_in;

	/*vec3 world_position = vec3(model2world_matrix * vec4(position_in.xyz, 1.0));

	float fow = texture(fow_texture_array, vec3((world_position.x - elevation_origin.x) / elevation_size.x, (world_position.y - elevation_origin.y) / elevation_size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	mat4 model2world_matrix_fow = (1.0 - step(0.7, fow)) * mat4(0.0)+ step(0.7, fow) * model2world_matrix;

	gl_Position = world2clip_matrix * model2world_matrix_fow * vec4(position_in, 1.0);*/
	gl_Position = world2clip_matrix * model2world_matrix * vec4(position_in, 1.0);
}
