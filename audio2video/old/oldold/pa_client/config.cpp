
#include "config.h"

using namespace std;


// --------------------------------------------------------------------------------------------
Pt::Pt() {
	x= 0.0;
	y= 0.0;
}

Pt::Pt(float x_, float y_) {
	x= x_;
	y= y_;
}

// --------------------------------------------------------------------------------------------

ActionConfig::ActionConfig() {
}

ActionConfig::~ActionConfig() {
}


// --------------------------------------------------------------------------------------------
ModelObjConfig::ModelObjConfig() {
	type= LOAD_OBJ;
	active= true;
	retrig= true;
	defaultize();
}


ostream & operator << (ostream & out_stream, ModelObjConfig model_obj) {
	unsigned int i;
	
	out_stream << "MODEL_OBJ" << DELIM_ACTION;

	out_stream << model_obj.active << DELIM_ACTION << model_obj.retrig << DELIM_ACTION << model_obj.lifetime << DELIM_ACTION;
	out_stream << model_obj.morph_pos.size() << DELIM_ACTION;
	for (i=0; i<model_obj.morph_pos.size(); i++)
		out_stream << model_obj.morph_pos[i].x << DELIM_ACTION << model_obj.morph_pos[i].y << DELIM_ACTION;
	out_stream << model_obj.alpha_pos.size() << DELIM_ACTION;
	for (i=0; i<model_obj.alpha_pos.size(); i++)
		out_stream << model_obj.alpha_pos[i].x << DELIM_ACTION << model_obj.alpha_pos[i].y << DELIM_ACTION;

	out_stream << model_obj.ch_obj << DELIM_ACTION << model_obj.ch_mat << DELIM_ACTION;


	for (i=0; i<16; i++)
		out_stream << model_obj.init_values.model2world[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << model_obj.init_values.ambient[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << model_obj.init_values.diffuse[i] << DELIM_ACTION;
	out_stream << model_obj.init_values.shininess << DELIM_ACTION;

	for (i=0; i<16; i++)
		out_stream << model_obj.final_values.model2world[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << model_obj.final_values.ambient[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << model_obj.final_values.diffuse[i] << DELIM_ACTION;
	out_stream << model_obj.final_values.shininess << DELIM_ACTION;
	
	return out_stream;
}


istream & operator >> (istream & in_stream, ModelObjConfig & model_obj) {
	string type;
	char c;
	bool ok= 1;
	unsigned int i, j;
	float x, y;
	ModelObjConfig model_obj_tmp;
	
	ok= getline(in_stream, type, DELIM_ACTION);
	if (type!= "MODEL_OBJ")
		ok= 0;

	in_stream >> model_obj_tmp.active >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> model_obj_tmp.retrig >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> model_obj_tmp.lifetime >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> j >> c; // j == morph_pos.size()
	if (c!= DELIM_ACTION) ok= 0;
	model_obj_tmp.morph_pos.clear();
	for (i=0; i<j; i++) {
		in_stream >> x >> c;
		if (c!= DELIM_ACTION) ok= 0;
		in_stream >> y >> c;
		if (c!= DELIM_ACTION) ok= 0;
		model_obj_tmp.morph_pos.push_back(Pt(x, y));
	}
	in_stream >> j >> c; // j == alpha_pos.size()
	if (c!= DELIM_ACTION) ok= 0;
	model_obj_tmp.alpha_pos.clear();
	for (i=0; i<j; i++) {
		in_stream >> x >> c;
		if (c!= DELIM_ACTION) ok= 0;
		in_stream >> y >> c;
		if (c!= DELIM_ACTION) ok= 0;
		model_obj_tmp.alpha_pos.push_back(Pt(x, y));
	}

	ok= getline(in_stream, model_obj_tmp.ch_obj, DELIM_ACTION);
	ok= getline(in_stream, model_obj_tmp.ch_mat, DELIM_ACTION);
	
	for (i=0; i<16; i++) {
		in_stream >> model_obj_tmp.init_values.model2world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> model_obj_tmp.init_values.ambient[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> model_obj_tmp.init_values.diffuse[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	in_stream >> model_obj_tmp.init_values.shininess >> c;
	if (c!= DELIM_ACTION) ok= 0;

	for (i=0; i<16; i++) {
		in_stream >> model_obj_tmp.final_values.model2world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> model_obj_tmp.final_values.ambient[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> model_obj_tmp.final_values.diffuse[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	in_stream >> model_obj_tmp.final_values.shininess >> c;
	if (c!= DELIM_ACTION) ok= 0;

	if (ok) {
		model_obj.active= model_obj_tmp.active;
		model_obj.retrig= model_obj_tmp.retrig;
		model_obj.lifetime= model_obj_tmp.lifetime;
		model_obj.morph_pos.clear();
		for (i=0; i<model_obj_tmp.morph_pos.size(); i++)
			model_obj.morph_pos.push_back(model_obj_tmp.morph_pos[i]);
		model_obj.alpha_pos.clear();
		for (i=0; i<model_obj_tmp.alpha_pos.size(); i++)
			model_obj.alpha_pos.push_back(model_obj_tmp.alpha_pos[i]);
		
		model_obj.ch_obj= model_obj_tmp.ch_obj;
		model_obj.ch_mat= model_obj_tmp.ch_mat;
		for (i=0; i<16; i++) {
			model_obj.init_values.model2world[i] = model_obj_tmp.init_values.model2world[i];
			model_obj.final_values.model2world[i]= model_obj_tmp.final_values.model2world[i];
		}
		for (i=0; i<3; i++) {
			model_obj.init_values.ambient[i] = model_obj_tmp.init_values.ambient[i];
			model_obj.init_values.diffuse[i] = model_obj_tmp.init_values.diffuse[i];
			model_obj.final_values.ambient[i]= model_obj_tmp.final_values.ambient[i];
			model_obj.final_values.diffuse[i]= model_obj_tmp.final_values.diffuse[i];
		}
		model_obj.init_values.shininess = model_obj_tmp.init_values.shininess;
		model_obj.final_values.shininess= model_obj_tmp.final_values.shininess;
	}
	else {
		cout << "Erreur >> ModelObjConfig" << endl;
	}
	
	return in_stream;
}


void ModelObjConfig::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void ModelObjConfig::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void ModelObjConfig::randomize() {
	unsigned int i, j;

	lifetime= rand_int(100, 2000);

	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, rand_double(0.0, 1.0)));
	j= rand_int(2, 8);
	for (i=1; i<j; i++)
		morph_pos.push_back(Pt(float(i)/ float(j), rand_double(0.0, 1.0)));
	morph_pos.push_back(Pt(1.0, rand_double(0.0, 1.0)));

	alpha_pos.clear();
	alpha_pos.push_back(Pt(0.0, rand_double(0.0, 1.0)));
	j= rand_int(2, 8);
	for (i=1; i<j; i++)
		alpha_pos.push_back(Pt(float(i)/ float(j), rand_double(0.0, 1.0)));
	alpha_pos.push_back(Pt(1.0, rand_double(0.0, 1.0)));
	
	vector<string> modeles= list_files(ROOT_MODELES, "obj");
	i= rand_int(0, modeles.size()- 1);
	ch_obj= "../modeles/"+ modeles[i];
	vector<string> mats= list_files(ROOT_MATS, "mtl");
	i= rand_int(0, mats.size()- 1);
	ch_mat= "../modeles/"+ mats[i];

	for (i=0; i<16; i++)
		init_values.model2world[i]= rand_double(-1., 1.);
	init_values.model2world[3]= init_values.model2world[7]= init_values.model2world[11]= 0.0;
	init_values.model2world[15]= 1.0;
	for (i=0; i<3; i++) {
		init_values.ambient[i]= rand_double(0., 1.);
		init_values.diffuse[i]= rand_double(0., 1.);
	}
	init_values.shininess= rand_double(10., 100.);

	for (i=0; i<16; i++)
		final_values.model2world[i]= rand_double(-1., 1.);
	final_values.model2world[3]= final_values.model2world[7]= final_values.model2world[11]= 0.0;
	final_values.model2world[15]= 1.0;
	for (i=0; i<3; i++) {
		final_values.ambient[i]= rand_double(0., 1.);
		final_values.diffuse[i]= rand_double(0., 1.);
	}
	final_values.shininess= rand_double(10., 100.);
}


void ModelObjConfig::defaultize() {
	unsigned int i;

	lifetime= 1000;

	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, 0.0));
	morph_pos.push_back(Pt(1.0, 1.0));

	alpha_pos.clear();
	alpha_pos.push_back(Pt(0.0, 0.0));
	alpha_pos.push_back(Pt(0.2, 1.0));
	alpha_pos.push_back(Pt(0.8, 1.0));
	alpha_pos.push_back(Pt(1.0, 0.0));

	vector<string> modeles= list_files(ROOT_MODELES, "obj");
	ch_obj= "../modeles/"+ modeles[0];
	vector<string> mats= list_files(ROOT_MATS, "mtl");
	ch_mat= "../modeles/"+ mats[0];

	for (i=0; i<16; i++)
		init_values.model2world[i]= 0.0;
	for (i=0; i<4; i++)
		init_values.model2world[5* i]= 1.0;
	for (i=0; i<3; i++) {
		init_values.ambient[i]= 1.0;
		init_values.diffuse[i]= 1.0;
	}
	init_values.shininess= 50.0;

	for (i=0; i<16; i++)
		final_values.model2world[i]= 0.0;
	for (i=0; i<4; i++)
		final_values.model2world[5* i]= 1.0;
	for (i=0; i<3; i++) {
		final_values.ambient[i]= 1.0;
		final_values.diffuse[i]= 1.0;
	}
	final_values.shininess= 50.0;
}


// --------------------------------------------------------------------------------------------
LightConfig::LightConfig() {
	type= LOAD_LIGHT;
	active= true;
	retrig= true;
	defaultize();
}


ostream & operator << (ostream & out_stream, LightConfig light) {
	unsigned int i;
	
	out_stream << "LIGHT" << DELIM_ACTION;

	out_stream << light.active << DELIM_ACTION << light.retrig << DELIM_ACTION << light.lifetime << DELIM_ACTION;
	out_stream << light.morph_pos.size() << DELIM_ACTION;
	for (i=0; i<light.morph_pos.size(); i++)
		out_stream << light.morph_pos[i].x << DELIM_ACTION << light.morph_pos[i].y << DELIM_ACTION;

	for (i=0; i<3; i++)
		out_stream << light.init_values.color[i] << DELIM_ACTION;
	for (i=0; i<4; i++)
		out_stream << light.init_values.position_world[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << light.init_values.spot_cone_direction_world[i] << DELIM_ACTION;

	for (i=0; i<3; i++)
		out_stream << light.final_values.color[i] << DELIM_ACTION;
	for (i=0; i<4; i++)
		out_stream << light.final_values.position_world[i] << DELIM_ACTION;
	for (i=0; i<3; i++)
		out_stream << light.final_values.spot_cone_direction_world[i] << DELIM_ACTION;
	
	return out_stream;
}


istream & operator >> (istream & in_stream, LightConfig & light) {
	string type;
	char c;
	bool ok= 1;
	unsigned int i, j;
	float x, y;
	LightConfig light_tmp;
	
	ok= getline(in_stream, type, DELIM_ACTION);
	if (type!= "LIGHT")
		ok= 0;
	
	in_stream >> light_tmp.active >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> light_tmp.retrig >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> light_tmp.lifetime >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> j >> c; // j == morph_pos.size()
	if (c!= DELIM_ACTION) ok= 0;
	light_tmp.morph_pos.clear();
	for (i=0; i<j; i++) {
		in_stream >> x >> c;
		if (c!= DELIM_ACTION) ok= 0;
		in_stream >> y >> c;
		if (c!= DELIM_ACTION) ok= 0;
		light_tmp.morph_pos.push_back(Pt(x, y));
	}
	
	for (i=0; i<3; i++) {
		in_stream >> light_tmp.init_values.color[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<4; i++) {
		in_stream >> light_tmp.init_values.position_world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> light_tmp.init_values.spot_cone_direction_world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}

	for (i=0; i<3; i++) {
		in_stream >> light_tmp.final_values.color[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<4; i++) {
		in_stream >> light_tmp.final_values.position_world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<3; i++) {
		in_stream >> light_tmp.final_values.spot_cone_direction_world[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	
	if (ok) {
		light.active= light_tmp.active;
		light.retrig= light_tmp.retrig;
		light.lifetime= light_tmp.lifetime;
		light.morph_pos.clear();
		for (i=0; i<light_tmp.morph_pos.size(); i++)
			light.morph_pos.push_back(light_tmp.morph_pos[i]);
		
		for (i=0; i<3; i++) {
			light.init_values.color[i]= light_tmp.init_values.color[i];
			light.init_values.spot_cone_direction_world[i]= light_tmp.init_values.spot_cone_direction_world[i];
			light.final_values.color[i]= light_tmp.final_values.color[i];
			light.final_values.spot_cone_direction_world[i]= light_tmp.final_values.spot_cone_direction_world[i];
		}
		for (i=0; i<4; i++) {
			light.init_values.position_world[i]= light_tmp.init_values.position_world[i];
			light.final_values.position_world[i]= light_tmp.final_values.position_world[i];
		}
	}
	else {
		cout << "Erreur >> LightConfig" << endl;
	}
	
	return in_stream;
}


void LightConfig::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void LightConfig::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void LightConfig::randomize() {
	unsigned int i, j;
	float m= 2.0;
	float decal= 0.1;
	
	lifetime= rand_int(100, 2000);
	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, rand_double(0.0, 1.0)));
	j= rand_int(2, 8);
	for (i=1; i<j; i++)
		morph_pos.push_back(Pt(float(i)/ float(j), rand_double(0.0, 1.0)));
	morph_pos.push_back(Pt(1.0, rand_double(0.0, 1.0)));

	float x_init= rand_double(-m, m);
	float y_init= rand_double(-m, m);
	float z_init= rand_double(-m, m);
	float n_inv_init= 1.0/ sqrt(x_init* x_init+ y_init* y_init+ z_init* z_init);
	float x_final= rand_double(-m, m);
	float y_final= rand_double(-m, m);
	float z_final= rand_double(-m, m);
	float n_inv_final= 1.0/ sqrt(x_final* x_final+ y_final* y_final+ z_final* z_final);

	for (i=0; i<3; i++)
		init_values.color[i]= rand_double(0., 1.);
	init_values.position_world[0]= x_init;
	init_values.position_world[1]= y_init;
	init_values.position_world[2]= z_init;
	init_values.position_world[3]= 1.;
	init_values.spot_cone_direction_world[0]= -n_inv_init* x_init+ rand_double(-decal, decal);
	init_values.spot_cone_direction_world[1]= -n_inv_init* y_init+ rand_double(-decal, decal); 
	init_values.spot_cone_direction_world[2]= -n_inv_init* z_init+ rand_double(-decal, decal);

	for (i=0; i<3; i++)
		final_values.color[i]= rand_double(0., 1.);
	final_values.position_world[0]= x_final;
	final_values.position_world[1]= y_final;
	final_values.position_world[2]= z_final;
	final_values.position_world[3]= 1.;
	final_values.spot_cone_direction_world[0]= -n_inv_final* x_final+ rand_double(-decal, decal);
	final_values.spot_cone_direction_world[1]= -n_inv_final* y_final+ rand_double(-decal, decal); 
	final_values.spot_cone_direction_world[2]= -n_inv_final* z_final+ rand_double(-decal, decal);
}


void LightConfig::defaultize() {
	unsigned int i;

	lifetime= 1000;
	
	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, 0.0));
	morph_pos.push_back(Pt(1.0, 1.0));

	for (i=0; i<3; i++)
		init_values.color[i]= 1.0;
	init_values.position_world[0]= 0.0;
	init_values.position_world[1]= 0.0;
	init_values.position_world[2]= 5.0;
	init_values.position_world[3]= 1.0;
	init_values.spot_cone_direction_world[0]= 0.0;
	init_values.spot_cone_direction_world[1]= 0.0; 
	init_values.spot_cone_direction_world[2]= -1.0;

	for (i=0; i<3; i++)
		final_values.color[i]= 1.0;
	final_values.position_world[0]= 0.0;
	final_values.position_world[1]= 0.0;
	final_values.position_world[2]= 5.0;
	final_values.position_world[3]= 1.0;
	final_values.spot_cone_direction_world[0]= 0.0;
	final_values.spot_cone_direction_world[1]= 0.0; 
	final_values.spot_cone_direction_world[2]= -1.0;
}


// --------------------------------------------------------------------------------------------
VMatConfig::VMatConfig() {
	type= LOAD_VMAT;
	active= true;
	retrig= true;
	defaultize();
}


ostream & operator << (ostream & out_stream, VMatConfig vmat) {
	unsigned int i;
	
	out_stream << "VMAT" << DELIM_ACTION;

	out_stream << vmat.active << DELIM_ACTION << vmat.retrig << DELIM_ACTION << vmat.lifetime << DELIM_ACTION;
	out_stream << vmat.morph_pos.size() << DELIM_ACTION;
	for (i=0; i<vmat.morph_pos.size(); i++)
		out_stream << vmat.morph_pos[i].x << DELIM_ACTION << vmat.morph_pos[i].y << DELIM_ACTION;
	out_stream << vmat.alpha_pos.size() << DELIM_ACTION;
	for (i=0; i<vmat.alpha_pos.size(); i++)
		out_stream << vmat.alpha_pos[i].x << DELIM_ACTION << vmat.alpha_pos[i].y << DELIM_ACTION;

	for (i=0; i<16; i++)
		out_stream << vmat.init_values.mat[i] << DELIM_ACTION;
	for (i=0; i<16; i++)
		out_stream << vmat.final_values.mat[i] << DELIM_ACTION;

	return out_stream;
}


istream & operator >> (istream & in_stream, VMatConfig & vmat) {
	string type;
	char c;
	bool ok= 1;
	unsigned int i, j;
	float x, y;
	VMatConfig vmat_tmp;
	
	ok= getline(in_stream, type, DELIM_ACTION);
	if (type!= "VMAT")
		ok= 0;
	
	in_stream >> vmat_tmp.active >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> vmat_tmp.retrig >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> vmat_tmp.lifetime >> c;
	if (c!= DELIM_ACTION) ok= 0;
	in_stream >> j >> c; // j == morph_pos.size()
	if (c!= DELIM_ACTION) ok= 0;
	vmat_tmp.morph_pos.clear();
	for (i=0; i<j; i++) {
		in_stream >> x >> c;
		if (c!= DELIM_ACTION) ok= 0;
		in_stream >> y >> c;
		if (c!= DELIM_ACTION) ok= 0;
		vmat_tmp.morph_pos.push_back(Pt(x, y));
	}
	in_stream >> j >> c; // j == alpha_pos.size()
	if (c!= DELIM_ACTION) ok= 0;
	vmat_tmp.alpha_pos.clear();
	for (i=0; i<j; i++) {
		in_stream >> x >> c;
		if (c!= DELIM_ACTION) ok= 0;
		in_stream >> y >> c;
		if (c!= DELIM_ACTION) ok= 0;
		vmat_tmp.alpha_pos.push_back(Pt(x, y));
	}
	
	for (i=0; i<16; i++) {
		in_stream >> vmat_tmp.init_values.mat[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	for (i=0; i<16; i++) {
		in_stream >> vmat_tmp.final_values.mat[i] >> c;
		if (c!= DELIM_ACTION) ok= 0;
	}
	
	if (ok) {
		vmat.active= vmat_tmp.active;
		vmat.retrig= vmat_tmp.retrig;
		vmat.lifetime= vmat_tmp.lifetime;
		vmat.morph_pos.clear();
		for (i=0; i<vmat_tmp.morph_pos.size(); i++)
			vmat.morph_pos.push_back(vmat_tmp.morph_pos[i]);
		vmat.alpha_pos.clear();
		for (i=0; i<vmat_tmp.alpha_pos.size(); i++)
			vmat.alpha_pos.push_back(vmat_tmp.alpha_pos[i]);

		for (i=0; i<16; i++) {
			vmat.init_values.mat[i]=  vmat_tmp.init_values.mat[i];
			vmat.final_values.mat[i]= vmat_tmp.final_values.mat[i];
		}
	}
	else {
		cout << "Erreur >> VMatConfig" << endl;
	}
	
	return in_stream;
}


void VMatConfig::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void VMatConfig::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void VMatConfig::randomize() {
	float m= 1.0;
	float d= 0.5;
	float trans= 0.5;
	unsigned int i, j;

	lifetime= rand_int(1000, 2000); // assez grand sinon ca bouge dans tous les sens
	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, rand_double(0.0, 1.0)));
	j= rand_int(2, 8);
	for (i=1; i<j; i++)
		morph_pos.push_back(Pt(float(i)/ float(j), rand_double(0.0, 1.0)));
	morph_pos.push_back(Pt(1.0, rand_double(0.0, 1.0)));

	alpha_pos.clear();
	//alpha_pos.push_back(Pt(0.0, rand_double(0.0, 1.0)));
	alpha_pos.push_back(Pt(0.0, 0.0)); // pour partir de la matrice Id meme quand random
	j= rand_int(2, 8);
	for (i=1; i<j; i++)
		alpha_pos.push_back(Pt(float(i)/ float(j), rand_double(0.0, 1.0)));
	//alpha_pos.push_back(Pt(1.0, rand_double(0.0, 1.0)));
	alpha_pos.push_back(Pt(1.0, 0.0)); // pour revenir à la matrice Id meme quand random
	
	for (i=0; i<16; i++)
		init_values.mat[i]= rand_double(-m, m);
	init_values.mat[3]= init_values.mat[7]= init_values.mat[11]= 0.0;
	init_values.mat[12]= rand_double(-trans, trans);
	init_values.mat[13]= rand_double(-trans, trans);
	init_values.mat[14]= rand_double(-trans, trans);
	init_values.mat[15]= 1.0;
	gram_schmidt(init_values.mat);
	
	for (i=0; i<16; i++)
		//final_values.mat[i]= rand_double(-m, m);
		final_values.mat[i]= init_values.mat[i]+ rand_double(-d, d);
	final_values.mat[3]= final_values.mat[7]= final_values.mat[11]= 0.0;
	final_values.mat[15]= 1.0;
	gram_schmidt(final_values.mat);
}


void VMatConfig::defaultize() {
	unsigned int i;

	lifetime= 1000;

	morph_pos.clear();
	morph_pos.push_back(Pt(0.0, 0.0));
	morph_pos.push_back(Pt(1.0, 1.0));

	alpha_pos.clear();
	alpha_pos.push_back(Pt(0.0, 0.0));
	alpha_pos.push_back(Pt(0.2, 1.0));
	alpha_pos.push_back(Pt(0.8, 1.0));
	alpha_pos.push_back(Pt(1.0, 0.0));

	for (i=0; i<16; i++)
		init_values.mat[i]= 0.0;
	for (i=0; i<4; i++)
		init_values.mat[i* 5]= 1.0;
	
	for (i=0; i<16; i++)
		final_values.mat[i]= 0.0;
	for (i=0; i<4; i++)
		final_values.mat[i* 5]= 1.0;
}


// --------------------------------------------------------------------------------------------
ChannelConfig::ChannelConfig() {
}


ostream & operator << (ostream & out_stream, ChannelConfig channel) {
	unsigned int i;
	
	out_stream << "CHANNEL" << DELIM_CHANNEL;
	out_stream << channel.actions.size() << DELIM_CHANNEL;
	for (i=0; i< channel.actions.size(); i++) {
		if (channel.actions[i]->type== LOAD_OBJ)
			out_stream << "MODEL_OBJ" << DELIM_CHANNEL << *(ModelObjConfig *)(channel.actions[i]) << DELIM_CHANNEL;
		else if (channel.actions[i]->type== LOAD_LIGHT)
			out_stream << "LIGHT" << DELIM_CHANNEL << *(LightConfig *)(channel.actions[i]) << DELIM_CHANNEL;
		if (channel.actions[i]->type== LOAD_VMAT)
			out_stream << "VMAT" << DELIM_CHANNEL << *(VMatConfig *)(channel.actions[i]) << DELIM_CHANNEL;
	}
	
	return out_stream;
}


istream & operator >> (istream & in_stream, ChannelConfig & channel) {
	string type, sub_type;
	char c;
	bool ok= 1;
	unsigned int i;
	ChannelConfig channel_tmp;
	unsigned int n_actions;
	
	ok= getline(in_stream, type, DELIM_CHANNEL);
	if (type!= "CHANNEL")
		ok= 0;
	in_stream >> n_actions >> c;
	if (c!= DELIM_CHANNEL) ok= 0;

	for (i=0; i<n_actions; i++) {
		ok= getline(in_stream, sub_type, DELIM_CHANNEL);
		if (sub_type== "MODEL_OBJ") {
			ModelObjConfig * moc= new ModelObjConfig();
			in_stream >> *moc >> c;
			if (c!= DELIM_CHANNEL) ok= 0; else channel_tmp.actions.push_back(moc);
		}
		else if (sub_type== "LIGHT") {
			LightConfig * lc= new LightConfig();
			in_stream >> *lc >> c;
			if (c!= DELIM_CHANNEL) ok= 0; else channel_tmp.actions.push_back(lc);
		}
		else if (sub_type== "VMAT") {
			VMatConfig * vmc= new VMatConfig();
			in_stream >> *vmc >> c;
			if (c!= DELIM_CHANNEL) ok= 0; else channel_tmp.actions.push_back(vmc);
		}
	}
	
	if (ok) {
		channel.actions= channel_tmp.actions;
	}
	else {
		cout << "Erreur >> ChannelConfig" << endl;
	}
	
	return in_stream;
}


void ChannelConfig::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void ChannelConfig::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void ChannelConfig::randomize() {
	release();
	unsigned int n_actions= rand_int(1, 4);
	unsigned int i;
	unsigned int idx_type;
	for (i=0; i< n_actions; i++) {
		idx_type= rand_int(0, 2);
		if (idx_type== 0) {
			ModelObjConfig * moc= new ModelObjConfig();
			moc->randomize();
			actions.push_back(moc);
		}
		else if (idx_type== 1) {
			LightConfig * lc= new LightConfig();
			lc->randomize();
			actions.push_back(lc);
		}
		else if (idx_type== 2) {
			VMatConfig * vmc= new VMatConfig();
			vmc->randomize();
			actions.push_back(vmc);
		}
	}
}


void ChannelConfig::release() {
	unsigned int i;
	for (i=0; i< actions.size(); i++) {
		delete actions[i];
	}
	actions.clear();
}


// --------------------------------------------------------------------------------------------
A2VConfig::A2VConfig() {
	unsigned int i;
	for (i=0; i< N_MAX_CHANNELS; i++) {
		channels_configs.push_back(ChannelConfig());
	}
}


ostream & operator << (ostream & out_stream, A2VConfig a2v) {
	unsigned int i;
	
	out_stream << "A2V" << DELIM_A2V;
	out_stream << a2v.channels_configs.size() << DELIM_A2V;
	for (i=0; i< a2v.channels_configs.size(); i++) {
		out_stream << a2v.channels_configs[i] << DELIM_A2V;
	}
	
	return out_stream;
}


istream & operator >> (istream & in_stream, A2VConfig & a2v) {
	string type;
	char c;
	bool ok= 1;
	unsigned int i;
	A2VConfig a2v_tmp;
	unsigned int n_channels;

	a2v_tmp.channels_configs.clear();
	
	ok= getline(in_stream, type, DELIM_A2V);
	if (type!= "A2V")
		ok= 0;
	in_stream >> n_channels >> c;
	if (c!= DELIM_A2V) ok= 0;

	for (i=0; i<n_channels; i++) {
		ChannelConfig cc;
		in_stream >> cc >> c;
		if (c!= DELIM_A2V) ok= 0; else a2v_tmp.channels_configs.push_back(cc);
	}
	
	if (ok) {
		a2v.channels_configs= a2v_tmp.channels_configs;
	}
	else {
		cout << "Erreur >> A2VConfig" << endl;
	}

	return in_stream;
}


void A2VConfig::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void A2VConfig::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void A2VConfig::randomize() {
	unsigned int i;
	for (i=0; i< N_MAX_CHANNELS; i++) {
		channels_configs[i].randomize();
	}
}


void A2VConfig::release() {
	unsigned int i;
	for (i=0; i< channels_configs.size(); i++) {
		channels_configs[i].release();
	}
	//channels_configs.clear();
}
