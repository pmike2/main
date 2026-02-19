#version 410


uniform mat4 world2clip_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 diffuse_in;
layout(location=3) in mat4 model2world_matrix;
layout(location=7) in vec3 team_color_in;
layout(location=8) in float hit_in;
layout(location=9) in float alpha_in;


out VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
	vec3 team_color;
	float hit;
	float alpha;
} vs_out;


void main(void) {
	vs_out.vertex_position = vec3(model2world_matrix * vec4(position_in.xyz, 1.0));
	vs_out.vertex_normal = normalize(normal_in);
	vs_out.vertex_diffuse_color = diffuse_in;
	vs_out.team_color = team_color_in;
	vs_out.hit = hit_in;
	vs_out.alpha = alpha_in;

	gl_Position = world2clip_matrix * model2world_matrix * vec4(position_in, 1.0);
}
