#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform float angle;

in GS_OUT {
	vec3 vertex_position;
	vec4 vertex_diffuse_color;
	vec3 vertex_normal_init;
	vec2 direction;
} fs_in;

out vec4 frag_color;


void main(void) {
	float dist = length(fs_in.vertex_position);
	float x = 1.0 * cos(dist + angle);
	vec3 vertex_normal= fs_in.vertex_normal_init + vec3(0.0, 0.0, 1.0) + vec3(x * fs_in.direction.xy, 0.0);

	vertex_normal = normalize(vertex_normal);

	// ambient
	float ambient_strength= 0.1;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position- fs_in.vertex_position);
	float diff= max(dot(vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff* light_color;

	// specular
	float specular_strength= 0.5;
	vec3 view_direction= normalize(view_position- fs_in.vertex_position);
	vec3 reflection_direction= reflect(-light_direction, vertex_normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= specular_strength* spec* light_color;
	
	vec3 result= (ambient+ diffuse+ specular)* vec3(fs_in.vertex_diffuse_color);
	
	frag_color= vec4(result, fs_in.vertex_diffuse_color.a);
}
