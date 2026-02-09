#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;


in GS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	//vec3 vertex_ambient_color;
	vec3 vertex_diffuse_color;
	//vec3 vertex_specular_color;
	//float vertex_shininess;
	//float vertex_opacity;
} fs_in;

out vec4 frag_color;


void main(void) {
	// ambient
	const float ambient_strength = 0.2;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position - fs_in.vertex_position);
	float diff= max(dot(fs_in.vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff * light_color;

	// specular
	const float specular_strength = 0.5;
	const float shininess = 10.0;
	vec3 view_direction = normalize(view_position- fs_in.vertex_position);
	vec3 reflection_direction = reflect(-light_direction, fs_in.vertex_normal);
	//float spec = pow(max(dot(view_direction, reflection_direction), 0.0), fs_in.vertex_shininess);
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;
	
	vec3 result= ambient * vec3(1.0, 1.0, 1.0) + diffuse * fs_in.vertex_diffuse_color + specular * fs_in.vertex_diffuse_color;
	
	frag_color= vec4(result, 1.0);
}
