#include <iostream>
#include <chrono>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_utils.h"
#include "utile.h"


using namespace std;


const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(SCREEN_HEIGHT)/ (float)(SCREEN_WIDTH);
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;


SDL_Window * window= 0;
SDL_GLContext main_context;
GLuint prog_texture_3d;
GLuint vao;
GLuint vbo;
GLuint texture_3d_id, texture_index_3d_id;
GLint camera2clip_loc, model2world_loc, position_loc, texture_3d_loc, texture_index_3d_loc, screen_width_loc, screen_height_loc, index_index_loc;
glm::mat4 camera2clip;
glm::mat4 model2world;

ScreenGL * screengl;
bool done= false;
chrono::system_clock::time_point t1, t2;

unsigned char * buffer_all= 0;
int n_frames= 0;
unsigned int width= 0;
unsigned int height= 0;
unsigned int compt= 0;
unsigned int n_index_index= 256;
float index_index= 0.0f;


int init_ffmpeg(const char * file_in) {
	// valeurs de retour des fonctions
	int ret= 0;

	// Format I/O context
	AVFormatContext * ctx_format = 0;

	// main external API structure
	AVCodecContext * ctx_codec = 0;

	// Codec
	const AVCodec * codec = 0;

	// This structure describes decoded (raw) audio or video data
	AVFrame * frame= av_frame_alloc();
	AVFrame * frame_rgb= av_frame_alloc();

	// Structure permettant de manipuler la data
	SwsContext * ctx_sws = 0;

	// Stream
	AVStream * vid_stream = 0;

	/*
	This structure stores compressed data.
	It is typically exported by demuxers and then passed as input to decoders, or received as output from encoders and then passed to muxers.
	For video, it should typically contain one compressed frame. For audio it may contain several compressed frames.
	Encoders are allowed to output empty packets, with no compressed data, containing only side data (e.g. to update some stream parameters at the end of encoding).
	*/
	AVPacket * pkt = av_packet_alloc();

	// buffer qui servira a stocker les données de frame_rgb après conversion du frame
	unsigned char * buffer_rgb= 0;
	// ici long pas int ! sinon bug lors du malloc
	unsigned long buffer_rgb_size= 0;

	// servira a faire la conversion de format de couleur entre frame et frame_rgb
	struct SwsContext * sws_ctx= 0;

	// indice du stream video du fichier en entrée
	int stream_idx= 0;

	/*
	Allocate memory for AVFormatContext.
	Read the probe_size about of data from the file (input url)
	Tries to guess the input file format, codec parameter for the input file. This is done by calling read_probe function pointer for each of the demuxer.
	Allocate the codec context, demuxed context, I/O context.
	*/
	ret = avformat_open_input(&ctx_format, file_in, 0, 0);
	if (ret!= 0) {
		cout << "ERREUR avformat_open_input\n";
		return ret;
	}

	/*
	Read packets of a media file to get stream information.
	This is useful for file formats with no headers such as MPEG. This function also computes the real framerate in case of MPEG-2 repeat frame mode.
	The logical file position is not changed by this function; examined packets may be buffered for later processing.
	*/
	ret= avformat_find_stream_info(ctx_format, 0);
	if (ret< 0) {
		cout << "ERREUR avformat_find_stream_info\n";
		return ret; // Couldn't find stream information
	}

	// Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base.
	//av_dump_format(ctx_format, 0, file_in, false);

	// pour chaque stream, si c'est un stream video on l'assigne à vid_stream
	for (int i = 0; i < ctx_format->nb_streams; i++)
		if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			stream_idx = i;
			vid_stream = ctx_format->streams[i];
			break;
	}
	if (vid_stream== 0) {
		cout << "ERREUR pas de stream video trouvé\n";
		return 1;
	}

	//float frame_rate= av_q2d(ctx_format->streams[stream_idx]->r_frame_rate);
	//cout << "frame rate= " << frame_rate << "\n";

	// 	Find a registered decoder with a matching codec ID
	codec= avcodec_find_decoder(vid_stream->codecpar->codec_id);
	if (!codec) {
		cout << "ERREUR avcodec_find_decoder\n";
		return 1;
	}

	// Allocate an AVCodecContext and set its fields to default values.
	ctx_codec= avcodec_alloc_context3(codec);

	// 	Fill the codec context based on the values from the supplied codec parameters
	ret= avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar);
	if (ret< 0) {
		cout << "ERREUR avcodec_parameters_to_context\n";
		return ret;
	}
	
	// Initialize the codec context to use the given AVCodec.
	ret= avcodec_open2(ctx_codec, codec, 0);
	if (ret< 0) {
		cout << "ERREUR avcodec_open2\n";
		return ret;
	}

	width= ctx_codec->width;
	height= ctx_codec->height;

	// AV_PIX_FMT_RGBA permet de rajouter un canal alpha
	//AVPixelFormat pixel_format= AV_PIX_FMT_RGB24;
	AVPixelFormat pixel_format= AV_PIX_FMT_RGBA;

	unsigned int width_rgb= width;
	unsigned int height_rgb= height;

	// Determine required buffer size and allocate buffer
	buffer_rgb_size= av_image_get_buffer_size(pixel_format, width_rgb, height_rgb, 32);
	buffer_rgb= (unsigned char *)av_malloc(buffer_rgb_size);

	// Assign appropriate parts of buffer to image planes in frame_rgb
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer_rgb, pixel_format, width_rgb, height_rgb, 1);

	// servira a faire la conversion de format de couleur
	//sws_ctx= sws_getContext(width, height, ctx_codec->pix_fmt, width, height, pixel_format, SWS_BILINEAR, 0, 0, 0);
	sws_ctx= sws_getContext(width, height, ctx_codec->pix_fmt, width_rgb, height_rgb, pixel_format, SWS_BILINEAR, 0, 0, 0);

	n_frames= ctx_format->streams[stream_idx]->nb_frames;
	cout << "NB Frames estimated = " << n_frames << "\n";

	// av_malloc échoue si la taille est trop grosse !
	buffer_all= (unsigned char *)malloc(buffer_rgb_size* n_frames);
	if (buffer_all== 0) {
		cout << "error malloc : " << buffer_rgb_size* n_frames << " bytes\n";
		exit(1);
	}
	
	// Return the next frame of a stream
	while (av_read_frame(ctx_format, pkt)>= 0){
		// s'il s'agit du bon stream
		if (pkt->stream_index == stream_idx) {
			// Supply raw packet data as input to a decoder
			ret= avcodec_send_packet(ctx_codec, pkt);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				cout << "avcodec_send_packet: " << ret << "\n";
				break;
			}

			while (ret>= 0) {
				// Return decoded output data from a decoder
				// AVERROR_EOF: the decoder has been fully flushed, and there will be no more output frames
				// AVERROR(EAGAIN): output is not available in this state
				ret= avcodec_receive_frame(ctx_codec, frame);
				if ((ret== AVERROR_EOF) || (ret== AVERROR(EAGAIN))) {
					//cout << "avcodec_receive_frame: " << ret << "\n";
					break;
				}

				//cout << "frame: " << ctx_codec->frame_number << "\n";
				n_frames= ctx_codec->frame_number;

				// conversion de frame vers frame_rgb
				sws_scale(sws_ctx, (unsigned char const * const *)frame->data, frame->linesize, 0, height, frame_rgb->data, frame_rgb->linesize);
	
				// gestion canal alpha
				for (int i= 0; i< width_rgb* height_rgb; i++) {
					//unsigned int alpha= (255* (i% width_rgb))/ width_rgb;
					unsigned int alpha= 255;
					buffer_rgb[4* i+ 3]= alpha;
				}
				memcpy(buffer_all+ (ctx_codec->frame_number- 1)* buffer_rgb_size* sizeof(unsigned char), buffer_rgb, buffer_rgb_size* sizeof(unsigned char));
			}

		}

		// Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
		av_packet_unref(pkt);
	}

	cout << "NB Frames exact = " << n_frames << "\n";

	// close format context
	avformat_close_input(&ctx_format);
	av_packet_unref(pkt);
	// Free the codec context and everything associated with it and write NULL to the provided pointer
	avcodec_free_context(&ctx_codec);
	// Free an AVFormatContext and all its streams.
	avformat_free_context(ctx_format);

	av_free(buffer_rgb);

	return 0;
}


