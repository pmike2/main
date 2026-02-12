#version 410


uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform float angle;
uniform float amplitude;
uniform float freq;
uniform float sea_level;
uniform float shininess;


in GS_OUT {
	vec2 vertex_position;
	vec4 vertex_diffuse_color;
} fs_in;

out vec4 frag_color;


void main(void) {
	vec2 direction = normalize(fs_in.vertex_position);
	float dist = length(fs_in.vertex_position);
	float x = amplitude * cos(freq * dist + angle);
	vec3 vertex_normal= normalize(vec3(0.0, 0.0, 1.0) + vec3(x * direction, 0.0));

	// ambient
	float ambient_strength= 0.4;
	vec3 ambient= ambient_strength * light_color;

	// diffuse 
	vec3 light_direction= normalize(light_position- vec3(fs_in.vertex_position, sea_level));
	float diff= max(dot(vertex_normal, light_direction), 0.0);
	vec3 diffuse= diff* light_color;

	// specular
	vec3 view_direction= normalize(view_position- vec3(fs_in.vertex_position, sea_level));
	vec3 reflection_direction= reflect(-light_direction, vertex_normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= shininess * spec * light_color;
	
	vec3 result= (ambient+ diffuse+ specular)* vec3(fs_in.vertex_diffuse_color);
	
	frag_color= vec4(result, fs_in.vertex_diffuse_color.a);
}
