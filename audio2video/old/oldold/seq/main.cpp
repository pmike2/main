#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <FL/Fl.H>

#include "seq.h"
#include "constantes_seq.h"


using namespace std;


// ----------------------------------------------------------------------------------------------------
struct SockStruct {
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
};


Seq * seq;
SockStruct socks[N_CHANNELS];
int current_tempo= 100;
int current_step= 0;
bool is_playing= false;


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


void play_clicked(Fl_Widget *w, void *data) {
	is_playing= true;
}


void stop_clicked(Fl_Widget *w, void *data) {
	is_playing= false;
}


void tempo_clicked(Fl_Widget *w, void *data) {
	Fl_Dial * dial= (Fl_Dial *)w;
	current_tempo= int(dial->value());
	//cout << dial->value() << endl;
}


void clear_clicked(Fl_Widget *w, void *data) {
	unsigned int i, j;
	for (i=0; i<N_CHANNELS; i++)
		for (j=0; j<COLS; j++)
			seq->main_scroll->w[(i+ 1)* COLS+ j]->value(0);
}


void random_clicked(Fl_Widget *w, void *data) {
	unsigned int i, j;
	unsigned int chance= 4;
	for (i=0; i<N_CHANNELS; i++)
		for (j=0; j<COLS; j++)
			if (rand()% chance== 0)
				seq->main_scroll->w[(i+ 1)* COLS+ j]->value(1);
			else
				seq->main_scroll->w[(i+ 1)* COLS+ j]->value(0);
}


void trig(unsigned int channel) {
	char c= '1';
	//cout << "trig " << channel << endl;
	int n = write(socks[channel].newsockfd, &c, sizeof(c));
	if (n < 0)
		printf("ERROR writing to socket\n");
}


void anim(void*) {
	unsigned int i;
	
	if (is_playing) {
		current_step++;
		if (current_step>= COLS)
			current_step= 0;
		cout <<current_step<<endl;
		for (i=0; i<COLS; i++)
			seq->main_scroll->w[i]->value(0);
	
		seq->main_scroll->w[current_step]->value(1);
		
		for (i=0; i<N_CHANNELS; i++)
			if (seq->main_scroll->w[current_step+ COLS* (i+ 1)]->value())
				trig(i);
	}
	
	Fl::repeat_timeout(60.0/ float(current_tempo), anim);
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
	srand(time(NULL));
	
	for (unsigned int i=0; i<N_CHANNELS; i++)
		init_server_socket(PORT+ i, &(socks[i]));
	
	seq = new Seq();
	
	seq->play->callback(play_clicked);
	seq->stop->callback(stop_clicked);
	seq->tempo->callback(tempo_clicked);
	seq->clear->callback(clear_clicked);
	seq->random->callback(random_clicked);
	
	is_playing= true;
	seq->main_scroll->w[1* COLS+ 0]->value(1);
	seq->main_scroll->w[1* COLS+ 4]->value(1);
	seq->main_scroll->w[1* COLS+ 8]->value(1);
	seq->main_scroll->w[1* COLS+ 12]->value(1);
	seq->main_scroll->w[2* COLS+ 0]->value(1);
	seq->main_scroll->w[2* COLS+ 8]->value(1);
	
	seq->main_window->show(argc, argv);
	
	current_tempo= int(seq->tempo->value());
	Fl::add_timeout(60.0/ float(current_tempo), anim);
	
	return Fl::run();
	
	return 0;
}

