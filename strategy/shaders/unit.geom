#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	//vec3 vertex_ambient_color;
	vec3 vertex_diffuse_color;
	//vec3 vertex_specular_color;
	//float vertex_shininess;
	//float vertex_opacity;
	vec3 team_color;
	float hit;
} gs_in[];


out GS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	//vec3 vertex_ambient_color;
	vec3 vertex_diffuse_color;
	//vec3 vertex_specular_color;
	//float vertex_shininess;
	//float vertex_opacity;
	vec3 team_color;
	float hit;
} gs_out;


void main() {
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz;

	vec3 v0 = p0 - p1;
	vec3 v1 = p2 - p1;
	vec3 n = normalize(cross(v0, v1));

	gl_Position = gl_in[0].gl_Position + vec4(gs_in[0].hit * n, 0.0);
	gs_out.vertex_position = gs_in[0].vertex_position;
	gs_out.vertex_normal = gs_in[0].vertex_normal;
	//gs_out.vertex_ambient_color = gs_in[0].vertex_ambient_color;
	gs_out.vertex_diffuse_color = gs_in[0].vertex_diffuse_color;
	//gs_out.vertex_specular_color = gs_in[0].vertex_specular_color;
	//gs_out.vertex_shininess = gs_in[0].vertex_shininess;
	//gs_out.vertex_opacity = gs_in[0].vertex_opacity;
	gs_out.team_color = gs_in[0].team_color;
	gs_out.hit = gs_in[0].hit;
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position + vec4(gs_in[1].hit * n, 0.0);
	gs_out.vertex_position = gs_in[1].vertex_position;
	gs_out.vertex_normal = gs_in[1].vertex_normal;
	//gs_out.vertex_ambient_color = gs_in[1].vertex_ambient_color;
	gs_out.vertex_diffuse_color = gs_in[1].vertex_diffuse_color;
	//gs_out.vertex_specular_color = gs_in[1].vertex_specular_color;
	//gs_out.vertex_shininess = gs_in[1].vertex_shininess;
	//gs_out.vertex_opacity = gs_in[1].vertex_opacity;
	gs_out.team_color = gs_in[1].team_color;
	gs_out.hit = gs_in[1].hit;
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position + vec4(gs_in[2].hit * n, 0.0);
	gs_out.vertex_position = gs_in[2].vertex_position;
	gs_out.vertex_normal = gs_in[2].vertex_normal;
	//gs_out.vertex_ambient_color = gs_in[2].vertex_ambient_color;
	gs_out.vertex_diffuse_color = gs_in[2].vertex_diffuse_color;
	//gs_out.vertex_specular_color = gs_in[2].vertex_specular_color;
	//gs_out.vertex_shininess = gs_in[2].vertex_shininess;
	//gs_out.vertex_opacity = gs_in[2].vertex_opacity;
	gs_out.team_color = gs_in[2].team_color;
	gs_out.hit = gs_in[2].hit;
	EmitVertex();
	
	EndPrimitive();
}
