
#include "audio2video.h"

using namespace std;



void socket_channel_listen(unsigned int portno, SocketChannel * sc) {
	// possibilité de lancer client et serveur sur 2 postes differents
	sc->hostname= "localhost";
	sc->portno= portno;

	sc->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sc->sockfd < 0) {
		cout << "ERROR opening socket" << endl;
	}
	sc->server = gethostbyname(sc->hostname.c_str());
	if (sc->server == NULL) {
		cout << "ERROR, no such host" << endl;
	}

	bzero((char *) &(sc->serv_addr), sizeof(sc->serv_addr));
	sc->serv_addr.sin_family = AF_INET;
	bcopy((char *)(sc->server)->h_addr, (char *)&(sc->serv_addr).sin_addr.s_addr, (sc->server)->h_length);
	sc->serv_addr.sin_port = htons(sc->portno);
	if (connect(sc->sockfd, (struct sockaddr *) &(sc->serv_addr), sizeof(sc->serv_addr)) < 0) {
		cout << "ERROR connecting to port " << portno << endl;
	}

	// pour rendre le IO non bloquant (a faire du cote serveur ET client)
	int flags = fcntl(sc->sockfd, F_GETFL, 0);
	fcntl(sc->sockfd, F_SETFL, flags | O_NONBLOCK);
}


Audio2Video::Audio2Video() {
}


Audio2Video::Audio2Video(GLuint prog_draw_, GLuint prog_repere_) {
	unsigned int i;
	
	prog_draw= prog_draw_;
	prog_repere= prog_repere_;
	
	lights_ubo.set_prog(prog_draw);
	lights_ubo.init();
	
	is_debug= false;
	
	//config.randomize();
	
	/*string s;
	for (int channel_number=0; channel_number<N_MAX_CHANNELS; channel_number++)
		for (i=0; i< config.channels_configs[channel_number].actions.size(); i++)
			if (config.channels_configs[channel_number].actions[i]->type== LOAD_OBJ) {
				config.channels_configs[channel_number].actions[i]->save_xml(s);
				//cout << "test=" << channel_number << " ; " << i << " : " << config.channels_configs[channel_number].actions[i]->lifetime << endl;
			}*/
	// ----------------------------------------------------------------------------------------------------
	for (i=0; i<N_MAX_CHANNELS; i++) {
		// cf client/main.c : on emet de PORT_NUMBER à PORT_NUMBER+ N_MAX_CHANNELS- 1
		socket_channel_listen(PORT_NUMBER+ i, sc+ i);
		
		// des le debut on cree N_MAX_CHANNELS Channels
		channels.push_back(Channel());
	}
	
	socket_channel_listen(PORT_NUMBER_FLTK, &sc_fltk); // sur 49999 on recupere les XML de fltk

	// ----------------------------------------------------------------------------------------------------
	
}


void Audio2Video::read_socket() {
	char buffer[SOCKET_BUFFER_SIZE];
	unsigned int channel_number;
	
	for (channel_number=0; channel_number<N_MAX_CHANNELS; channel_number++) {
		bzero(buffer, SOCKET_BUFFER_SIZE);
		int n = read(sc[channel_number].sockfd, buffer, 1);

		if (n> 0) {
			if (!channels[channel_number].triggable)
				return;
		
			if (is_debug) cout << "trig channel " << channel_number << endl;
		
			channels[channel_number].triggable= false;
			gettimeofday(&(channels[channel_number].last_trigger), NULL);
			
			for (unsigned int i=0; i< config.channels_configs[channel_number].actions.size(); i++) {
				if (config.channels_configs[channel_number].actions[i]->active)
					trig(channel_number, i);
			}
		}
	}
}


void Audio2Video::read_socket_fltk() {
	char buffer[SOCKET_BUFFER_SIZE_FLTK];
	bzero(buffer, SOCKET_BUFFER_SIZE_FLTK);
	int n = read(sc_fltk.sockfd, buffer, SOCKET_BUFFER_SIZE_FLTK- 1);
	if (n> 0) {
		//cout << string(buffer) << endl;
		istringstream istr(buffer);
		istr >> config;
	}
}


