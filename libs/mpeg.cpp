#include <iostream>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <OpenGL/gl3.h>

#include "mpeg.h"
#include "utile.h"


using namespace std;


MPEG::MPEG() : _n_frames(0), _fps(0), _data(0), _width(0), _height(0), _frame_size(0) {

}


MPEG::MPEG(string mpeg_path) {
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

	// servira a faire la conversion de format de couleur entre frame et frame_rgb
	struct SwsContext * sws_ctx= 0;

	// indice du stream video du fichier en entrée
	int stream_idx= 0;

	// valeurs de retour des fonctions
	int ret= 0;

	/*
	Allocate memory for AVFormatContext.
	Read the probe_size about of data from the file (input url)
	Tries to guess the input file format, codec parameter for the input file. This is done by calling read_probe function pointer for each of the demuxer.
	Allocate the codec context, demuxed context, I/O context.
	*/
	ret = avformat_open_input(&ctx_format, mpeg_path.c_str(), 0, 0);
	if (ret!= 0) {
		cout << "ERREUR avformat_open_input\n";
		return;
	}

	/*
	Read packets of a media file to get stream information.
	This is useful for file formats with no headers such as MPEG. This function also computes the real framerate in case of MPEG-2 repeat frame mode.
	The logical file position is not changed by this function; examined packets may be buffered for later processing.
	*/
	ret= avformat_find_stream_info(ctx_format, 0);
	if (ret< 0) {
		cout << "ERREUR avformat_find_stream_info\n";
		return; // Couldn't find stream information
	}

	// Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base.
	//av_dump_format(ctx_format, 0, sample_path.c_str(), false);

	// pour chaque stream, si c'est un stream video on l'assigne à vid_stream
	for (int i = 0; i < ctx_format->nb_streams; i++)
		if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			stream_idx = i;
			vid_stream = ctx_format->streams[i];
			break;
	}
	if (vid_stream== 0) {
		cout << "ERREUR pas de stream video trouvé\n";
		return;
	}

	_fps= av_q2d(ctx_format->streams[stream_idx]->r_frame_rate);

	// 	Find a registered decoder with a matching codec ID
	codec= avcodec_find_decoder(vid_stream->codecpar->codec_id);
	if (!codec) {
		cout << "ERREUR avcodec_find_decoder\n";
		return;
	}

	// Allocate an AVCodecContext and set its fields to default values.
	ctx_codec= avcodec_alloc_context3(codec);

	// 	Fill the codec context based on the values from the supplied codec parameters
	ret= avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar);
	if (ret< 0) {
		cout << "ERREUR avcodec_parameters_to_context\n";
		return;
	}
	
	// Initialize the codec context to use the given AVCodec.
	ret= avcodec_open2(ctx_codec, codec, 0);
	if (ret< 0) {
		cout << "ERREUR avcodec_open2\n";
		return;
	}

	_width= ctx_codec->width;
	_height= ctx_codec->height;

	// Determine required buffer size and allocate buffer
	buffer_rgb_size= av_image_get_buffer_size(AV_PIX_FMT_RGB24, _width, _height, 32);
	_frame_size= buffer_rgb_size* sizeof(unsigned char);
	buffer_rgb= (unsigned char *)av_malloc(_frame_size);

	// Assign appropriate parts of buffer to image planes in frame_rgb
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer_rgb, AV_PIX_FMT_RGB24, _width, _height, 1);

	// servira a faire la conversion de format de couleur
	sws_ctx= sws_getContext(_width, _height, ctx_codec->pix_fmt, _width, _height, AV_PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);

	_n_frames= ctx_format->streams[stream_idx]->nb_frames;
	//cout << "NB Frames estimated = " << _n_frames << "\n";
	_data= (unsigned char *)av_malloc(_frame_size* _n_frames);

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
				_n_frames= ctx_codec->frame_number;

				// conversion de frame vers frame_rgb
				sws_scale(sws_ctx, (unsigned char const * const *)frame->data, frame->linesize, 0, _height, frame_rgb->data, frame_rgb->linesize);
	
				memcpy(_data+ (ctx_codec->frame_number- 1)* _frame_size, buffer_rgb, _frame_size);
			}

		}

		// Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
		av_packet_unref(pkt);
	}

	// close format context
	avformat_close_input(&ctx_format);
	av_packet_unref(pkt);
	// Free the codec context and everything associated with it and write NULL to the provided pointer
	avcodec_free_context(&ctx_codec);
	// Free an AVFormatContext and all its streams.
	avformat_free_context(ctx_format);
}


MPEG::~MPEG() {
	av_free(_data);
}


void * MPEG::get_frame(unsigned int frame_idx) {
	return (void *)(_data+ frame_idx* _frame_size* sizeof(unsigned char));
}



// -------------------------------------------------------------
MPEGTextures::MPEGTextures() {

}


