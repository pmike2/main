
#version 410


uniform mat4 world2clip_matrix;


layout(location=0) in vec3 position_in;
layout(location=1) in vec4 color_in;
layout(location=2) in vec3 normal_in;
layout(location=3) in vec2 direction_in;

out VS_OUT {
	vec3 vertex_position;
	vec4 vertex_diffuse_color;
	vec3 vertex_normal_init;
	vec2 direction;
} vs_out;


void main(void) {
	vs_out.vertex_position= position_in;
	vs_out.vertex_diffuse_color= color_in;
	vs_out.vertex_normal_init = normal_in;
	vs_out.direction = direction_in;

	gl_Position= world2clip_matrix * vec4(position_in, 1.0);
}
