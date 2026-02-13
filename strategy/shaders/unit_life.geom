#version 410 core

uniform sampler2DArray fow_texture_array;
uniform float idx_team;
uniform vec2 size;
uniform vec2 origin;
uniform float fow_active;


layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VS_OUT {
	vec2 unit_position;
	vec4 color;
} gs_in[];


out GS_OUT {
	vec4 color;
} gs_out;


void main() {
	float fow = texture(fow_texture_array, vec3((gs_in[0].unit_position.x - origin.x) / size.x, (gs_in[0].unit_position.y - origin.y) / size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * gl_in[0].gl_Position;
	gs_out.color = gs_in[0].color;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * gl_in[1].gl_Position;
	gs_out.color = gs_in[1].color;
	EmitVertex();
	
	gl_Position = (1.0 - step(0.7, fow)) * vec4(0.0) + step(0.7, fow) * gl_in[2].gl_Position;
	gs_out.color = gs_in[2].color;
	EmitVertex();
	
	EndPrimitive();
}
