#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;

in vec3 vertex_position;
in vec3 vertex_normal;
//in vec3 vertex_ambient_color;
in vec3 vertex_diffuse_color;
//in vec3 vertex_specular_color;
//in float vertex_shininess;
//in float vertex_opacity;
//in vec3 team_color;

out vec4 frag_color;


void main(void) {
	// ambient
	const float ambient_strength = 0.2;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position- vertex_position);
	float diff= max(dot(vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff * light_color;

	// specular
	const float specular_strength = 0.5;
	const float shininess = 10.0;
	vec3 view_direction = normalize(view_position- vertex_position);
	vec3 reflection_direction = reflect(-light_direction, vertex_normal);
	//float spec = pow(max(dot(view_direction, reflection_direction), 0.0), vertex_shininess);
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;
	
	//vec3 result= ambient * vertex_ambient_color + diffuse * vertex_diffuse_color + specular * vertex_specular_color;
	vec3 result= ambient * vec3(1.0, 1.0, 1.0) + diffuse * vertex_diffuse_color + specular * vertex_diffuse_color;
	
	frag_color= vec4(result, 1.0);
}
