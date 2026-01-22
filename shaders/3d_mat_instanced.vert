#version 410


uniform mat4 world2clip_matrix;
uniform mat4 world2camera_matrix;


layout(location=0) in vec4 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in float idx_material_in;
layout(location=3) in mat4 instanced_matrix;


flat out vec4 vertex_position;
flat out vec3 vertex_normal;
flat out float idx_material;


void main(void)
{
	vertex_normal= normalize(mat3(world2camera_matrix* instanced_matrix)* normal_in);
	vertex_position= world2camera_matrix* instanced_matrix* position_in;

	idx_material= idx_material_in;

	gl_Position= world2clip_matrix* instanced_matrix* position_in;
}
