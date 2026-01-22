#version 410

uniform sampler2DArray diffuse_texture_array;

in vec2 tex_coord;
in float current_layer;

out vec4 color;


void main() {
	vec4 sampled= texture(diffuse_texture_array, vec3(tex_coord.x, tex_coord.y, current_layer));
	if (sampled.a< 0.5) {
		discard;
	}

	color= sampled;
}
