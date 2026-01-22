#version 410 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform vec2 viewport_size;
uniform mat4 world2clip_matrix;
uniform float thickness;

in VS_OUT {
	flat vec4 vertex_color;
	flat vec3 start_pos;
	vec3 vertex_pos;
} gs_in[];

flat out vec4 vertex_color_bis;
flat out vec3 start_pos_bis;
out vec3 vertex_pos_bis;


void main() {
	vec4 p1 = gl_in[0].gl_Position;
	vec4 p2 = gl_in[1].gl_Position;

	vec2 dir = normalize((p2.xy/ p2.w - p1.xy/ p1.w) * viewport_size);
	vec2 offset = vec2(-dir.y, dir.x) * thickness / viewport_size;

	gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);
	vertex_color_bis= gs_in[0].vertex_color;
	start_pos_bis= gs_in[0].start_pos;
	vertex_pos_bis= gs_in[0].vertex_pos;
	EmitVertex();
	
	gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);
	vertex_color_bis= gs_in[0].vertex_color;
	start_pos_bis= gs_in[0].start_pos;
	vertex_pos_bis= gs_in[0].vertex_pos;
	EmitVertex();

	gl_Position = p2 + vec4(offset.xy * p2.w, 0.0, 0.0);
	vertex_color_bis= gs_in[1].vertex_color;
	start_pos_bis= gs_in[1].start_pos;
	vertex_pos_bis= gs_in[1].vertex_pos;
	EmitVertex();
	
	gl_Position = p2 - vec4(offset.xy * p2.w, 0.0, 0.0);
	vertex_color_bis= gs_in[1].vertex_color;
	start_pos_bis= gs_in[1].start_pos;
	vertex_pos_bis= gs_in[1].vertex_pos;
	EmitVertex();

	EndPrimitive();
}
