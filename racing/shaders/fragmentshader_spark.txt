#version 410 core


in float alpha;

out vec4 frag_color;


void main(void) {
	frag_color= vec4(1.0, 1.0, 0.0, alpha);
}
