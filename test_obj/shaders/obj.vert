#version 410


uniform mat4 world2clip_matrix;
uniform mat4 model2world_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 tex_coord_in;

out vec3 vertex_position;
out vec3 vertex_normal;
out vec3 tex_coord;


void main(void) {
	vertex_position = position_in;
	vertex_normal = mat3(model2world_matrix) * normalize(normal_in);
	tex_coord = tex_coord_in;

	gl_Position = world2clip_matrix * model2world_matrix * vec4(vertex_position, 1.0);
}
