#version 410


uniform mat4 world2clip_matrix;

layout(location=0) in vec3 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec3 diffuse_in;

out VS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
} vs_out;


void main(void) {
	vs_out.vertex_position = position_in;
	vs_out.vertex_normal = normalize(normal_in);
	vs_out.vertex_diffuse_color = diffuse_in;

	/*float fow = texture(fow_texture_array, vec3((vertex_position.x - elevation_origin.x) / elevation_size.x, (vertex_position.y - elevation_origin.y) / elevation_size.y, idx_team)).r;
	fow = (1.0 - step(0.5, fow_active)) * 1.0 + step(0.5, fow_active) * fow;

	vertex_position = position_in;
	vertex_position.z = (1.0 - step(0.3, fow)) * z_fow + step(0.3, fow) * vertex_position.z;

	vertex_color = (1.0 - step(0.3, fow)) * vec4(0.5, 0.5, 0.5, 1.0)
		+ step(0.3, fow) * (1.0 - step(0.7, fow)) * mix(color_in, vec4(0.5, 0.5, 0.5, 1.0), 0.7)
		+ step(0.7, fow) * color_in;
	
	vertex_normal = (1.0 - step(0.3, fow)) * vec3(0.0, 0.0, 1.0) + step(0.3, fow) * normal_in;*/

	gl_Position= world2clip_matrix * vec4(position_in, 1.0);
}
