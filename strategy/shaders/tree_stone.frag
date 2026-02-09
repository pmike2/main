#version 410


in GS_OUT {
	vec3 vertex_position;
	vec3 vertex_normal;
	vec3 vertex_diffuse_color;
} fs_in;

out vec4 frag_color;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;


void main(void) {
	// ambient
	float ambient_strength= 0.1;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position- fs_in.vertex_position);
	float diff= max(dot(fs_in.vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff* light_color;

	// specular
	float specular_strength= 0.5;
	vec3 view_direction= normalize(view_position- fs_in.vertex_position);
	vec3 reflection_direction= reflect(-light_direction, fs_in.vertex_normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= specular_strength* spec* light_color;
	
	vec3 result= (ambient + diffuse + specular)* fs_in.vertex_diffuse_color;
	
	frag_color= vec4(result, 1.0);
}
