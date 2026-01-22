#version 410 core


uniform sampler2DArray texture_array;

in vec2 tex_coord;
in float current_layer;
in float selection;

out vec4 color;


void main() {
	vec4 current_color= texture(texture_array, vec3(tex_coord.x, tex_coord.y, current_layer));
	if (current_color.a< 0.1) {
		discard;
	}
	
	color= mix(current_color, vec4(1.0, 1.0, 0.0, 1.0), selection);
}
