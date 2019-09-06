#version 410 core


uniform samplerCube skybox;

in vec3 text_coords;

out vec4 frag_color;


void main() {    
	frag_color = texture(skybox, text_coords);
}
