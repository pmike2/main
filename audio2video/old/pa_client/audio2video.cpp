
#include "audio2video.h"

using namespace std;
using json = nlohmann::json;



CC::CC() {
	value= 0.;
	value_max= -1e7;
	value_min= 1e7;
}


void Audio2Video::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) {
	(void) remoteEndpoint; // suppress unused parameter warning
	try {

		osc::ReceivedMessageArgumentStream args= m.ArgumentStream();

		if (std::strcmp(m.AddressPattern(), "/amplitude" )== 0) {
			float channel_f, cc; // puredata envoie des floats, pas de ints
			args >> channel_f >> cc >> osc::EndMessage;
			int channel= (int) channel_f;
			//cout << "received amplitude "<< cc << " ; " << channel << endl;
			
			mtxs[channel- 1].lock();
			ccs[channel- 1].value= cc;
			mtxs[channel- 1].unlock();
			
		}

		else if (std::strcmp(m.AddressPattern(), "/sync" )== 0) {
			const char * ch_json;
			args >> ch_json >> osc::EndMessage;
			std::string config_load= string(ch_json);
			load(config_load);
		}

		else if (std::strcmp(m.AddressPattern(), "/param" )== 0) {
			int idx_channel, idx_action;
			const char * val;
			const char * key;
			args >> idx_channel >> idx_action >> key >> val >> osc::EndMessage;
			cout << idx_channel << " ; " << idx_action << " ; " << key << " ; " << val << endl;
			set_param(idx_channel, idx_action, string(key), string(val));
		}

		else if (std::strcmp(m.AddressPattern(), "/at_load" )== 0) {
			int idx_item, val;
			const char * key;
			args >> idx_item >> key >> val >> osc::EndMessage;
			cout << idx_item << " ; " << key << " ; " << val << endl;
		}
		
	} catch (osc::Exception& e) {
		std::cout << "error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n";
	}
}


void Audio2Video::osc_thread_method() {
	osc_socket->RunUntilSigInt();
}


Audio2Video::Audio2Video() {
}


Audio2Video::Audio2Video(GLuint prog_draw_, GLuint prog_repere_) {

	prog_draw= prog_draw_;
	prog_repere= prog_repere_;
	
	lights_ubo.set_prog(prog_draw);
	lights_ubo.init();
	
	is_debug= false;
	
	for (int i=0; i<N_MAX_CHANNELS; ++i) {
		ccs.push_back(CC());
	}
	
	osc_socket= new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT_NUMBER), this);
	osc_thread= new thread(&Audio2Video::osc_thread_method, this);
	
	//load("../configs/test.json");
}


void Audio2Video::draw(float * world2clip) {
	unsigned int i;
	for (i=0; i<model_objs.size(); i++)
		model_objs[i].draw();
	
	if (is_debug) {
		for (i=0; i<lights.size(); i++)
			lights[i].draw(world2clip);
	}
}


void Audio2Video::anim(float * world2camera, float * camera2clip) {
	
	for (unsigned int i=0; i< vmats.size(); i++)
		vmats[i].anim();

	glm::mat4 glm_world2camera_vmat= glm::make_mat4(world2camera);
	for (unsigned int i=0; i< vmats.size(); i++) {
		glm::mat4 glm_vmat= glm::make_mat4(vmats[i].mat);
		glm::mat4 id4= glm::mat4();
		glm_world2camera_vmat= glm_world2camera_vmat* ((1- vmats[i].alpha)* id4+ vmats[i].alpha* glm_vmat);
	}
	float world2camera_vmat[16];
	memcpy(world2camera_vmat, glm::value_ptr(glm_world2camera_vmat), sizeof(float) * 16);
	
	for (unsigned int i=0; i< model_objs.size(); i++)
		model_objs[i].anim(world2camera_vmat, camera2clip);

	for (unsigned int i=0; i< lights.size(); i++)
		lights[i].anim(world2camera);
	lights_ubo.update(lights);

}


