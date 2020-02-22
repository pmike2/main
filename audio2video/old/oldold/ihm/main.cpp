#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cmath>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tree_Item.H>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform.hpp>


//#include "a2v_interface_extend.h"
#include "a2v_interface.h"

#include "../pa_client/constantes.h"
#include "../pa_client/config.h"

using namespace std;


// ----------------------------------------------------------------------------------------------------
struct SockStruct {
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
};


const string CHANNEL_PREFIX= "Channel ";
const string ACTION_PREFIX= "Action ";
vector<string> TRANSFOS;

A2VInterface * a2vi;
SockStruct * sock;
A2VConfig a2vc;
int current_channel_idx= -1;
int current_action_idx= -1;
int integers[16]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


// ----------------------------------------------------------------------------------------------------
void init_server_socket(int portno, SockStruct * ss) {
	ss->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (ss->sockfd < 0) 
		printf("ERROR opening socket\n");
	
	int optval = 1;
    setsockopt(ss->sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	
	bzero((char *) &(ss->serv_addr), sizeof(ss->serv_addr));
	ss->serv_addr.sin_family = AF_INET;
	ss->serv_addr.sin_addr.s_addr = INADDR_ANY;
	ss->serv_addr.sin_port = htons(portno);
	if (bind(ss->sockfd, (struct sockaddr *) &(ss->serv_addr), sizeof(ss->serv_addr)) < 0) 
		printf("ERROR on binding\n");
	
	listen(ss->sockfd, SOMAXCONN);
	ss->clilen = sizeof(ss->cli_addr);
	ss->newsockfd = accept(ss->sockfd, (struct sockaddr *) &(ss->cli_addr), &(ss->clilen));
	if (ss->newsockfd < 0)
		printf("ERROR on accept\n");
	
	// pour rendre le IO non bloquant (a faire du cote serveur ET client)
	int flags = fcntl(ss->newsockfd, F_GETFL, 0);
	fcntl(ss->newsockfd, F_SETFL, flags | O_NONBLOCK);
}


void redraw() {
	unsigned int i, j;
	char s[16];
	Fl_Tree_Item * item;
	
	a2vi->tree_a2v->clear();
	for (i=0; i<a2vc.channels_configs.size(); i++) {
		string nom_channel= CHANNEL_PREFIX+ to_string(i);
		item= a2vi->tree_a2v->add(nom_channel.c_str());
		if (i== current_channel_idx)
			item->labelbgcolor(FL_RED);
	
		for (j=0; j<a2vc.channels_configs[i].actions.size(); j++) {
			string nom_action= nom_channel+ "/"+ ACTION_PREFIX+ to_string(j);
			item= a2vi->tree_a2v->add(nom_action.c_str());
			if ((i== current_channel_idx) && (j== current_action_idx))
				item->labelbgcolor(FL_RED);
		}
	}
	a2vi->tree_a2v->root_label("A2V");
	a2vi->tree_a2v->redraw();
	
	a2vi->group_model_obj->hide();
	a2vi->group_light->hide();
	a2vi->group_vmat->hide();
	
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;

	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	a2vi->active->value(ac->active);
	a2vi->retrig->value(ac->retrig);
	a2vi->lifetime->value(ac->lifetime);

	a2vi->morph_graph->pos.clear();
	for (i=0; i<ac->morph_pos.size(); i++) {
		float x= ac->morph_pos[i].x;
		float y= ac->morph_pos[i].y;
		int xx= int(x* float(a2vi->morph_graph->w())+ float(a2vi->morph_graph->x()));
		int yy= int((1.0- y)* float(a2vi->morph_graph->h())+ float(a2vi->morph_graph->y()));
		a2vi->morph_graph->pos.push_back(PtIhm(xx, yy));
	}
	a2vi->morph_graph->redraw();
		
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		//cout << *moc << endl;
		
		a2vi->model_obj_alpha_graph->pos.clear();
		for (i=0; i<moc->alpha_pos.size(); i++) {
			float x= moc->alpha_pos[i].x;
			float y= moc->alpha_pos[i].y;
			int xx= int(x* float(a2vi->model_obj_alpha_graph->w())+ float(a2vi->model_obj_alpha_graph->x()));
			int yy= int((1.0- y)* float(a2vi->model_obj_alpha_graph->h())+ float(a2vi->model_obj_alpha_graph->y()));
			a2vi->model_obj_alpha_graph->pos.push_back(PtIhm(xx, yy));
		}
		a2vi->model_obj_alpha_graph->redraw();

		a2vi->model_obj_ch_obj->value(moc->ch_obj.c_str());
		a2vi->model_obj_ch_mat->value(moc->ch_mat.c_str());
		
		for (i=0; i<16; i++) {
			sprintf(s, "%f", moc->init_values.model2world[i]);
			a2vi->model_obj_init_values_model2world->w[i]->value(s);
		}		
		for (i=0; i<3; i++) {	
			a2vi->model_obj_init_values_ambient->w[i]->value(moc->init_values.ambient[i]);
			a2vi->model_obj_init_values_diffuse->w[i]->value(moc->init_values.diffuse[i]);
		}
		a2vi->model_obj_init_values_shininess->value(moc->init_values.shininess);

		for (i=0; i<16; i++) {
			sprintf(s, "%f", moc->final_values.model2world[i]);
			a2vi->model_obj_final_values_model2world->w[i]->value(s);
		}		
		for (i=0; i<3; i++) {	
			a2vi->model_obj_final_values_ambient->w[i]->value(moc->final_values.ambient[i]);
			a2vi->model_obj_final_values_diffuse->w[i]->value(moc->final_values.diffuse[i]);
		}
		a2vi->model_obj_final_values_shininess->value(moc->final_values.shininess);
				
		a2vi->group_model_obj->show();
	}
	else if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		//cout << *lc << endl;
		
		for (i=0; i<3; i++) {	
			a2vi->light_init_values_color->w[i]->value(lc->init_values.color[i]);
			a2vi->light_init_values_spot_cone_direction_world->w[i]->value(lc->init_values.spot_cone_direction_world[i]);
		}
		for (i=0; i<4; i++) {	
			sprintf(s, "%f", lc->init_values.position_world[i]);
			a2vi->light_init_values_position_world->w[i]->value(s);
		}

		for (i=0; i<3; i++) {	
			a2vi->light_final_values_color->w[i]->value(lc->final_values.color[i]);
			a2vi->light_final_values_spot_cone_direction_world->w[i]->value(lc->final_values.spot_cone_direction_world[i]);
		}
		for (i=0; i<4; i++) {	
			sprintf(s, "%f", lc->final_values.position_world[i]);
			a2vi->light_final_values_position_world->w[i]->value(s);
		}
		
		a2vi->group_light->show();
	}
	else if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		//cout << *vmc << endl;

		a2vi->vmat_alpha_graph->pos.clear();
		for (i=0; i<vmc->alpha_pos.size(); i++) {
			float x= vmc->alpha_pos[i].x;
			float y= vmc->alpha_pos[i].y;
			int xx= int(x* float(a2vi->vmat_alpha_graph->w())+ float(a2vi->vmat_alpha_graph->x()));
			int yy= int((1.0- y)* float(a2vi->vmat_alpha_graph->h())+ float(a2vi->vmat_alpha_graph->y()));
			a2vi->vmat_alpha_graph->pos.push_back(PtIhm(xx, yy));
		}
		a2vi->vmat_alpha_graph->redraw();

		for (i=0; i<16; i++) {
			sprintf(s, "%f", vmc->init_values.mat[i]);
			a2vi->vmat_init_values_mat->w[i]->value(s);
		}

		for (i=0; i<16; i++) {
			sprintf(s, "%f", vmc->final_values.mat[i]);
			a2vi->vmat_final_values_mat->w[i]->value(s);
		}
		
		a2vi->group_vmat->show();
	}

}