void init_sdl() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("decode_mpeg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);
	//glEnable(GL_DEPTH_CLAMP);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//glPointSize(4.0f);
	
	SDL_GL_SwapWindow(window);

	screengl= new ScreenGL(SCREEN_WIDTH, SCREEN_HEIGHT, GL_WIDTH, GL_HEIGHT);
}


void init_vao_vbo() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	float vertices[18];
	float x= -0.5f* screengl->_gl_width;
	float y= -0.5f* screengl->_gl_height;
	float z= 0.0f;
	float w= screengl->_gl_width;
	float h= screengl->_gl_height;

	vertices[0]= x;
	vertices[1]= y+ h;
	vertices[2]= z;

	vertices[3]= x;
	vertices[4]= y;
	vertices[5]= z;

	vertices[6]= x+ w;
	vertices[7]= y;
	vertices[8]= z;

	vertices[9]= x;
	vertices[10]= y+ h;
	vertices[11]= z;

	vertices[12]= x+ w;
	vertices[13]= y;
	vertices[14]= z;

	vertices[15]= x+ w;
	vertices[16]= y+ h;
	vertices[17]= z;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 18* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init_program() {
	// cf https://community.khronos.org/t/2-samplers-fs-fails-different-sampler-types-for-same-sample-texture-unit-in-fragment/72598
	// il faut faire les glUniform1i avant check_gl_program, d'où l'option check de create_prog à false, et check_gl_program + tard
	prog_texture_3d= create_prog("../../shaders/vertexshader_3d_texture.txt"  , "../../shaders/fragmentshader_3d_texture.txt", false);
	camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	model2world= glm::mat4(1.0f);

	glUseProgram(prog_texture_3d);
	camera2clip_loc= glGetUniformLocation(prog_texture_3d, "camera2clip_matrix");
	model2world_loc= glGetUniformLocation(prog_texture_3d, "model2world_matrix");
	texture_3d_loc= glGetUniformLocation(prog_texture_3d, "texture_3d");
	texture_index_3d_loc= glGetUniformLocation(prog_texture_3d, "texture_index_3d");
	screen_width_loc= glGetUniformLocation(prog_texture_3d, "screen_width");
	screen_height_loc= glGetUniformLocation(prog_texture_3d, "screen_height");
	index_index_loc= glGetUniformLocation(prog_texture_3d, "index_index");
	position_loc= glGetAttribLocation(prog_texture_3d, "position_in");

	glUniform1i(texture_3d_loc, 0); //Sampler refers to texture unit 0
	glUniform1i(texture_index_3d_loc, 1);

	glUseProgram(0);

	check_gl_program(prog_texture_3d);

	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
}