MPEGTextures::MPEGTextures(vector<string> mpeg_paths, int loc, int base_index) :
	_base_index(base_index), _loc(loc)
{
	glGenTextures(N_MAX_TEXTURES, _ids);
	for (unsigned int i=0; i<N_MAX_TEXTURES; ++i) {
		_indices[i]= _base_index+ i;
	}

	for (unsigned int i=0; i<N_MAX_TEXTURES; ++i) {
		if (i>= mpeg_paths.size()) {
			_ids[i]= _ids[0];
			continue;
		}
		MPEG * mpeg= new MPEG(mpeg_paths[i]);
		
		glActiveTexture(GL_TEXTURE0+ _base_index+ i);
		glBindTexture(GL_TEXTURE_3D, _ids[i]);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, mpeg->_width, mpeg->_height, mpeg->_n_frames, 0, GL_RGB, GL_UNSIGNED_BYTE, mpeg->_data);
		delete mpeg;

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	glUniform1iv(_loc, N_MAX_TEXTURES, &_indices[0]);
}


MPEGTextures::~MPEGTextures() {

}


void MPEGTextures::prepare2draw() {
	glUniform1iv(_loc, N_MAX_TEXTURES, &_indices[0]);
	for (unsigned int i=0; i<N_MAX_TEXTURES; ++i) {
		glActiveTexture(GL_TEXTURE0+ _base_index+ i);
		glBindTexture(GL_TEXTURE_3D, _ids[i]);
	}
}


// -----------------------------------------------------------------------------------
MPEGReaders::MPEGReaders() {

}


MPEGReaders::MPEGReaders(unsigned int n_readers,
		unsigned int alpha_width, unsigned int alpha_height, unsigned int alpha_texture_index, unsigned int alpha_loc,
		unsigned int time_height, unsigned int time_texture_index, unsigned int time_loc,
		unsigned int index_time_texture_index, unsigned int index_time_loc) :
	_alpha_width(alpha_width), _alpha_height(alpha_height), _alpha_depth(n_readers), _alpha_texture_index(alpha_texture_index), _alpha_loc(alpha_loc),
	_time_width(n_readers), _time_height(time_height), _time_texture_index(time_texture_index), _time_loc(time_loc),
	_index_time_width(n_readers), _index_time_texture_index(index_time_texture_index), _index_time_loc(index_time_loc)
{
	// -------------------
	glGenTextures(1, &_alpha_id);

	_alpha_data= new float[_alpha_width* _alpha_height* _alpha_depth];
	for (unsigned int i=0; i<_alpha_width* _alpha_height* _alpha_depth; ++i) {
		_alpha_data[i]= 0.0f;
	}

	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, _alpha_width, _alpha_height, _alpha_depth, 0, GL_RED, GL_FLOAT, _alpha_data);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	/*for (unsigned int i=0; i<_alpha_depth; ++i) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _width, _height, 1, GL_RGBA, GL_FLOAT, _data0[i]);
	}*/
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glUniform1i(_alpha_loc, _alpha_texture_index);

	// -------------------
	glGenTextures(1, &_time_id);

	_time_data= new float[_time_width* _time_height];
	for (unsigned int i=0; i<_time_width* _time_height; ++i) {
		//_time_data[i]= 0.0f;
		_time_data[i]= rand_float(0.0f, 1.0f);
	}

	glActiveTexture(GL_TEXTURE0+ _time_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, _time_width, _time_height, 0, GL_RED, GL_FLOAT, _time_data);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);

	glUniform1i(_time_loc, _time_texture_index);

	// -------------------
	glGenTextures(1, &_index_time_id);

	_index_time_data= new unsigned int[_index_time_width];
	for (unsigned int i=0; i<_index_time_width; ++i) {
		_index_time_data[i]= 0;
	}

	glActiveTexture(GL_TEXTURE0+ _index_time_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, _index_time_width, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, _index_time_data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	glUniform1i(_index_time_loc, _index_time_texture_index);
}


MPEGReaders::~MPEGReaders() {
	delete[] _alpha_data;
	delete[] _time_data;
	delete[] _index_time_data;
}

