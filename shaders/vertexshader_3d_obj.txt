#version 410


uniform mat4 model2clip_matrix;
uniform mat4 model2camera_matrix;
uniform mat3 normal_matrix;


layout(location=0) in vec4 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in float idx_material_in;


flat out vec4 vertex_position;
flat out vec3 vertex_normal;
flat out float idx_material;


void main(void) {

	vertex_normal= normalize(normal_matrix* normal_in);
	vertex_position= model2camera_matrix* position_in;

	idx_material= idx_material_in;

	gl_Position= model2clip_matrix * position_in;
}