float index_function(float x, float y, float t) {
	//return 0.0f;
	//return t;
	/*if ((x>0.1f) && (x<0.4f) && (y>0.1f) && (y<0.3f)) {
		return t* 0.5f;
	}
	else {
		return t;
	}*/
	//return t* sqrt(2.0f)* sqrt(0.5f- (x- 0.5)* (x- 0.5)- (y- 0.5)* (y- 0.5));
	return t* rand_float(0.0f, 1.0f);
	/*if (rand_int(0, 1)) {
		return t;
	}
	else {
		return 1.0f- t;
	}*/
}


void init_texture() {
	// ----------------------------------------
	glGenTextures(1, &texture_3d_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_3d_id);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, n_frames, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_all);
	av_free(buffer_all);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(0);

	// ----------------------------------------
	glGenTextures(1, &texture_index_3d_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_index_3d_id);

	float * data_index= new float[SCREEN_WIDTH* SCREEN_HEIGHT* n_index_index];
	for (unsigned int j=0; j<n_index_index; ++j) {
		float r= rand_float(0.0f, 1.0f);
		//float index_f= (float)(j)/ (float)(n_index_index);
		int m= 10;
		int k= rand_int(-m, m);
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			float x= (float)(i % SCREEN_WIDTH)/ (float)(SCREEN_WIDTH);
			float y= (float)(i / SCREEN_WIDTH)/ (float)(SCREEN_HEIGHT);
			//data_index[i+ SCREEN_WIDTH* SCREEN_HEIGHT* j]= r;
			//data_index[i+ SCREEN_WIDTH* SCREEN_HEIGHT* j]= index_function(x, y, index_f);
			if (j== 0) {
				data_index[i+ SCREEN_WIDTH* SCREEN_HEIGHT* j]= r;
			}
			else {
				data_index[i+ SCREEN_WIDTH* SCREEN_HEIGHT* j]= data_index[i+ SCREEN_WIDTH* SCREEN_HEIGHT* (j- 1)]+ k/ (float)(n_index_index);
			}
		}
	}
	for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT* n_index_index; ++i) {
		if (data_index[i]< 0.0f) {
			data_index[i]= 0.0f;
		}
		if (data_index[i]> 1.0f) {
			data_index[i]= 1.0f;
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, n_index_index, 0, GL_RED, GL_FLOAT, data_index);
	delete[] data_index;

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(0);
}


