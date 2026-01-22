#version 410 core

uniform sampler2D tex;
uniform float alpha;

in vec2 tex_coord;
out vec4 color;

void main() {
	vec4 sampled= texture(tex, tex_coord);
	if (sampled.a< 0.5) {
		discard;
	}

	//color= vec4(sampled.x, sampled.y, sampled.z, sampled.w* alpha);
	color= sampled;
	//color= vec4(1.0, 0.0, 0.0, 1.0);
}