void sync() {
	//cout << "sync" << endl;
	stringstream ss;
	ss << a2vc;
	string s= ss.str();
	
	int n = write(sock->newsockfd, s.c_str(), strlen(s.c_str()));
	if (n < 0)
		cout << "ERROR writing to socket" << endl;
}


// ----------------------------------------------------------------------------------------------------
void load_a2v_clicked(Fl_Widget *) {
	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::SINGLE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		current_channel_idx= -1;
		current_action_idx = -1;
		a2vc.load(chooser.value());
		redraw();
		sync();
	}
}


void save_a2v_clicked(Fl_Widget *) {
	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::CREATE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		a2vc.save(chooser.value());
	}
}


void randomize_a2v_clicked(Fl_Widget *) {
	current_channel_idx= -1;
	current_action_idx = -1;
	a2vc.randomize();
	redraw();
	sync();
}


void defaultize_a2v_clicked(Fl_Widget *) {
	current_channel_idx= -1;
	current_action_idx = -1;
	a2vc.release();
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
void load_channel_clicked(Fl_Widget *) {
	if (current_channel_idx== -1)
		return;

	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::SINGLE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		current_action_idx = -1;
		a2vc.channels_configs[current_channel_idx].load(chooser.value());
		redraw();
		sync();
	}
}


