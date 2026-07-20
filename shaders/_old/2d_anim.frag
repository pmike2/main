#version 410 core

uniform sampler2DArray texture_array;
//uniform int current_layer;
//uniform int next_layer;
//uniform float interpol_layer;

in vec2 tex_coord;
in float current_layer;
out vec4 color;

void main() {
	vec4 current_color= texture(texture_array, vec3(tex_coord.x, tex_coord.y, current_layer));
	if (current_color.a< 0.5) {
		discard;
	}
	//vec4 next_color   = texture(texture_array, vec3(tex_coord.x, tex_coord.y, next_layer));
	//vec4 mix_color= mix(current_color, next_color, interpol_layer);
	//color= vec4(mix_color.x, mix_color.y, mix_color.z, min(current_color.w, next_color.w));
	
	//color= vec4(1.0f, 0.0f, 0.0f, 1.0f);
	color= current_color;
}
