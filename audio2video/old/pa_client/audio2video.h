
#ifndef AUDIO2VIDEO_H
#define AUDIO2VIDEO_H

#include <vector>
#include <string>
#include <sys/time.h>
#include <sstream>
#include <math.h>

#include <thread>
#include <mutex>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"

#include "light.h"
#include "objfile.h"
#include "vmat.h"
#include "config.h"
#include "constantes.h"





class CC {
public:
	CC();
	
	float value, value_max, value_min;
};


class Audio2Video : public osc::OscPacketListener {
public:
	Audio2Video();
	Audio2Video(GLuint prog_draw_, GLuint prog_repere_);
	void draw(float * world2clip);
	void anim(float * world2camera, float * camera2clip);
	void idle();
	void release();
	void clear();
	void print();
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void osc_thread_method();
	virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint);
	void set_param(unsigned int idx_channel, unsigned int idx_action, std::string key, std::string val);
	
	
	GLuint prog_draw, prog_repere; // prog_repere sert au dessin des lights
	
	std::vector<ModelObj> model_objs;
	std::vector<Light> lights;
	LightsUBO lights_ubo;
	std::vector<VMat> vmats;
	std::vector<ChannelConfig *> channels_configs;

	UdpListeningReceiveSocket * osc_socket;
	std::thread * osc_thread;
	std::vector<CC> ccs;
	std::mutex mtxs[N_MAX_CHANNELS]; // pas de vector de mutex ; cf https://stackoverflow.com/questions/16465633/how-can-i-use-something-like-stdvectorstdmutex
	
	bool is_debug;
};


#endif