void save_channel_clicked(Fl_Widget *) {
	if (current_channel_idx== -1)
		return;

	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::CREATE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		a2vc.channels_configs[current_channel_idx].save(chooser.value());
	}
}


void randomize_channel_clicked(Fl_Widget *) {
	if (current_channel_idx== -1)
		return;
	
	current_action_idx = -1;
	a2vc.channels_configs[current_channel_idx].randomize();
	redraw();
	sync();
}


void defaultize_channel_clicked(Fl_Widget *) {
	if (current_channel_idx== -1)
		return;

	current_action_idx = -1;
	a2vc.channels_configs[current_channel_idx].release();
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
// ATTENTION pour l'instant ne fonctionne que si l'action loadée est du meme type que l'ancienne action !
void load_action_clicked(Fl_Widget *) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;

	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::SINGLE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		a2vc.channels_configs[current_channel_idx].actions[current_action_idx]->load(chooser.value());
		redraw();
		sync();
	}
}


void save_action_clicked(Fl_Widget *) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;

	Fl_File_Chooser chooser(".", "*.*", Fl_File_Chooser::CREATE, "Choisir fichier de config");
	chooser.show();

	while (chooser.shown())
		Fl::wait();

	if (chooser.value() != NULL) {
		a2vc.channels_configs[current_channel_idx].actions[current_action_idx]->save(chooser.value());
	}
}


void randomize_action_clicked(Fl_Widget *) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;

	a2vc.channels_configs[current_channel_idx].actions[current_action_idx]->randomize();
	redraw();
	sync();
}


void defaultize_action_clicked(Fl_Widget *) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;

	a2vc.channels_configs[current_channel_idx].actions[current_action_idx]->defaultize();
	redraw();
	sync();
}


void add_action_clicked(Fl_Widget *) {
	if (current_channel_idx== -1)
		return;
	
	string choice= string(a2vi->add_action_choice->value());
	if (choice== "OBJ")
		a2vc.channels_configs[current_channel_idx].actions.push_back(new ModelObjConfig());
	else if (choice== "LIGHT")
		a2vc.channels_configs[current_channel_idx].actions.push_back(new LightConfig());
	else if (choice== "VMAT")
		a2vc.channels_configs[current_channel_idx].actions.push_back(new VMatConfig());
	
	current_action_idx= a2vc.channels_configs[current_channel_idx].actions.size()- 1;
	
	redraw();
	sync();
}


void delete_action_clicked(Fl_Widget *) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;
	
	delete a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	a2vc.channels_configs[current_channel_idx].actions.erase(a2vc.channels_configs[current_channel_idx].actions.begin()+ current_action_idx);
	
	current_action_idx= -1;
	
	redraw();
	sync();
}


void enable_all_clicked(Fl_Widget *) {
	string choice= string(a2vi->enable_all_choice->value());
	for (unsigned int channel_idx=0; channel_idx<a2vc.channels_configs.size(); channel_idx++)
		for (unsigned int action_idx=0; action_idx<a2vc.channels_configs[channel_idx].actions.size(); action_idx++) {
			ActionConfig * ac= a2vc.channels_configs[channel_idx].actions[action_idx];
			if ( ((ac->type== LOAD_OBJ) && (choice== "OBJ")) || ((ac->type== LOAD_LIGHT) && (choice== "LIGHT")) || ((ac->type== LOAD_VMAT) && (choice== "VMAT")) )
				ac->active= true;
		}
	redraw();
	sync();
}


