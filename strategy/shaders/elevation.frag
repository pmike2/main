#version 410

uniform sampler2DArray diffuse_texture;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_position;
uniform float fog_start;
uniform float fog_end;
uniform vec4 fog_color;

in vec3 vertex_position;
flat in vec4 vertex_color;
flat in vec3 vertex_normal;
in vec2 tex_coord;
flat in float tex_idx;
flat in float fow;

out vec4 frag_color;


void main(void) {
	vec4 sampled = texture(diffuse_texture, vec3(tex_coord, tex_idx));

	sampled = (1.0 - step(0.3, fow)) * vec4(0.5, 0.5, 0.5, 1.0)
		+ step(0.3, fow) * (1.0 - step(0.7, fow)) * mix(sampled, vec4(0.5, 0.5, 0.5, 1.0), 0.7)
		+ step(0.7, fow) * sampled;

	// ambient
	float ambient_strength = 0.01;
	vec3 ambient = ambient_strength * light_color * vec3(vertex_color);

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
	
	vec3 result = (ambient + diffuse + specular) * vec3(sampled);
	//vec3 result = (ambient + diffuse + specular) * vec3(vertex_color);
	//vec3 result = (ambient + diffuse)* vec3(vertex_color);
	//vec3 result = (specular)* vec3(vertex_color);

	// brouillard
	float fog_factor = (fog_end - length(view_position- vertex_position)) / (fog_end - fog_start);
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	result = mix(vec3(fog_color), result, fog_factor);
	
	frag_color = vec4(result, 1.0);
	//frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
