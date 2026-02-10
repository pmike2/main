#version 410 core

uniform mat4 camera2clip_matrix;
uniform float z;

layout(location=0) in vec4 vertex_in; // <vec2 pos, vec2 tex>
layout(location=1) in vec4 color_in;
layout(location=2) in float current_layer_in;

out vec2 tex_coords;
out vec4 color;
out float current_layer;


void main() {
	gl_Position= camera2clip_matrix* vec4(vertex_in.xy, z, 1.0);
	tex_coords= vertex_in.zw;
	color= color_in;
	current_layer= current_layer_in;
}
