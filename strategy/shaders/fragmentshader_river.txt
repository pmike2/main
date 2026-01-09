#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform float angle;

in vec3 vertex_position;
in vec4 vertex_color;
in vec3 vertex_normal_init;
in vec2 direction;

out vec4 frag_color;


void main(void) {
	//vec2 direction = normalize(vertex_position.xy);
	float dist = length(vertex_position);
	//float x = 1.0 * cos(angle + fract(dist));
	//float x = 1.0 * cos(angle + 0.1 * cos(dist));
	//float x = 1.0 * cos(angle);
	//float x = 1.0 * cos(dist + cos(angle));
	//float x = 1.0 * cos(dist) + 0.1 * cos(angle);
	float x = 1.0 * cos(dist + angle);
	//vec3 vertex_normal= normalize(vec3(x, x, 1.0));
	vec3 vertex_normal= vertex_normal_init + vec3(0.0, 0.0, 1.0) + vec3(x * direction.xy, 0.0);

	vertex_normal = normalize(vertex_normal);

	// ambient
	float ambient_strength= 0.1;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position- vertex_position);
	float diff= max(dot(vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff* light_color;

	// specular
	float specular_strength= 0.5;
	vec3 view_direction= normalize(view_position- vertex_position);
	vec3 reflection_direction= reflect(-light_direction, vertex_normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= specular_strength* spec* light_color;
	
	vec3 result= (ambient+ diffuse+ specular)* vec3(vertex_color);
	
	//frag_color= vec4(result, 1.0);
	frag_color= vec4(result, vertex_color.a);
}
