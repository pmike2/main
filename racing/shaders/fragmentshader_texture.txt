#version 410 core


uniform sampler2DArray texture_array;
uniform sampler2DArray texture_array_bump;
uniform float gray_blend;

in vec2 tex_coord;
in float bump;
in float current_layer;

out vec4 color;


void main() {
	vec4 current_color= texture(texture_array, vec3(tex_coord.x, tex_coord.y, current_layer));
	if (current_color.a< 0.1) {
		discard;
	}
	vec4 current_color_bump= texture(texture_array_bump, vec3(tex_coord.x, tex_coord.y, current_layer));

	vec4 mixed_color= mix(current_color, current_color_bump, bump);
	
	color= mix(mixed_color, vec4(0.2, 0.2, 0.2, 1.0), gray_blend);
}
