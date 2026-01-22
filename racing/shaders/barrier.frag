#version 410 core


in vec4 vertex_color;
in float lambda;


out vec4 frag_color;

// lambda vaut 0.0 en bas d'une barrière et 1.0 en haut ; on veut mapper ça sur bords = 0.0 et centre = 1.0
void main(void) {
	float luminosity= 1.0- 4.0* (lambda- 0.5)* (lambda- 0.5);
	frag_color= vec4(luminosity* vertex_color.r, luminosity* vertex_color.g, luminosity* vertex_color.b, vertex_color.a);
}
