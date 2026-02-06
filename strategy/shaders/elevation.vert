#version 410


uniform mat4 world2clip_matrix;
uniform usampler2DArray fow_texture_array;
uniform int idx_team;
uniform vec2 elevation_size;
uniform vec2 elevation_origin;
uniform float z_fow;

layout(location=0) in vec3 position_in;
layout(location=1) in vec4 color_in;
layout(location=2) in vec3 normal_in;

out vec3 vertex_position;
out vec4 vertex_color;
out vec3 vertex_normal;


void main(void) {
	vertex_position= position_in;
	vertex_color = color_in;
	vertex_normal= normalize(normal_in);

	//uvec3 v = texture(fow_texture_array, vec3((vertex_position.x - elevation_origin.x) / elevation_size.x, (vertex_position.y - elevation_origin.y) / elevation_size.y, float(idx_team)));
	uint v = uint(texture(fow_texture_array, vec3((vertex_position.x - elevation_origin.x) / elevation_size.x, (vertex_position.y - elevation_origin.y) / elevation_size.y, float(idx_team))).r);

	/*vertex_position = position_in;
	vertex_position.z = (1.0 - step(0.5, fow)) * z_fow + step(0.5, fow) * vertex_position.z;

	vertex_color = (1.0 - step(0.5, fow)) * vec4(0.0, 0.0, 0.0, 1.0)
		+ step(0.5, fow) * (1.0 - step(1.5, fow)) * mix(color_in, vec4(0.5, 0.5, 0.5, 1.0), 0.5)
		+ step(1.5, fow) * color_in;
	
	vertex_normal = (1.0 - step(0.5, fow)) * vec3(0.0, 0.0, 1.0) + step(0.5, fow) * normal_in;*/

	gl_Position= world2clip_matrix * vec4(vertex_position, 1.0);
}
