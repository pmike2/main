#version 410 core


uniform sampler2D map_texture;
uniform float alpha;

in vec2 tex_coord;

out vec4 color;


void main() {
	vec4 current_color = texture(map_texture, tex_coord);
	color = vec4(current_color.rgb, alpha);
	//color = vec4(1.0, 0.0, alpha, 1.0);
}
