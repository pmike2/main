#version 410


uniform mat4 camera2clip_matrix;

in vec2 position_in;
in vec2 tex_coord_in;
in float current_layer_in;
in float hit_in;
in float alpha_in;
in float rotation_in;
in float scale_in;
in vec2 center_in;


out VS_OUT {
	vec2 tex_coord;
	float current_layer;
	float hit;
	float alpha;
} vs_out;


void main() {
	float x= center_in.x+ scale_in* ((position_in.x- center_in.x)* cos(rotation_in)- (position_in.y- center_in.y)* sin(rotation_in));
	float y= center_in.y+ scale_in* ((position_in.x- center_in.x)* sin(rotation_in)+ (position_in.y- center_in.y)* cos(rotation_in));
	// les ships sont à z == 0.0
	gl_Position= camera2clip_matrix* vec4(x, y, 0.0, 1.0);
	vs_out.tex_coord= tex_coord_in;
	vs_out.current_layer= current_layer_in;
	vs_out.hit= hit_in;
	vs_out.alpha= alpha_in;
}