void disable_all_clicked(Fl_Widget *) {
	string choice= string(a2vi->enable_all_choice->value()); // le meme qu'enable ...
	for (unsigned int channel_idx=0; channel_idx<a2vc.channels_configs.size(); channel_idx++)
		for (unsigned int action_idx=0; action_idx<a2vc.channels_configs[channel_idx].actions.size(); action_idx++) {
			ActionConfig * ac= a2vc.channels_configs[channel_idx].actions[action_idx];
			if ( ((ac->type== LOAD_OBJ) && (choice== "OBJ")) || ((ac->type== LOAD_LIGHT) && (choice== "LIGHT")) || ((ac->type== LOAD_VMAT) && (choice== "VMAT")) )
				ac->active= false;
		}
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
void tree_a2v_select(Fl_Widget *w, void *data) {
	Fl_Tree *tree = (Fl_Tree*)w;
	Fl_Tree_Item *item = (Fl_Tree_Item*)tree->item_clicked();
	
	current_channel_idx= -1;
	current_action_idx= -1;

	string item_name= item->label();
	if (!item_name.compare(0, CHANNEL_PREFIX.size(), CHANNEL_PREFIX))
		current_channel_idx= atoi(item_name.substr(CHANNEL_PREFIX.size()).c_str());
	else if (!item_name.compare(0, ACTION_PREFIX.size(), ACTION_PREFIX)) {
		current_action_idx= atoi(item_name.substr(ACTION_PREFIX.size()).c_str());
		Fl_Tree_Item *parent= item->parent();
		string parent_name= parent->label();
		if (!parent_name.compare(0, CHANNEL_PREFIX.size(), CHANNEL_PREFIX))
			current_channel_idx= atoi(parent_name.substr(CHANNEL_PREFIX.size()).c_str());
	}
	
	redraw();
	//cout << "channel=" << current_channel_idx << " ; action=" << current_action_idx << endl;
}


// ----------------------------------------------------------------------------------------------------
void lifetime_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;	

	Fl_Value_Slider *sl= (Fl_Value_Slider *)w;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	ac->lifetime= sl->value();
	sync();
}


void active_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;	

	Fl_Check_Button *but= (Fl_Check_Button *)w;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	ac->active= but->value();
	sync();
}


void retrig_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;	

	Fl_Check_Button *but= (Fl_Check_Button *)w;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	ac->retrig= but->value();
	sync();
}


void copy_init2final_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;	
	
	unsigned int i;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		for (i=0; i<16; i++)
			moc->final_values.model2world[i]= moc->init_values.model2world[i];
		for (i=0; i<3; i++) {
			moc->final_values.ambient[i]= moc->init_values.ambient[i];
			moc->final_values.diffuse[i]= moc->init_values.diffuse[i];
		}
		moc->final_values.shininess= moc->init_values.shininess;
	}
	else if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		for (i=0; i<3; i++) {
			lc->final_values.color[i]= lc->init_values.color[i];
			lc->final_values.spot_cone_direction_world[i]= lc->init_values.spot_cone_direction_world[i];
		}
		for (i=0; i<4; i++)
			lc->final_values.position_world[i]= lc->init_values.position_world[i];
	}
	else if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		for (i=0; i<16; i++)
			vmc->final_values.mat[i]= vmc->init_values.mat[i];
	}
	
	redraw();
	sync();
}


void morph_graph_ok_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;
	
	unsigned int i;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	ac->morph_pos.clear();
	for (i=0; i<a2vi->morph_graph->pos.size(); i++) {
		int x= a2vi->morph_graph->pos[i].x;
		int y= a2vi->morph_graph->pos[i].y;
		float xx= float(x- a2vi->morph_graph->x())/ float(a2vi->morph_graph->w());
		float yy= 1.0- float(y- a2vi->morph_graph->y())/ float(a2vi->morph_graph->h());
		ac->morph_pos.push_back(Pt(xx, yy));
	}
	sync();
}


void model_obj_alpha_graph_ok_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;
	
	unsigned int i;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->alpha_pos.clear();
		for (i=0; i<a2vi->model_obj_alpha_graph->pos.size(); i++) {
			int x= a2vi->model_obj_alpha_graph->pos[i].x;
			int y= a2vi->model_obj_alpha_graph->pos[i].y;
			float xx= float(x- a2vi->model_obj_alpha_graph->x())/ float(a2vi->model_obj_alpha_graph->w());
			float yy= 1.0- float(y- a2vi->model_obj_alpha_graph->y())/ float(a2vi->model_obj_alpha_graph->h());
			moc->alpha_pos.push_back(Pt(xx, yy));
		}
	}
	sync();
}


