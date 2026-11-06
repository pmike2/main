#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform sampler2DArray diffuse_texture;


in GS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 tex_coord;
	vec3 team_color;
	float hit;
	float alpha;
} fs_in;


out vec4 frag_color;


void main(void) {
	vec4 sampled = texture(diffuse_texture, fs_in.tex_coord);

	// ambient
	const float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color;

	// diffuse
	vec3 light_direction = normalize(light_position - fs_in.vertex_position);
	float diff = max(dot(fs_in.vertex_normal, light_direction), 0.0);
	vec3 diffuse = diff * light_color;

	// specular
	// TODO : mettre en uniform ?
	const float specular_strength = 0.5;
	const float shininess = 32.0;
	vec3 view_direction = normalize(view_position- fs_in.vertex_position);
	vec3 reflection_direction = reflect(-light_direction, fs_in.vertex_normal);
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;

	/*vec3 vertex_diffuse_color_modified = fs_in.vertex_diffuse_color;
	if (fs_in.vertex_diffuse_color.r < 0.01 && fs_in.vertex_diffuse_color.g < 0.01 && fs_in.vertex_diffuse_color.b < 0.01) {
		vertex_diffuse_color_modified = fs_in.team_color;
	}
	vec3 result= ambient * vec3(1.0, 1.0, 1.0) + diffuse * vertex_diffuse_color_modified + specular * vertex_diffuse_color_modified;*/

	vec3 result = (ambient + diffuse + specular) * vec3(sampled);	
	
	result = mix(result, vec3(1.0, 0.0, 0.0), fs_in.hit);
	
	//frag_color = vec4(result, fs_in.alpha);
	frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
