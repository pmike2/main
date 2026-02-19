#version 410 core

uniform sampler2DArray fow_texture_array;
uniform float idx_team;
uniform vec2 size;
uniform vec2 origin;
uniform float fow_active;


layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
	vec3 team_color;
	float hit;
	float alpha;
} gs_in[];


out GS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
	vec3 team_color;
	float hit;
	float alpha;
} gs_out;


void main() {
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz;

	vec3 v0 = p0 - p1;
	vec3 v1 = p2 - p1;
	vec3 n = normalize(cross(v0, v1));

	float fow = texture(fow_texture_array, vec3((gs_in[0].vertex_position.x - origin.x) / size.x, (gs_in[0].vertex_position.y - origin.y) / size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * (gl_in[0].gl_Position + vec4((gs_in[0].hit + 1.0 * (1.0 - gs_in[0].alpha)) * n, 0.0));
	gs_out.vertex_position = gs_in[0].vertex_position;
	gs_out.vertex_normal = gs_in[0].vertex_normal;
	gs_out.vertex_diffuse_color = gs_in[0].vertex_diffuse_color;
	gs_out.team_color = gs_in[0].team_color;
	gs_out.hit = gs_in[0].hit;
	gs_out.alpha = gs_in[0].alpha;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * (gl_in[1].gl_Position + vec4((gs_in[1].hit + 1.0 * (1.0 - gs_in[1].alpha)) * n, 0.0));
	gs_out.vertex_position = gs_in[1].vertex_position;
	gs_out.vertex_normal = gs_in[1].vertex_normal;
	gs_out.vertex_diffuse_color = gs_in[1].vertex_diffuse_color;
	gs_out.team_color = gs_in[1].team_color;
	gs_out.hit = gs_in[1].hit;
	gs_out.alpha = gs_in[1].alpha;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * (gl_in[2].gl_Position + vec4((gs_in[2].hit + 1.0 * (1.0 - gs_in[2].alpha)) * n, 0.0));
	gs_out.vertex_position = gs_in[2].vertex_position;
	gs_out.vertex_normal = gs_in[2].vertex_normal;
	gs_out.vertex_diffuse_color = gs_in[2].vertex_diffuse_color;
	gs_out.team_color = gs_in[2].team_color;
	gs_out.hit = gs_in[2].hit;
	gs_out.alpha = gs_in[2].alpha;
	EmitVertex();
	
	EndPrimitive();
}
