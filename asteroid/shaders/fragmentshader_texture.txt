#version 410 core


uniform sampler2DArray texture_array;

in vec2 tex_coord_bis;
in float current_layer_bis;
in float hit_bis;
in float alpha_bis;

out vec4 color;


void main() {
	vec4 current_color= texture(texture_array, vec3(tex_coord_bis.x, tex_coord_bis.y, current_layer_bis));
	if (current_color.a< 0.5) {
		discard;
	}
	
	// hit == 0 : pas touché ; hit == 1 : vient de se faire toucher
	color= mix(vec4(current_color.rgb, alpha_bis), vec4(1.0, 0.0, 0.0, alpha_bis), hit_bis);
}
