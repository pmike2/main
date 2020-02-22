
#include "vmat.h"

using namespace std;


VMat::VMat(struct timeval t_){
	is_active= true;
	t= t_;
}


void VMat::anim() {
	if (!is_active)
		return;
	
	unsigned int i;
	unsigned int diff_ms= diff_time_ms_from_now(&t);
	float morph= (float)(diff_ms)/ (float)(config->lifetime); // entre 0 (initial) et 1 (final)
	float morph_alpha= morph;
	float alpha;
	if (morph> 1.0) {
		is_active= false;
		return;
	}

	// modif de morph en fonction des morph_pos
	for (i=1; i<config->morph_pos.size(); i++)
		if (config->morph_pos[i].x> morph)
			break;
	morph= config->morph_pos[i- 1].y+ (morph- config->morph_pos[i- 1].x)* 
		(config->morph_pos[i].y- config->morph_pos[i- 1].y)/ (config->morph_pos[i].x- config->morph_pos[i- 1].x);

	for (i=0; i<16; i++)
		current_values.mat[i]= (1.0- morph)* config->init_values.mat[i]+ morph* config->final_values.mat[i];

	for (i=1; i<config->alpha_pos.size(); i++)
		if (config->alpha_pos[i].x> morph_alpha)
			break;
	alpha= config->alpha_pos[i- 1].y+ (morph_alpha- config->alpha_pos[i- 1].x)* 
		(config->alpha_pos[i].y- config->alpha_pos[i- 1].y)/ (config->alpha_pos[i].x- config->alpha_pos[i- 1].x);
	for (i=0; i<16; i++) {
		if ((i==3) || (i==7) || (i==11))
			current_values.mat[i]= 0.0;
		else if (i==15)
			current_values.mat[i]= 1.0;
		else if ((i==0) || (i==5) || (i==10))
			current_values.mat[i]= alpha* current_values.mat[i]+ (1.0- alpha);
		else
			current_values.mat[i]= alpha* current_values.mat[i];
	}
		
	gram_schmidt(current_values.mat);
}


void VMat::set_config(VMatConfig * config_) {
	config= config_;
}


void VMat::print() {
	unsigned int i;

	cout << "VMat -----------------------------" << endl;
	cout << "current_values.mat= (attention transposee)" << endl;
	for (i=0; i<16; i++) {
		cout << current_values.mat[i];
		if (i!= 15)
			cout << " , ";
		if ((i+ 1)% 4== 0)
			cout << endl;
	}
	cout << "is_active=" << is_active << endl;
	cout << "config->lifetime=" << config->lifetime << endl;
	cout << "-----------------------------" << endl;
}
