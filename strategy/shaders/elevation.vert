#version 410


uniform mat4 world2clip_matrix;
uniform sampler2DArray fow_texture_array;
uniform float idx_team;
uniform vec2 size;
uniform vec2 origin;
uniform float z_fow;
uniform float fow_active;

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

	float fow = texture(fow_texture_array, vec3((vertex_position.x - origin.x) / size.x, (vertex_position.y - origin.y) / size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	vertex_position = position_in;
	vertex_position.z = (1.0 - step(0.3, fow)) * z_fow + step(0.3, fow) * vertex_position.z;

	vertex_color = (1.0 - step(0.3, fow)) * vec4(0.5, 0.5, 0.5, 1.0)
		+ step(0.3, fow) * (1.0 - step(0.7, fow)) * mix(color_in, vec4(0.5, 0.5, 0.5, 1.0), 0.7)
		+ step(0.7, fow) * color_in;
	
	vertex_normal = (1.0 - step(0.3, fow)) * vec3(0.0, 0.0, 1.0) + step(0.3, fow) * normal_in;

	gl_Position= world2clip_matrix * vec4(vertex_position, 1.0);
}