// ----------------------------------------------------------------------------------------------------
void model_obj_ch_obj_clicked(Fl_Widget *w, void *data) {
	Fl_Input_Choice *choice= (Fl_Input_Choice *)w;
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->ch_obj= ROOT_MODELES+ "/"+ string(choice->value());
		choice->value(moc->ch_obj.c_str());
		sync();
	}
}


void model_obj_ch_mat_clicked(Fl_Widget *w, void *data) {
	Fl_Input_Choice *choice= (Fl_Input_Choice *)w;
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->ch_mat= ROOT_MATS+ "/"+ string(choice->value());
		choice->value(moc->ch_mat.c_str());
		sync();
	}
}


// ----------------------------------------------------------------------------------------------------
void model_obj_init_values_model2world_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->init_values.model2world[*idx]= atof(in->value());
		sync();
	}
}


void model_obj_init_values_ambient_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->init_values.ambient[*idx]= in->value();
		sync();
	}
}


void model_obj_init_values_diffuse_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->init_values.diffuse[*idx]= in->value();
		sync();
	}
}


void model_obj_init_values_shininess_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->init_values.shininess= in->value();
		sync();
	}
}


void model_obj_init_values_transfo_ok_clicked(Fl_Widget *w, void *data) {
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];

	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		string choice= string(a2vi->model_obj_init_values_transfo_choice->value());
		float value= atof(a2vi->model_obj_init_values_transfo_value->value());
		glm::mat4 glm_model2world= glm::make_mat4(moc->init_values.model2world);
		if (choice== "translateX")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(value, 0.f, 0.f));
		else if (choice== "translateY")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(0.f, value, 0.f));
		else if (choice== "translateZ")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(0.f, 0.f, value));
		else if (choice== "rotateX")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(1.f, 0.f, 0.f));
		else if (choice== "rotateY")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 1.f, 0.f));
		else if (choice== "rotateZ")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 0.f, 1.f));
		else if (choice== "scaleX")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(value, 1.f, 1.f));
		else if (choice== "scaleY")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(1.f, value, 1.f));
		else if (choice== "scaleZ")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(1.f, 1.f, value));
		
		memcpy(moc->init_values.model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	}
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
void model_obj_final_values_model2world_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->final_values.model2world[*idx]= atof(in->value());
		sync();
	}
}


void model_obj_final_values_ambient_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->final_values.ambient[*idx]= in->value();
		sync();
	}
}


void model_obj_final_values_diffuse_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->final_values.diffuse[*idx]= in->value();
		sync();
	}
}


void model_obj_final_values_shininess_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		moc->final_values.shininess= in->value();
		sync();
	}
}


void model_obj_final_values_transfo_ok_clicked(Fl_Widget *w, void *data) {
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];

	if (ac->type== LOAD_OBJ) {
		ModelObjConfig * moc= (ModelObjConfig *)ac;
		string choice= string(a2vi->model_obj_final_values_transfo_choice->value());
		float value= atof(a2vi->model_obj_final_values_transfo_value->value());
		glm::mat4 glm_model2world= glm::make_mat4(moc->final_values.model2world);
		if (choice== "translateX")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(value, 0.f, 0.f));
		else if (choice== "translateY")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(0.f, value, 0.f));
		else if (choice== "translateZ")
			glm_model2world= glm::translate(glm_model2world, glm::vec3(0.f, 0.f, value));
		else if (choice== "rotateX")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(1.f, 0.f, 0.f));
		else if (choice== "rotateY")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 1.f, 0.f));
		else if (choice== "rotateZ")
			glm_model2world= glm::rotate(glm_model2world, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 0.f, 1.f));
		else if (choice== "scaleX")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(value, 1.f, 1.f));
		else if (choice== "scaleY")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(1.f, value, 1.f));
		else if (choice== "scaleZ")
			glm_model2world= glm::scale(glm_model2world, glm::vec3(1.f, 1.f, value));
		
		memcpy(moc->final_values.model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	}
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
void light_init_values_color_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->init_values.color[*idx]= in->value();
		sync();
	}
}


void light_init_values_spot_cone_direction_world_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->init_values.spot_cone_direction_world[*idx]= in->value();
		sync();
	}
}


void light_init_values_position_world_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->init_values.position_world[*idx]= atof(in->value());
		sync();
	}
}


