

#include "repere.h"


Repere::Repere(){
}


Repere::Repere(GLuint prog_draw_){
	float data_[]= {
		0., 0., 0., 1., 0., 0.,
		1., 0., 0., 1., 0., 0.,
		0., 0., 0., 0., 1., 0.,
		0., 1., 0., 0., 1., 0.,
		0., 0., 0., 0., 0., 1.,
		0., 0., 1., 0., 0., 1.,
	};
	for (int i=0; i<36; i++)
		data[i]= data_[i];
	
	is_active= false;
	
	prog_draw= prog_draw_;

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	
	position_loc= glGetAttribLocation(prog_draw, "position_in");
	diffuse_color_loc= glGetAttribLocation(prog_draw, "color_in");
	world2clip_loc= glGetUniformLocation(prog_draw, "world2clip_matrix");
}


void Repere::draw(float * world2clip){
	if (is_active){	
		glUseProgram(prog_draw);
		glUniformMatrix4fv(world2clip_loc, 1, GL_FALSE, world2clip);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
		glDrawArrays(GL_LINES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glUseProgram(0);
	}

}


