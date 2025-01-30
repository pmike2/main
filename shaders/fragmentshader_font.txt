#version 410 core

uniform sampler2DArray texture_array;

in vec2 tex_coords;
in vec4 color;
in float current_layer;

out vec4 color_out;


void main() {
	vec4 sampled= vec4(1.0, 1.0, 1.0, texture(texture_array, vec3(tex_coords.rg, current_layer)).r);
	
	color_out= color* sampled;

	//color_out= mix(vec4(1.0, 0.0, 0.0, 1.0), color* sampled, 0.5);
	//color_out= sampled;
	//color_out= vec4(1.0, 0.0, 0.0, 1.0);
}
