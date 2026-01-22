#version 410 core

flat in vec4 vertex_color_bis;

out vec4 frag_color;


void main(void) {
	frag_color= vertex_color_bis;
}
