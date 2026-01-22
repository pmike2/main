#version 410 core


uniform sampler2DArray texture_array;

in vec2 tex_coord;
in float alpha;
in float current_layer;

out vec4 color;


void main() {
	vec4 current_color = texture(texture_array, vec3(tex_coord.x, tex_coord.y, current_layer));
	if (current_color.a < 0.01) {
		discard;
	}
	if (alpha < 0.01) {
		discard;
	}
	
	color= vec4(current_color.rgb, alpha);
}
