#version 410 core


in vec3 vertex_color;
in vec2 tex_coord;

out vec4 frag_color;


void main(void) {
	float dx= tex_coord.x- 0.5;
	float dy= tex_coord.y- 0.5;
	float dist= dx* dx+ dy* dy;

	if (dist> 0.25) {
		discard;
	}

	frag_color= mix(vec4(vertex_color, 1.0), vec4(vertex_color, 0.0), smoothstep(0.18, 0.25, dist));
}
