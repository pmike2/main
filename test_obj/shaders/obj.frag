#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform sampler2DArray diffuse_texture;


in vec3 vertex_position;
in vec3 vertex_normal;
in vec3 tex_coord;


out vec4 frag_color;


void main(void) {
	vec4 sampled = texture(diffuse_texture, tex_coord);

	// ambient
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color;

	// diffuse 
	vec3 light_direction = normalize(light_position - vertex_position);
	float diff = max(dot(vertex_normal, light_direction), 0.0);
	vec3 diffuse = diff * light_color;

	// specular
	float specular_strength = 0.5;
	vec3 view_direction = normalize(view_position- vertex_position);
	vec3 reflection_direction = reflect(-light_direction, vertex_normal);
	float spec = pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular = specular_strength* spec* light_color;
	
	//vec3 result = (ambient + diffuse + specular) * vertex_color;
	vec3 result = (ambient + diffuse + specular) * vec3(sampled);
	//vec3 result = vertex_color;
	
	frag_color = vec4(result, 1.0);
	//frag_color = vec4(1.0, 0.0, 0.0, 1.0);
	//frag_color = sampled;
}
