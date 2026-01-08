/*

http://dranger.com/ffmpeg/tutorial01.html

Utilisation :
./decode_mp4 ../data/test.mp4 ../data/result

*/

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <SDL2/SDL.h>

#include <iostream>

using namespace std;


int SCREEN_WIDTH= 1024;
int SCREEN_HEIGHT= 1024;


SDL_Window * window= 0;
SDL_Renderer * renderer= 0;
SDL_Surface * surf;
SDL_Texture * tex;
Uint32 rmask, gmask, bmask, amask;
bool done= false;

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
int buffer_rgb_size= 0;
unsigned char * buffer_all= 0;

// servira a faire la conversion de format de couleur entre frame et frame_rgb
struct SwsContext * sws_ctx= 0;

// indice du stream video du fichier en entrée
int stream_idx= 0;

// valeurs de retour des fonctions
int ret= 0;

int n_frames= 0;
int width= 0;
int height= 0;
int current_idx= 0;



int init(const char * file_in) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		cout << "SDL_Init failed: " << SDL_GetError() << "\n";
		return 1;
	}

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// RGBA
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif

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

	// Determine required buffer size and allocate buffer
	buffer_rgb_size= av_image_get_buffer_size(pixel_format, width, height, 32);
	buffer_rgb= (unsigned char *)av_malloc(buffer_rgb_size* sizeof(unsigned char));

	// Assign appropriate parts of buffer to image planes in frame_rgb
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer_rgb, pixel_format, width, height, 1);

	// servira a faire la conversion de format de couleur
	sws_ctx= sws_getContext(width, height, ctx_codec->pix_fmt, width, height, pixel_format, SWS_BILINEAR, 0, 0, 0);


	n_frames= ctx_format->streams[stream_idx]->nb_frames;
	cout << "NB Frames estimated = " << n_frames << "\n";
	buffer_all= (unsigned char *)av_malloc(buffer_rgb_size* sizeof(unsigned char)* n_frames);


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
				for (int i= 0; i< width* height; i++) {
					uint alpha= (255* (i% width))/ width;
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

	return 0;
}


void idle() {
	SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
	SDL_RenderClear(renderer);

	current_idx++;
	if (current_idx>= n_frames) {
		current_idx= 0;
	}

	// si canal alpha 32, sinon 24
	//int depth = 24;
	int depth = 32;

	// si canal alpha 4, sinon 3
	//int pitch = 3* width;
	int pitch = 4* width;

	surf= SDL_CreateRGBSurfaceFrom((void*)(buffer_all+ current_idx* buffer_rgb_size* sizeof(unsigned char)), width, height, depth, pitch, rmask, gmask, bmask, amask);
	tex= SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	SDL_Rect texture_rect;
	texture_rect.x = 0;
	texture_rect.y = 0;
	texture_rect.w = SCREEN_WIDTH;
	texture_rect.h = SCREEN_HEIGHT;

	SDL_RenderCopy(renderer, tex, NULL, &texture_rect);
	SDL_DestroyTexture(tex);
	
	SDL_RenderPresent(renderer);
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
	SDL_DestroyRenderer(renderer);
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
	
	ret= init(file_in);
	main_loop();
	clean();
	
	return 0;
}



