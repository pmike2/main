
#ifndef AUDIO2VIDEO_H
#define AUDIO2VIDEO_H

#include <vector>
#include <string>
#include <sys/time.h>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "light.h"
#include "objfile.h"
#include "vmat.h"
#include "config.h"
#include "constantes.h"
#include "channel.h"


struct SocketChannel {
	std::string hostname;
	int portno;
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
};


void socket_channel_listen(unsigned int portno, SocketChannel * sc);


class Audio2Video{
public:
	Audio2Video();
	Audio2Video(GLuint prog_draw_, GLuint prog_repere_);
	void read_socket();
	void read_socket_fltk();
	void trig(unsigned int idx_channel, unsigned int idx_action);
	void draw(float * world2clip);
	void anim(float * world2camera, float * camera2clip);
	void idle();
	void release();
	void clear();
	void print();
	
	
	SocketChannel sc[N_MAX_CHANNELS];
	SocketChannel sc_fltk;
	GLuint prog_draw, prog_repere; // prog_repere sert au dessin des lights
	
	std::vector<Channel> channels;
	std::vector<ModelObj> model_objs;
	std::vector<Light> lights;
	LightsUBO lights_ubo;
	std::vector<VMat> vmats;
	A2VConfig config;
	
	bool is_debug;
};


#endif