void Audio2Video::trig(unsigned int idx_channel, unsigned int idx_action) {
	ActionConfig * ac= config.channels_configs[idx_channel].actions[idx_action];
	struct timeval now;
	gettimeofday(&now, NULL);
	unsigned int i;

	if (ac->type== LOAD_OBJ) {
		if (model_objs.size()>= N_MAX_OBJS)
			return;
	
		if (is_debug) cout << "load_obj " << endl;
		
		// si retrig activé, supprimer les objets qui ont cette config
		if (ac->retrig) {
			bool done= false;
			while (!done) {
				done= true;
				for (i=0; i<model_objs.size(); i++) {
					if (model_objs[i].config== ac) {
						model_objs.erase(model_objs.begin()+ i);
						done= false;
						break;
					}
				}
			}
		}
		
		ModelObj model_obj(prog_draw, now);
		model_obj.set_config((ModelObjConfig *)(ac));
		model_objs.push_back(model_obj);
	}
	else if (ac->type== LOAD_LIGHT) {
		if (lights.size()>= N_MAX_LIGHTS)
			return;
	
		if (is_debug) cout << "load_light " << endl;

		// si retrig activé, supprimer les lights qui ont cette config
		if (ac->retrig) {
			bool done= false;
			while (!done) {
				done= true;
				for (i=0; i<lights.size(); i++) {
					if (lights[i].config== ac) {
						lights.erase(lights.begin()+ i);
						done= false;
						break;
					}
				}
			}
		}

		Light light(true, prog_repere, now);
		light.set_config((LightConfig *)(ac));
		lights.push_back(light);
		lights_ubo.update(lights);
	}
	else if (ac->type== LOAD_VMAT) {
		if (vmats.size()>= N_MAX_VMATS)
			return;

		if (is_debug) cout << "load_vmat " << endl;

		// si retrig activé, supprimer les vmats qui ont cette config
		if (ac->retrig) {
			bool done= false;
			while (!done) {
				done= true;
				for (i=0; i<vmats.size(); i++) {
					if (vmats[i].config== ac) {
						vmats.erase(vmats.begin()+ i);
						done= false;
						break;
					}
				}
			}
		}

		VMat vmat(now);
		vmat.set_config((VMatConfig *)(ac));
		vmats.push_back(vmat);
	}
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
		glm::mat4 glm_vmat= glm::make_mat4(vmats[i].current_values.mat);
		glm_world2camera_vmat= glm_world2camera_vmat* glm_vmat;
	}
	float world2camera_vmat[16];
	memcpy(world2camera_vmat, glm::value_ptr(glm_world2camera_vmat), sizeof(float) * 16);
	
	for (unsigned int i=0; i< model_objs.size(); i++)
		//model_objs[i].anim(world2camera, camera2clip);
		model_objs[i].anim(world2camera_vmat, camera2clip);

	for (unsigned int i=0; i< lights.size(); i++)
		lights[i].anim(world2camera);
	lights_ubo.update(lights);

}


void Audio2Video::idle() {
	unsigned int i;
	bool all_actives= false;
	
	read_socket_fltk();
	
	// lire le socket; éventuellement rendre triggables des channels
	read_socket();
	for (i=0; i< channels.size(); i++)
		channels[i].update_triggered();
	
	// supprimer les objets, lights et vmats inactifs

	all_actives= false;
	while (!all_actives) {
		all_actives= true;
		for (i=0; i<model_objs.size(); i++) {
			if (!model_objs[i].is_active) {
				model_objs.erase(model_objs.begin()+ i);
				all_actives= false;
				break;
			}
		}
	}
	
	all_actives= false;
	while (!all_actives) {
		all_actives= true;
		for (i=0; i<lights.size(); i++) {
			if (!lights[i].is_active) {
				lights.erase(lights.begin()+ i);
				lights_ubo.update(lights);
				all_actives= false;
				break;
			}
		}
	}

	all_actives= false;
	while (!all_actives) {
		all_actives= true;
		for (i=0; i<vmats.size(); i++) {
			if (!vmats[i].is_active) {
				vmats.erase(vmats.begin()+ i);
				all_actives= false;
				break;
			}
		}
	}

}


void Audio2Video::release() {
	unsigned int i;
	
	for (i=0; i<N_MAX_CHANNELS; i++)
		close(sc[i].sockfd);
	
	for (i=0; i<model_objs.size(); i++)
		model_objs[i].release();

	lights_ubo.release();
}


void Audio2Video::clear() {
	/*for (unsigned int i=0; i< channels.size(); i++)
		channels[i].clear_actions();*/
	model_objs.clear();
	lights.clear();
	lights_ubo.update(lights);
	vmats.clear();
}


void Audio2Video::print() {
	unsigned int i, j;
	
	cout << "A2V DEBUG =======================================================" << endl;
	
	cout << "config=" << config << endl;
	
	for (i=0; i<config.channels_configs.size(); i++) {
		for (j=0; j<config.channels_configs[i].actions.size(); j++)
			cout << "channel " << i << " action " << j << " : type=" << config.channels_configs[i].actions[j]->type << endl;
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

