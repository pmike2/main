#version 410


uniform mat4 model2clip_matrix;
uniform mat4 model2camera_matrix;
uniform mat3 normal_matrix;


in vec4 position_in;
in vec4 color_in;
in vec3 normal_in;


flat out vec4 vertex_position;
smooth out vec4 vertex_color;
flat out vec3 vertex_normal;


void main(void)
{
	vertex_color= color_in;
	vertex_normal= normalize(normal_matrix* normal_in);
	vertex_position= model2camera_matrix* position_in;

	gl_Position= model2clip_matrix * position_in;
}
