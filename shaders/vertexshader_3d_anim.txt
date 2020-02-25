#version 410 core

const int n_joints= 128;

uniform mat4 model2clip_matrix;
uniform mat4 model2camera_matrix;
uniform mat3 normal_matrix;
uniform mat4 joints[n_joints];


layout(location=0) in vec4 position_in;
layout(location=1) in vec3 normal_in;
layout(location=2) in vec4 color_in;
layout(location=3) in vec4 indices;
layout(location=4) in vec4 weights;


flat out vec4 vertex_position;
smooth out vec4 vertex_color;
flat out vec3 vertex_normal;


void main(void) {
	vec4 new_position;
	vec4 new_normal;
	int index;

/*
	new_position=position_in;
	new_normal=vec4(normal_in, 0.0);
*/

	index= int(indices.x);
	new_position= (joints[index]* position_in)* weights.x;
	new_normal= (joints[index]* vec4(normal_in, 0.0))* weights.x;
	
	index= int(indices.y);
	new_position= new_position+ (joints[index]* position_in)* weights.y;
	new_normal= new_normal+ (joints[index]* vec4(normal_in, 0.0))* weights.y;

	index= int(indices.z);
	new_position= new_position+ (joints[index]* position_in)* weights.z;
	new_normal= new_normal+ (joints[index]* vec4(normal_in, 0.0))* weights.z;

	index= int(indices.w);
	new_position= new_position+ (joints[index]* position_in)* weights.w;
	new_normal= new_normal+ (joints[index]* vec4(normal_in, 0.0))* weights.w;

	//new_position= vec4(new_position.x* scale.x, new_position.y* scale.y, new_position.z* scale.z, 1.0);
	//new_normal= vec4(new_normal.x* scale.x, new_normal.y* scale.y, new_normal.z* scale.z, 1.0); // est-ce necessaire ?

	vertex_position= model2camera_matrix* new_position;
	vertex_normal= normalize(normal_matrix* vec3(new_normal));
	vertex_color= color_in;

	gl_Position= model2clip_matrix * vec4(new_position.xyz, 1.0); // on force w (qui pourrait différer legerement de 1.0) a 1.0
}