void init(const char * file_in) {
	srand(time(NULL));

	int ret= init_ffmpeg(file_in);
	if (ret!= 0) {
		cerr << "ffmpeg error " << ret << "\n";
		exit(1);
	}

	init_sdl();
	init_vao_vbo();
	init_program();
	init_texture();
}


void update() {
	compt++;
	if (compt>= n_index_index) {
		compt= 0;
	}
	index_index= (float)(compt)/ (float)(n_index_index);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_index_3d_id);
	float data_index[SCREEN_WIDTH* SCREEN_HEIGHT];
	for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
		data_index[i]= rand_float(0.0f, 1.0f);
		//data_index[i]= t;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, data_index);*/
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glUseProgram(prog_texture_3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_3d_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_index_3d_id);

	glUniform1i(texture_3d_loc, 0); //Sampler refers to texture unit 0
	glUniform1i(texture_index_3d_loc, 1);
	glUniform1i(screen_width_loc, SCREEN_WIDTH);
	glUniform1i(screen_height_loc, SCREEN_HEIGHT);
	glUniform1f(index_index_loc, index_index);
	glUniformMatrix4fv(camera2clip_loc, 1, GL_FALSE, glm::value_ptr(camera2clip));
	glUniformMatrix4fv(model2world_loc, 1, GL_FALSE, glm::value_ptr(model2world));
	
	glEnableVertexAttribArray(position_loc);

	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(position_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_3D, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);
	
	SDL_GL_SwapWindow(window);
}


void compute_fps() {
	t2= chrono::system_clock::now();
	auto d= chrono::duration_cast<chrono::milliseconds>(t2- t1).count();
	t1= t2;

	char s_fps[256];
	sprintf(s_fps, "%lld", d);
	SDL_SetWindowTitle(window, s_fps);
}


void idle() {
	update();
	draw();
	compute_fps();
}


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		while (SDL_PollEvent(& event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym== SDLK_ESCAPE) {
						done= true;
						return;
					}
					break;
					
				case SDL_KEYUP:
					break;
					
				case SDL_QUIT:
					done= true;
					break;
					
				default:
					break;
			}
		}
		idle();
	}
}


void clean() {
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main(int argc, char **argv) {
	if (argc!= 2) {
		cerr << "Donner en argument le fichier mpeg en entrée\n";
		return 1;
	}
	// fichier mp4 en entrée
	const char * file_in= argv[1];

	init(file_in);
	main_loop();
	clean();
	
	return 0;
}



