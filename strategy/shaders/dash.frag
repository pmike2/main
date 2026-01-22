#version 410 core

uniform vec2 viewport_size;
uniform float dash_size;
uniform float gap_size;


flat in vec4 vertex_color_bis;
flat in vec3 start_pos_bis;
in vec3 vertex_pos_bis;

out vec4 frag_color;


void main(void) {
	vec2 dir = (vertex_pos_bis.xy - start_pos_bis.xy) * viewport_size * 0.5;
	float dist = length(dir);

	if (fract(dist / (dash_size + gap_size)) > dash_size / (dash_size + gap_size)) {
		discard;
	}

	frag_color= vertex_color_bis;
}
