#version 410 core

// line width :
// https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
// si je veux faire des jolies jonctions entre segments :
// https://stackoverflow.com/questions/3484260/opengl-line-width/59688394


uniform float z;


layout(location=0) in vec2 position_in;
layout(location=1) in vec4 color_in;


out VS_OUT {
	flat vec4 vertex_color;
} vs_out;


void main(void) {
	vs_out.vertex_color = color_in;

	gl_Position = vec4(position_in.xy, z, 1.0);
}
