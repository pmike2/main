#version 410 core

uniform sampler2DArray fow_texture_array;
uniform float idx_team;
uniform vec2 elevation_size;
uniform vec2 elevation_origin;
uniform float fow_active;


layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VS_OUT {
	vec2 vertex_position;
	vec4 vertex_diffuse_color;
} gs_in[];


out GS_OUT {
	vec2 vertex_position;
	vec4 vertex_diffuse_color;
} gs_out;


void main() {
	float fow = texture(fow_texture_array, vec3((gs_in[0].vertex_position.x - elevation_origin.x) / elevation_size.x, (gs_in[0].vertex_position.y - elevation_origin.y) / elevation_size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	gl_Position = (1.0 - step(0.3, fow)) * vec4(0.0) + step(0.3, fow) * gl_in[0].gl_Position;
	gs_out.vertex_position = gs_in[0].vertex_position;
	gs_out.vertex_diffuse_color = (1.0 - step(0.7, fow)) * mix(gs_in[0].vertex_diffuse_color, vec4(0.5, 0.5, 0.5, gs_in[0].vertex_diffuse_color.a), 0.7) + step(0.7, fow) * gs_in[0].vertex_diffuse_color;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.3, fow)) * vec4(0.0) + step(0.3, fow) * gl_in[1].gl_Position;
	gs_out.vertex_position = gs_in[1].vertex_position;
	gs_out.vertex_diffuse_color = (1.0 - step(0.7, fow)) * mix(gs_in[1].vertex_diffuse_color, vec4(0.5, 0.5, 0.5, gs_in[1].vertex_diffuse_color.a), 0.7) + step(0.7, fow) * gs_in[1].vertex_diffuse_color;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.3, fow)) * vec4(0.0) + step(0.3, fow) * gl_in[2].gl_Position;
	gs_out.vertex_position = gs_in[2].vertex_position;
	gs_out.vertex_diffuse_color = (1.0 - step(0.7, fow)) * mix(gs_in[2].vertex_diffuse_color, vec4(0.5, 0.5, 0.5, gs_in[2].vertex_diffuse_color.a), 0.7) + step(0.7, fow) * gs_in[2].vertex_diffuse_color;
	EmitVertex();
	
	EndPrimitive();
}
