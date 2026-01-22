#version 410 core


uniform mat4 model2clip_matrix;
uniform mat4 model2camera_matrix;
uniform mat3 normal_matrix;
uniform float tide;

const float tide_freq_x= 0.03;
const float tide_freq_y= 0.01;
const float tide_factor_position= 10.0;
const float tide_factor_color= 0.4;

layout(location=0) in vec4 position_in;
layout(location=1) in vec4 color_in;
layout(location=2) in vec3 normal_in;


flat out vec4 vertex_position;
smooth out vec4 vertex_color;
flat out vec3 vertex_normal;


void main(void) {
	float z_water= (1.0- step(0.0, position_in.z- 0.01))* sin(tide_freq_x* position_in.x+ tide)* sin(tide_freq_y* position_in.y+ tide);
	vec4 new_position= vec4(position_in.x, position_in.y, position_in.z+ z_water* tide_factor_position, 1.0);
	vec4 new_color= vec4(color_in.x+ z_water* tide_factor_color, color_in.y+ z_water* tide_factor_color, color_in.z+ z_water* tide_factor_color, color_in.w);

	vertex_color= new_color;
	vertex_normal= normalize(normal_matrix* normal_in);
	vertex_position= model2camera_matrix* new_position;

	gl_Position= model2clip_matrix * new_position;
}