// ----------------------------------------------------------------------------------------------------
void light_final_values_color_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->final_values.color[*idx]= in->value();
		sync();
	}
}


void light_final_values_spot_cone_direction_world_clicked(Fl_Widget *w, void *data) {
	Fl_Value_Slider *in = (Fl_Value_Slider*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->final_values.spot_cone_direction_world[*idx]= in->value();
		sync();
	}
}


void light_final_values_position_world_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_LIGHT) {
		LightConfig * lc= (LightConfig *)ac;
		lc->final_values.position_world[*idx]= atof(in->value());
		sync();
	}
}


// ----------------------------------------------------------------------------------------------------
void vmat_alpha_graph_ok_clicked(Fl_Widget *w, void *data) {
	if ((current_channel_idx== -1) || (current_action_idx== -1))
		return;
	
	unsigned int i;
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		vmc->alpha_pos.clear();
		for (i=0; i<a2vi->vmat_alpha_graph->pos.size(); i++) {
			int x= a2vi->vmat_alpha_graph->pos[i].x;
			int y= a2vi->vmat_alpha_graph->pos[i].y;
			float xx= float(x- a2vi->vmat_alpha_graph->x())/ float(a2vi->vmat_alpha_graph->w());
			float yy= 1.0- float(y- a2vi->vmat_alpha_graph->y())/ float(a2vi->vmat_alpha_graph->h());
			vmc->alpha_pos.push_back(Pt(xx, yy));
		}
		sync();
	}
}


// ----------------------------------------------------------------------------------------------------
void vmat_init_values_mat_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		vmc->init_values.mat[*idx]= atof(in->value());
		sync();
	}
}


void vmat_init_values_transfo_ok_clicked(Fl_Widget *w, void *data) {
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];

	if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		string choice= string(a2vi->vmat_init_values_transfo_choice->value());
		float value= atof(a2vi->vmat_init_values_transfo_value->value());
		glm::mat4 glm_mat= glm::make_mat4(vmc->init_values.mat);
		if (choice== "translateX")
			glm_mat= glm::translate(glm_mat, glm::vec3(value, 0.f, 0.f));
		else if (choice== "translateY")
			glm_mat= glm::translate(glm_mat, glm::vec3(0.f, value, 0.f));
		else if (choice== "translateZ")
			glm_mat= glm::translate(glm_mat, glm::vec3(0.f, 0.f, value));
		else if (choice== "rotateX")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(1.f, 0.f, 0.f));
		else if (choice== "rotateY")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 1.f, 0.f));
		else if (choice== "rotateZ")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 0.f, 1.f));
		else if (choice== "scaleX")
			glm_mat= glm::scale(glm_mat, glm::vec3(value, 1.f, 1.f));
		else if (choice== "scaleY")
			glm_mat= glm::scale(glm_mat, glm::vec3(1.f, value, 1.f));
		else if (choice== "scaleZ")
			glm_mat= glm::scale(glm_mat, glm::vec3(1.f, 1.f, value));
		
		memcpy(vmc->init_values.mat, glm::value_ptr(glm_mat), sizeof(float) * 16);
	}
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
void vmat_final_values_mat_clicked(Fl_Widget *w, void *data) {
	Fl_Float_Input *in = (Fl_Float_Input*)w;
	int *idx= (int *)(data);
	
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];
	if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		vmc->final_values.mat[*idx]= atof(in->value());
		sync();
	}
}


