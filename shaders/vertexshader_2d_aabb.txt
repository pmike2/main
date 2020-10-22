#version 410


uniform mat4 camera2clip_matrix;
uniform mat4 model2world_matrix;
uniform float z;

in vec2 position_in;
in vec3 color_in;

out vec4 vertex_color;


void main(void)
{
	vertex_color= vec4(color_in, 1.0);
	gl_Position= camera2clip_matrix* model2world_matrix* vec4(position_in.rg, z, 1.0);
}