void Audio2Video::idle() {
	unsigned int idx_channel;
	
	
	/*float lambda= 7.;
	float offset_x= 0.8; // valeurs normales 0.5 et 0.5
	float offset_y= 0.3;*/
	
	for (idx_channel=0; idx_channel<channels_configs.size(); ++idx_channel) {
		mtxs[idx_channel].lock();
		if (ccs[idx_channel].value_min> ccs[idx_channel].value)
			ccs[idx_channel].value_min= ccs[idx_channel].value;
		if (ccs[idx_channel].value_max< ccs[idx_channel].value)
			ccs[idx_channel].value_max= ccs[idx_channel].value;
		mtxs[idx_channel].unlock();
		
		float val= 0.;
		float valsigmoid= 0.;
		if (abs(ccs[idx_channel].value_max- ccs[idx_channel].value_min)> 1e-5) {
			val= (ccs[idx_channel].value- ccs[idx_channel].value_min)/ (ccs[idx_channel].value_max- ccs[idx_channel].value_min);
			//valsigmoid= offset_y+ (1.+ lambda* 0.5)* (val- offset_x)/ (1.+ lambda* abs(val- offset_x));
			valsigmoid= val;
		}
		
		for (int idx_action=0; idx_action<channels_configs[idx_channel]->actions.size(); ++idx_action) {
			ActionConfig * ac= channels_configs[idx_channel]->actions[idx_action];
			//cout << ac->active << " ; " << ac->type << " ; " << ac->target << endl;
			if (!ac->active)
				continue;
			
			if (ac->type== "OBJ") {
				
				if (ac->idx>= model_objs.size()) {
					cout << "erreur config idx obj" << endl;
					continue;
				}
				
				if (ac->target== "alpha") {
					model_objs[ac->idx].alpha= ac->mult_offset* valsigmoid+ ac->add_offset;
					if (model_objs[ac->idx].alpha> 1.) model_objs[ac->idx].alpha= 1.;
					if (model_objs[ac->idx].alpha< 0.) model_objs[ac->idx].alpha= 0.;
				}
				else if (ac->target== "diffuse") {
					model_objs[ac->idx].diffuse[ac->target_idx]= ac->mult_offset* valsigmoid+ ac->add_offset;
					if (model_objs[ac->idx].diffuse[ac->target_idx]> 1.) model_objs[ac->idx].diffuse[ac->target_idx]= 1.;
					if (model_objs[ac->idx].diffuse[ac->target_idx]< 0.) model_objs[ac->idx].diffuse[ac->target_idx]= 0.;
				}
			}

			else if (ac->type== "LIGHT") {
				
				if (ac->idx>= lights.size()) {
					cout << "erreur config idx light" << endl;
					continue;
				}
				
				if (ac->target== "color") {
					lights[ac->idx].color[ac->target_idx]= ac->mult_offset* valsigmoid+ ac->add_offset;
					if (lights[ac->idx].color[ac->target_idx]> 1.) lights[ac->idx].color[ac->target_idx]= 1.;
					if (lights[ac->idx].color[ac->target_idx]< 0.) lights[ac->idx].color[ac->target_idx]= 0.;
				}
			}

			else if (ac->type== "VMAT") {
				
				if (ac->idx>= vmats.size()) {
					cout << "erreur config idx vmat" << endl;
					continue;
				}
				
				if (ac->target== "alpha") {
					vmats[ac->idx].alpha= ac->mult_offset* valsigmoid+ ac->add_offset;
					if (vmats[ac->idx].alpha> 1.) vmats[ac->idx].alpha= 1.;
					if (vmats[ac->idx].alpha< 0.) vmats[ac->idx].alpha= 0.;
				}
			}
			
		}
			
	}

}


void Audio2Video::release() {
	unsigned int i;
	
	for (i=0; i<model_objs.size(); i++)
		model_objs[i].release();

	lights_ubo.release();
	
	for (i=0; i< channels_configs.size(); i++) {
		channels_configs[i]->release();
	}
	channels_configs.clear();

}


void Audio2Video::clear() {
	model_objs.clear();
	lights.clear();
	lights_ubo.update(lights);
	vmats.clear();
}


void Audio2Video::save(string ch) {
}