/*void MPEGReaders::hidden(float * data) {
	for (unsigned int i=0; i<_width* _height* _depth* 4; ++i) {
		data[i]= 0.0f;
	}
}


void MPEGReaders::linear(float ** data, int depth, float z) {
	float f_width= (float)(_width);
	float f_height= (float)(_height);
	for (unsigned int j=0; j<_height; ++j) {
		for (unsigned int i=0; i<_width; ++i) {
			data[depth][(j* _width+ i)* 4+ 0]= (float)(i)/ f_width;
			data[depth][(j* _width+ i)* 4+ 1]= (float)(j)/ f_height;
			data[depth][(j* _width+ i)* 4+ 2]= z;
			data[depth][(j* _width+ i)* 4+ 3]= 0.5f;
		}
	}
}


void MPEGReaders::total_rand(float * data) {
	for (unsigned int j=0; j<_depth; ++j) {
		for (unsigned int i=0; i<_width* _height; ++i) {
			data[(_width* _height* j+ i)* 3+ 0]= rand_float(0.0f, 1.0f);
			data[(_width* _height* j+ i)* 3+ 1]= rand_float(0.0f, 1.0f);
			data[(_width* _height* j+ i)* 3+ 2]= rand_float(0.0f, 1.0f);
		}
	}
}


void MPEGReaders::time_rand(float * data) {
	for (unsigned int j=0; j<_depth; ++j) {
		for (unsigned int i=0; i<_width* _height; ++i) {
			float x= (float)(i % _width)/ (float)(_width);
			float y= (float)(i / _width)/ (float)(_height);
			data[(_width* _height* j+ i)* 3+ 0]= x;
			data[(_width* _height* j+ i)* 3+ 1]= y;
			data[(_width* _height* j+ i)* 3+ 2]= rand_float(0.0f, 1.0f);
		}
	}
}


void MPEGReaders::position_rand(float * data) {
	for (unsigned int j=0; j<_depth; ++j) {
		for (unsigned int i=0; i<_width* _height; ++i) {
			data[(_width* _height* j+ i)* 3+ 0]= rand_float(0.0f, 1.0f);
			data[(_width* _height* j+ i)* 3+ 1]= rand_float(0.0f, 1.0f);
			data[(_width* _height* j+ i)* 3+ 2]= (float)(j)/ (float)(_depth);
		}
	}
}


void MPEGReaders::smooth_rand_time(float * data, int m) {
	for (unsigned int j=0; j<_depth; ++j) {
		float r= rand_float(0.0f, 1.0f);
		int k= rand_int(-m, m);
		for (unsigned int i=0; i<_width* _height; ++i) {
			float x= (float)(i % _width)/ (float)(_width);
			float y= (float)(i / _width)/ (float)(_height);
			
			data[(_width* _height* j+ i)* 3+ 0]= x;
			data[(_width* _height* j+ i)* 3+ 1]= y;
			
			if (j== 0) {
				data[(_width* _height* j+ i)* 3+ 2]= r;
			}
			else {
				data[(_width* _height* j+ i)* 3+ 2]= data[(_width* _height* (j- 1)+ i)* 3+ 2]+ (float)(k)/ (float)(_depth);
			}
		}
	}
	for (unsigned int i=0; i<_width* _height* _depth* 3; ++i) {
		if (data[i]< 0.0f) {
			data[i]= 0.0f;
		}
		if (data[i]> 1.0f) {
			data[i]= 1.0f;
		}
	}
}


void MPEGReaders::mosaic(float * data, int size) {
	unsigned int * t= new unsigned int[size* size];
	for (unsigned int i=0; i<size; ++i) {
		for (unsigned int j=0; j<size; ++j) {
			t[i+ j* size]= rand_int(0, _depth- 1);
		}
	}
	for (unsigned int k=0; k<_depth; ++k) {
		for (unsigned int i=0; i<_width; ++i) {
			for (unsigned int j=0; j<_height; ++j) {
				float x= (float)(i % size)/ (float)(size);
				float y= (float)(j % size)/ (float)(size);
				float z= (float)((t[(i / size)+ (j / size)* size]+ k) % _depth)/ (float)(_depth);
				
				data[(_width* _height* k+ _width* j+ i)* 3+ 0]= x;
				data[(_width* _height* k+ _width* j+ i)* 3+ 1]= y;
				data[(_width* _height* k+ _width* j+ i)* 3+ 2]= z;
			}
		}
	}
	delete[] t;
}*/


void MPEGReaders::prepare2draw() {
	glUniform1i(_alpha_loc, _alpha_texture_index);
	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);

	glUniform1i(_time_loc, _time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _time_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);

	glUniform1i(_index_time_loc, _index_time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _index_time_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
}


/*void MPEGReaders::next(unsigned int reader_idx) {
	_idx+= 1;
	if (_idx>= _depth) {
		_idx= 0;
	}
	for (unsigned int i=0; i<N_MAX_READERS; ++i) {
		_index_indices[i]= (float)(_idx)/ (float)(_depth);
	}*/

	/*_idx[reader_idx]+= 0.01f;
	if (_idx[reader_idx]> 1.0f) {
		_idx[reader_idx]= 0.0f;
	}
	
	linear(reader_idx, _idx[reader_idx]);
}*/


void MPEGReaders::update_alpha(unsigned int depth) {
	for (unsigned int i=0; i<_alpha_width* _alpha_height; ++i) {
		_alpha_data[_alpha_width* _alpha_height* depth+ i]= rand_float(0.0f, 1.0f);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, depth, _alpha_width, _alpha_height, 1, GL_RED, GL_FLOAT, _alpha_data+ _alpha_width* _alpha_height* depth);
}


void MPEGReaders::next_index_time(unsigned int depth) {
	_index_time_data[depth]++;
	if (_index_time_data[depth]>= _time_height) {
		_index_time_data[depth]= 0;
	}
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
	glTexSubImage1D(GL_TEXTURE_1D, 0, depth, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &_index_time_data[depth]);
}

