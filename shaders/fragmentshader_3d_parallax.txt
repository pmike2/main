#version 410


uniform vec3 light_color;
uniform sampler2DArray diffuse_texture_array;
uniform sampler2DArray normal_texture_array;
uniform sampler2DArray parallax_texture_array;
uniform float height_scale;

in vec2 tex_coord;
in float current_layer_diffuse;
in float current_layer_normal;
in float current_layer_parallax;
in vec3 tangent_light_position;
in vec3 tangent_view_position;
in vec3 tangent_vertex_position;

out vec4 frag_color;


vec2 parallax_mapping(vec2 tex_coord, vec3 view_direction) {
	float height= texture(parallax_texture_array, vec3(tex_coord.x, tex_coord.y, current_layer_parallax)).r;
	vec2 p= view_direction.xy/ view_direction.z* (height* height_scale);
	return tex_coord- p;
}


vec2 parallax_mapping_v2(vec2 tex_coord, vec3 view_direction) {
	// number of depth layers
	const float num_layers= 10.0;
	// calculate the size of each layer
	float layer_depth= 1.0 / num_layers;
	// depth of current layer
	float current_layer_depth= 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 p= view_direction.xy * height_scale; 
	vec2 delta_tex_coords= p/ num_layers;
	// get initial values
	vec2 current_tex_coord= tex_coord;
	float current_depth_map_value= texture(parallax_texture_array, vec3(current_tex_coord.x, current_tex_coord.y, current_layer_parallax)).r;
	
	while (current_layer_depth< current_depth_map_value)	{
		// shift texture coordinates along direction of P
		current_tex_coord-= delta_tex_coords;
		// get depthmap value at current texture coordinates
		current_depth_map_value= texture(parallax_texture_array, vec3(current_tex_coord.x, current_tex_coord.y, current_layer_parallax)).r;
		// get depth of next layer
		current_layer_depth+= layer_depth;
	}

	return current_tex_coord;
}

void main(void) {
	vec3 view_direction= normalize(tangent_view_position- tangent_vertex_position);

	vec2 tex_coord_para= parallax_mapping(tex_coord, view_direction);

	vec3 diffuse_color= texture(diffuse_texture_array, vec3(tex_coord_para.x, tex_coord_para.y, current_layer_diffuse)).rgb;

	vec3 normal= texture(normal_texture_array, vec3(tex_coord_para.x, tex_coord_para.y, current_layer_normal)).rgb;
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
	vec3 reflection_direction= reflect(-light_direction, normal);
	float spec= pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	vec3 specular= specular_strength* spec* light_color;
	
	vec3 result= (ambient+ diffuse+ specular)* vec3(diffuse_color);
	
	frag_color= vec4(result, 1.0);
}