void vmat_final_values_transfo_ok_clicked(Fl_Widget *w, void *data) {
	ActionConfig * ac= a2vc.channels_configs[current_channel_idx].actions[current_action_idx];

	if (ac->type== LOAD_VMAT) {
		VMatConfig * vmc= (VMatConfig *)ac;
		string choice= string(a2vi->vmat_final_values_transfo_choice->value());
		float value= atof(a2vi->vmat_final_values_transfo_value->value());
		glm::mat4 glm_mat= glm::make_mat4(vmc->final_values.mat);
		if (choice== "translateX")
			glm_mat= glm::translate(glm_mat, glm::vec3(value, 0.f, 0.f));
		else if (choice== "translateY")
			glm_mat= glm::translate(glm_mat, glm::vec3(0.f, value, 0.f));
		else if (choice== "translateZ")
			glm_mat= glm::translate(glm_mat, glm::vec3(0.f, 0.f, value));
		else if (choice== "rotateX")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(1.f, 0.f, 0.f));
		else if (choice== "rotateY")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 1.f, 0.f));
		else if (choice== "rotateZ")
			glm_mat= glm::rotate(glm_mat, value* float(M_PI)/ 180.0f, glm::vec3(0.f, 0.f, 1.f));
		else if (choice== "scaleX")
			glm_mat= glm::scale(glm_mat, glm::vec3(value, 1.f, 1.f));
		else if (choice== "scaleY")
			glm_mat= glm::scale(glm_mat, glm::vec3(1.f, value, 1.f));
		else if (choice== "scaleZ")
			glm_mat= glm::scale(glm_mat, glm::vec3(1.f, 1.f, value));
		
		memcpy(vmc->final_values.mat, glm::value_ptr(glm_mat), sizeof(float) * 16);
	}
	redraw();
	sync();
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
	unsigned int i;
	
	srand(time(NULL));
	
	sock= new SockStruct();
	//init_server_socket(PORT_NUMBER_FLTK, sock);
	
	a2vi = new A2VInterface();

	a2vi->add_action_choice->add("OBJ");
	a2vi->add_action_choice->add("LIGHT");
	a2vi->add_action_choice->add("VMAT");
	a2vi->add_action_choice->value(0);

	a2vi->enable_all_choice->add("OBJ");
	a2vi->enable_all_choice->add("LIGHT");
	a2vi->enable_all_choice->add("VMAT");
	a2vi->enable_all_choice->value(0);
	
	TRANSFOS.push_back("translateX");
	TRANSFOS.push_back("translateY");
	TRANSFOS.push_back("translateZ");
	TRANSFOS.push_back("rotateX");
	TRANSFOS.push_back("rotateY");
	TRANSFOS.push_back("rotateZ");
	TRANSFOS.push_back("scaleX");
	TRANSFOS.push_back("scaleY");
	TRANSFOS.push_back("scaleZ");
	
	for (i=0; i<TRANSFOS.size(); i++)
		a2vi->model_obj_init_values_transfo_choice->add(TRANSFOS[i].c_str());
	a2vi->model_obj_init_values_transfo_choice->value(0);

	for (i=0; i<TRANSFOS.size(); i++)
		a2vi->model_obj_final_values_transfo_choice->add(TRANSFOS[i].c_str());
	a2vi->model_obj_final_values_transfo_choice->value(0);

	for (i=0; i<TRANSFOS.size(); i++)
		a2vi->vmat_init_values_transfo_choice->add(TRANSFOS[i].c_str());
	a2vi->vmat_init_values_transfo_choice->value(0);

	for (i=0; i<TRANSFOS.size(); i++)
		a2vi->vmat_final_values_transfo_choice->add(TRANSFOS[i].c_str());
	a2vi->vmat_final_values_transfo_choice->value(0);
	
	vector<string> modeles= list_files(ROOT_MODELES, "obj");
	for (i=0; i<modeles.size(); i++)
		a2vi->model_obj_ch_obj->add(modeles[i].c_str());
	if (modeles.size()> 0)
		a2vi->model_obj_ch_obj->value(0);
	
	vector<string> mats= list_files(ROOT_MATS, "mtl");
	for (i=0; i<mats.size(); i++)
		a2vi->model_obj_ch_mat->add(mats[i].c_str());
	if (mats.size()> 0)
		a2vi->model_obj_ch_mat->value(0);
	
	a2vi->load_a2v->callback(load_a2v_clicked);
	a2vi->save_a2v->callback(save_a2v_clicked);
	a2vi->randomize_a2v->callback(randomize_a2v_clicked);
	a2vi->defaultize_a2v->callback(defaultize_a2v_clicked);

	a2vi->load_channel->callback(load_channel_clicked);
	a2vi->save_channel->callback(save_channel_clicked);
	a2vi->randomize_channel->callback(randomize_channel_clicked);
	a2vi->defaultize_channel->callback(defaultize_channel_clicked);

	a2vi->load_action->callback(load_action_clicked);
	a2vi->save_action->callback(save_action_clicked);
	a2vi->randomize_action->callback(randomize_action_clicked);
	a2vi->defaultize_action->callback(defaultize_action_clicked);
	a2vi->add_action->callback(add_action_clicked);
	a2vi->delete_action->callback(delete_action_clicked);
	a2vi->enable_all->callback(enable_all_clicked);
	a2vi->disable_all->callback(disable_all_clicked);

	a2vi->tree_a2v->callback(tree_a2v_select, NULL);
	
	a2vi->lifetime->callback(lifetime_clicked);
	a2vi->active->callback(active_clicked);
	a2vi->retrig->callback(retrig_clicked);
	a2vi->copy_init2final->callback(copy_init2final_clicked);
	//a2vi->morph_graph->callback(morph_graph_clicked); // ne fonctionne pas ! -> obligé de faire un bouton morph_graph_ok
	a2vi->morph_graph_ok->callback(morph_graph_ok_clicked);
	a2vi->model_obj_alpha_graph_ok->callback(model_obj_alpha_graph_ok_clicked);
	
	// ----------------------------------------------------------------------
	a2vi->model_obj_ch_obj->callback(model_obj_ch_obj_clicked);
	a2vi->model_obj_ch_mat->callback(model_obj_ch_mat_clicked);
	
	for (i=0; i<16; i++)
		a2vi->model_obj_init_values_model2world->w[i]->callback(model_obj_init_values_model2world_clicked, &integers[i]);
	for (i=0; i<3; i++) {
		a2vi->model_obj_init_values_ambient->w[i]->callback(model_obj_init_values_ambient_clicked, &integers[i]);
		a2vi->model_obj_init_values_diffuse->w[i]->callback(model_obj_init_values_diffuse_clicked, &integers[i]);
	}
	a2vi->model_obj_init_values_shininess->callback(model_obj_init_values_shininess_clicked);
	a2vi->model_obj_init_values_transfo_ok->callback(model_obj_init_values_transfo_ok_clicked);

	for (i=0; i<16; i++)
		a2vi->model_obj_final_values_model2world->w[i]->callback(model_obj_final_values_model2world_clicked, &integers[i]);
	for (i=0; i<3; i++) {
		a2vi->model_obj_final_values_ambient->w[i]->callback(model_obj_final_values_ambient_clicked, &integers[i]);
		a2vi->model_obj_final_values_diffuse->w[i]->callback(model_obj_final_values_diffuse_clicked, &integers[i]);
	}
	a2vi->model_obj_final_values_shininess->callback(model_obj_final_values_shininess_clicked);
	a2vi->model_obj_final_values_transfo_ok->callback(model_obj_final_values_transfo_ok_clicked);
	
	// ----------------------------------------------------------------------
	for (i=0; i<3; i++) {
		a2vi->light_init_values_color->w[i]->callback(light_init_values_color_clicked, &integers[i]);
		a2vi->light_init_values_spot_cone_direction_world->w[i]->callback(light_init_values_spot_cone_direction_world_clicked, &integers[i]);
	}
	for (i=0; i<4; i++)
		a2vi->light_init_values_position_world->w[i]->callback(light_init_values_position_world_clicked, &integers[i]);

	for (i=0; i<3; i++) {
		a2vi->light_final_values_color->w[i]->callback(light_final_values_color_clicked, &integers[i]);
		a2vi->light_final_values_spot_cone_direction_world->w[i]->callback(light_final_values_spot_cone_direction_world_clicked, &integers[i]);
	}
	for (i=0; i<4; i++)
		a2vi->light_final_values_position_world->w[i]->callback(light_final_values_position_world_clicked, &integers[i]);

	// ----------------------------------------------------------------------
	a2vi->vmat_alpha_graph_ok->callback(vmat_alpha_graph_ok_clicked);

	for (i=0; i<16; i++)
		a2vi->vmat_init_values_mat->w[i]->callback(vmat_init_values_mat_clicked, &integers[i]);
	a2vi->vmat_init_values_transfo_ok->callback(vmat_init_values_transfo_ok_clicked);

	for (i=0; i<16; i++)
		a2vi->vmat_final_values_mat->w[i]->callback(vmat_final_values_mat_clicked, &integers[i]);
	a2vi->vmat_final_values_transfo_ok->callback(vmat_final_values_transfo_ok_clicked);

	// ----------------------------------------------------------------------
	a2vi->main_window->position(1200, 0);
	a2vi->main_window->show(argc, argv);

	redraw();
	sync();
	
	return Fl::run();
	
	return 0;
}

