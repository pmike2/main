#version 410 core

// pointillés :
// cf https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3

// line width :
// https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
// si je veux faire des jolies jonctions entre segments :
// https://stackoverflow.com/questions/3484260/opengl-line-width/59688394


uniform mat4 world2clip_matrix;


layout(location=0) in vec3 position_in;
layout(location=1) in vec4 color_in;


out VS_OUT {
	flat vec4 vertex_color;
	flat vec3 start_pos;
	vec3 vertex_pos;
} vs_out;


void main(void) {
	vec4 pos = world2clip_matrix* vec4(position_in, 1.0);

	vs_out.vertex_color = color_in;
	vs_out.vertex_pos = pos.xyz / pos.w;
	vs_out.start_pos = pos.xyz / pos.w;

	gl_Position = pos;
}
