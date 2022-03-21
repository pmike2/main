#include <iostream>
#include <fstream>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "json.hpp"

#include "video_sampler.h"

using namespace std;
using json = nlohmann::json;



SUB_SAMPLE_MODE get_sample_mode(string str_mode) {
	if (str_mode== "HOLD") {
		return HOLD;
	}
	if (str_mode== "FROM_START") {
		return FROM_START;
	}
	if (str_mode== "TO_END") {
		return TO_END;
	}
	if (str_mode== "ALL") {
		return ALL;
	}
	cout << "mode " << str_mode << " non supporté\n";
	return ALL;
}


// ------------------------------------------------------------------------------------------
VideoSample::VideoSample() : _n_frames(0), _fps(0), _data(0), _width(0), _height(0), _frame_size(0) {

}


VideoSample::VideoSample(string sample_path) {
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
	ret = avformat_open_input(&ctx_format, sample_path.c_str(), 0, 0);
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


VideoSample::~VideoSample() {

}


void * VideoSample::get_frame(unsigned int frame_idx) {
	return (void *)(_data+ frame_idx* _frame_size* sizeof(unsigned char));
}


// ----------------------------------------------------------------------
VideoSamplePool::VideoSamplePool() {

}


VideoSamplePool::~VideoSamplePool() {
	for (const auto &x : _samples) {
		delete x.second;
	}
}


VideoSample * VideoSamplePool::get_sample(string sample_path) {
	if (!_samples.count(sample_path)) {
		VideoSample * sample= new VideoSample(sample_path);
		_samples[sample_path]= sample;
	}
	return _samples[sample_path];
}


// ----------------------------------------------------------------------
VideoSubSample::VideoSubSample() :
	_sample(0), _t_start(time_type::zero()), _t_end(time_type::zero()), _mode(ALL) 
{

}


VideoSubSample::VideoSubSample(VideoSample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
	_sample(sample), _t_start(t_start), _t_end(t_end), _mode(mode), _x(x), _y(y), _w(w), _h(h)
{

}


VideoSubSample::~VideoSubSample() {
	
}


// ----------------------------------------------------------------------
VideoTrackSample::VideoTrackSample() : _frame_idx(0), _playing(false), _frame_idx_inc(0.0) {
	_info.set_null();
}


VideoTrackSample::~VideoTrackSample() {

}


// ----------------------------------------------------------------------
VideoSampler::VideoSampler() {

}


VideoSampler::VideoSampler(string json_path) {
	_sample_pool= new VideoSamplePool();
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_track_samples[idx_track]= new VideoTrackSample();
	}

	ifstream istr(json_path);
	json js;
	istr >> js;
	
	for (auto & mapping : js.items()) {
		key_type key= (key_type)(mapping.key().c_str()[0]);
		json val= mapping.value();

		string sample_path= "";
		SUB_SAMPLE_MODE mode= ALL;
		time_type t_start= time_type::zero();
		time_type t_end= time_type::zero();
		unsigned int x= 0;
		unsigned int y= 0;
		unsigned int w= 0;
		unsigned int h= 0;
		
		if (val.contains("sample_path")) {
			sample_path= val["sample_path"];
		}
		else {
			cout << "Attribut sample_path manquant !\n";
			continue;
		}
		if (val.contains("mode")) {
			mode= get_sample_mode(val["mode"]);
		}
		if (val.contains("t_start")) {
			t_start= chrono::milliseconds(val["t_start"]);
		}
		if (val.contains("t_end")) {
			t_end= chrono::milliseconds(val["t_end"]);
		}
		if (val.contains("x")) {
			x= val["x"];
		}
		if (val.contains("y")) {
			y= val["y"];
		}
		if (val.contains("w")) {
			w= val["w"];
		}
		if (val.contains("h")) {
			h= val["h"];
		}

		VideoSample * sample= _sample_pool->get_sample(sample_path);
		_map[key]= new VideoSubSample(sample, t_start, t_end, mode, x, y, w, h);
	}

	_debug_path= "../data/debug/video_sampler.txt";
}


VideoSampler::~VideoSampler() {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _track_samples[idx_track];
	}
	for (const auto &x : _map) {
		delete x.second;
	}

	delete _sample_pool;
}


void VideoSampler::note_on(unsigned int idx_track) {
	//cout << "note_on\n";
	_track_samples[idx_track]->_info= _data_current[idx_track];
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_frame_idx_inc= 0.0;
	_track_samples[idx_track]->_playing= true;
}


void VideoSampler::note_off(unsigned int idx_track) {
	//cout << "note_off\n";
	_track_samples[idx_track]->_info.set_null();
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_frame_idx_inc= 0.0;
	_track_samples[idx_track]->_playing= false;
}


VideoSubSample * VideoSampler::get_subsample(key_type key) {
	if (!_map.count(key)) {
		return 0;
	}
	return _map[key];
}