void Audio2Video::load(string ch) {
	cout << "load " << ch << endl;
	std::ifstream istr(ch);
	json js;
	istr >> js;
	
	json a2v= js["a2v"];
	
	for (json::iterator it= a2v.begin(); it!= a2v.end(); ++it) {
		//cout << it.key() << " : " << it.value() << endl;
		string str= it.key();
		if (str== "at_load") {
			json at_load= it.value();
			for (json::iterator it2= at_load.begin(); it2!= at_load.end(); ++it2) {
				string type= (*it2)["type"];
				if (type== "OBJ") {
					string obj= (*it2)["obj"];
					string mtl= (*it2)["mtl"];
					ModelObj model_obj(prog_draw);
					model_obj.load(obj, mtl);
					model_objs.push_back(model_obj);
				}
				else if (type== "LIGHT") {
					bool spotlight= (*it2)["spotlight"];
					Light light(spotlight, prog_draw);
					lights.push_back(light);
					lights_ubo.update(lights);
				}
				else if (type== "VMAT") {
					glm::mat4 glm_mat= glm::mat4();
					json transformations= (*it2)["transformations"];
					for (json::iterator it3= transformations.begin(); it3!= transformations.end(); ++it3) {
						string transfo_type= (*it3)["type"];
						float x= (*it3)["x"];
						float y= (*it3)["y"];
						float z= (*it3)["z"];
						if (transfo_type== "rotation") {
							glm_mat= glm::rotate(glm_mat, glm::radians(x), glm::vec3(1., 0., 0.));
							glm_mat= glm::rotate(glm_mat, glm::radians(y), glm::vec3(0., 1., 0.));
							glm_mat= glm::rotate(glm_mat, glm::radians(z), glm::vec3(0., 0., 1.));
						}
						else if (transfo_type== "translation") {
							glm_mat= glm::translate(glm_mat, glm::vec3(x, y, z));
						}
						else if (transfo_type== "scale") {
							glm_mat= glm::scale(glm_mat, glm::vec3(x, y, z));
						}
					}
					VMat vmat;
					memcpy(vmat.mat, glm::value_ptr(glm_mat), sizeof(float) * 16);
					vmats.push_back(vmat);
				}
			}
		}
		else if (str.substr(0, 7)== "channel") {
			//cout << it.value() << endl;
			vector<string> x= str_split(str, '_');
			unsigned int idx_channel= stoi(x[1]);
			
			ChannelConfig * channel_config= new ChannelConfig();
			
			json cc= it.value()["cc"];
			for (json::iterator it2= cc.begin(); it2!= cc.end(); ++it2) {
				ActionConfig * ac= new ActionConfig();
				ac->type     = (*it2)["type"];
				ac->idx= (*it2)["idx"];
				json action     = (*it2)["action"];
				
				for (json::iterator it3= action.begin(); it3!= action.end(); ++it3) {
					//cout << it3.key() << " : " << it3.value() << endl;
					if (it3.key()== "target") {
						ac->target= it3.value();
					}
					else if (it3.key()== "idx") {
						ac->target_idx= it3.value();
					}
					else if (it3.key()== "mult_offset") {
						ac->mult_offset= it3.value();
					}
					else if (it3.key()== "add_offset") {
						ac->add_offset= it3.value();
					}
					
				}
				channel_config->actions.push_back(ac);
			}
			channels_configs.push_back(channel_config);
		}
	}
	
	
}


void Audio2Video::set_param(unsigned int idx_channel, unsigned int idx_action, std::string key, std::string val) {
	
	if (idx_channel>= channels_configs.size()) {
		cout << "set_param error : idx_channel=" << idx_channel << ">=" << channels_configs.size() << endl;
		return;
	}

	if (idx_action>= channels_configs[idx_channel]->actions.size()) {
		cout << "set_param error : idx_action=" << idx_action << ">=" << channels_configs[idx_channel]->actions.size() << endl;
		return;
	}
	
	if (key== "target") {
		channels_configs[idx_channel]->actions[idx_action]->target= val;
	}
	else if (key== "active") {
		channels_configs[idx_channel]->actions[idx_action]->active= bool(stoi(val));
	}
	else if (key== "add_offset") {
		channels_configs[idx_channel]->actions[idx_action]->add_offset= stof(val);
	}
	else if (key== "mult_offset") {
		channels_configs[idx_channel]->actions[idx_action]->mult_offset= stof(val);
	}
	else if (key== "idx") {
		channels_configs[idx_channel]->actions[idx_action]->idx= stoi(val);
	}
	
}


void Audio2Video::randomize() {
	unsigned int i;
	for (i=0; i< channels_configs.size(); i++) {
		channels_configs[i]->randomize();
	}
}


void Audio2Video::print() {
	unsigned int i, j;
	
	cout << "A2V DEBUG =======================================================" << endl;
	
	for (i=0; i<channels_configs.size(); i++) {
		for (j=0; j<channels_configs[i]->actions.size(); j++)
			cout << "channel " << i << " action " << j << " : type=" << channels_configs[i]->actions[j]->type << endl;
	}
	
	for (i=0; i<model_objs.size(); i++) {
		model_objs[i].print(false);
	}

	for (i=0; i<lights.size(); i++) {
		lights[i].print();
	}
	lights_ubo.print();
	
	for (i=0; i<vmats.size(); i++) {
		vmats[i].print();
	}
	
	cout << "=================================================================" << endl;
}

