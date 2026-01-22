#version 410


uniform vec3 light_color;
uniform sampler2DArray diffuse_texture_array;
uniform sampler2DArray normal_texture_array;

in vec2 tex_coord;
in float current_layer_diffuse;
in float current_layer_normal;
in vec3 tangent_light_position;
in vec3 tangent_view_position;
in vec3 tangent_vertex_position;

out vec4 frag_color;


void main(void) {
	vec3 diffuse_color= texture(diffuse_texture_array, vec3(tex_coord.x, tex_coord.y, current_layer_diffuse)).rgb;

	vec3 normal= texture(normal_texture_array, vec3(tex_coord.x, tex_coord.y, current_layer_normal)).rgb;
	normal= normalize(normal* 2.0- 1.0);
	
	// ambient
	float ambient_strength= 0.1;
	vec3 ambient= ambient_strength * light_color;

	// diffuse
	vec3 light_direction= normalize(tangent_light_position- tangent_vertex_position);
	float diff= max(dot(normal, light_direction), 0.0);
	vec3 diffuse= diff* light_color;

	// specular
	float specular_strength= 0.5;
	vec3 view_direction= normalize(tangent_view_position- tangent_vertex_position);
	vec3 reflection_direction= reflect(-light_direction, normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= specular_strength* spec* light_color;
	
	vec3 result= (ambient+ diffuse+ specular)* vec3(diffuse_color);
	
	frag_color= vec4(result, 1.0);
}
