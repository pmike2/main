#version 410


uniform mat4 camera2clip_matrix;
uniform float z;

layout(location=0) in vec4 pos_tex; // vec2 pos , vec2 tex

out vec2 tex_coord;


void main() {
	gl_Position = camera2clip_matrix * vec4(pos_tex.xy, z, 1.0);
	tex_coord = pos_tex.zw;
}
