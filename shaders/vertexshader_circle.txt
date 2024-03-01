#version 410 core


uniform mat4 world2clip_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 color_in;
layout(location=2) in vec2 tex_coord_in;

out vec3 vertex_color;
out vec2 tex_coord;


void main(void) {
	gl_Position= world2clip_matrix* vec4(position_in, 1.0);
	vertex_color= color_in;
	tex_coord= tex_coord_in;
}
