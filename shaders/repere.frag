#version 410 core


flat in vec4 vertex_color;


out vec4 frag_color;


void main(void) {
	frag_color= vertex_color;
}
