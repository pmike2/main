#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "mpeg.h"


using namespace std;



MPEGReader::MPEGReader() : _n_frames(0), _fps(0), _data(0), _width(0), _height(0), _frame_size(0) {

}


MPEGReader::MPEGReader(string input_path) {
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
	ret = avformat_open_input(&ctx_format, input_path.c_str(), 0, 0);
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


MPEGReader::~MPEGReader() {
	av_free(_data);
}


unsigned char * MPEGReader::get_frame(unsigned int frame_idx) {
	return (unsigned char *)(_data+ frame_idx* _frame_size* sizeof(unsigned char));
}


// ----------------------------------------------------------------------------------------------------------------
MPEGWriter::MPEGWriter() {

}


MPEGWriter::MPEGWriter(unsigned int width, unsigned int height, unsigned int fps, unsigned int bitrate, string output_path) :
	_width(width), _height(height), _fps(fps), _bitrate(bitrate), _output_path(output_path), _frame_count(0)
{	
	int err;

	string rm_cmd= "rm "+ _output_path+ "";
	system(rm_cmd.c_str());

	// verbosité
	av_log_set_level(AV_LOG_ERROR);

	_output_format= av_guess_format(nullptr, _output_path.c_str(), nullptr);
	if (!_output_format) {
		cout << "can't create output format\n";
		return;
	}

	err= avformat_alloc_output_context2(&_format_context, _output_format, nullptr, output_path.c_str());
	if (err) {
		cout << "can't create output context\n";
		return;
	}

	_codec= avcodec_find_encoder(_output_format->video_codec);
	if (!_codec) {
		cout << "can't create codec\n";
		return;
	}

	_stream= avformat_new_stream(_format_context, _codec);
	if (!_stream) {
		cout << "can't find format\n";
		return;
	}

	_codec_context= avcodec_alloc_context3(_codec);
	if (!_codec_context) {
		cout << "can't create codec context\n";
		return;
	}

	_stream->codecpar->codec_id= _output_format->video_codec;
	_stream->codecpar->codec_type= AVMEDIA_TYPE_VIDEO;
	_stream->codecpar->width= _width;
	_stream->codecpar->height= _height;
	_stream->codecpar->format= AV_PIX_FMT_YUV420P;
	_stream->codecpar->bit_rate= _bitrate* 1000;

	avcodec_parameters_to_context(_codec_context, _stream->codecpar);

	_codec_context->time_base= {1,1};
	_codec_context->max_b_frames= 2;
	_codec_context->gop_size= 12; // ?
	_codec_context->framerate= { (int)(_fps), 1 };

	if (_stream->codecpar->codec_id== AV_CODEC_ID_H264) {
		av_opt_set(_codec_context, "preset", "ultrafast", 0);
	}
	else if (_stream->codecpar->codec_id == AV_CODEC_ID_H265) {
		av_opt_set(_codec_context, "preset", "ultrafast", 0);
	}

	avcodec_parameters_from_context(_stream->codecpar, _codec_context);

	if ((err= avcodec_open2(_codec_context, _codec, NULL))< 0) {
		cout << "Failed to open codec" << err << "\n";
		return;
	}

	if (!(_output_format->flags & AVFMT_NOFILE)) {
		if ((err= avio_open(&_format_context->pb, _output_path.c_str(), AVIO_FLAG_WRITE))< 0) {
			cout << "Failed to open file" << err << "\n";
			return;
		}
	}

	if ((err= avformat_write_header(_format_context, NULL))< 0) {
		cout << "Failed to write header" << err << "\n";
		return;
	}

	//av_dump_format(_format_context, 0, _output_path.c_str(), 1);

	_frame= av_frame_alloc();
	_frame->format= AV_PIX_FMT_YUV420P;
	_frame->width= _width;
	_frame->height= _height;
	if ((err= av_frame_get_buffer(_frame, 0))< 0) {
		cout << "Failed to allocate picture" << err << "\n";
		return;
	}

	_filtered_frame= av_frame_alloc();

	_frame_rgb= av_frame_alloc();
	_frame_rgb->format= AV_PIX_FMT_RGB24;
	_frame_rgb->width= _width;
	_frame_rgb->height= _height;

	_sws_context= sws_getContext(_width, _height, AV_PIX_FMT_RGB24, _width, _height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);

	_pkt= av_packet_alloc();


	_filter_vflip= avfilter_get_by_name("vflip");
	_filter_buffer= avfilter_get_by_name("buffer");
	_filter_buffer_sink= avfilter_get_by_name("buffersink");
	_filter_out= avfilter_inout_alloc();
	_filter_in= avfilter_inout_alloc();
	_filter_graph = avfilter_graph_alloc();

	char args[512];
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		_width, _height, _codec_context->pix_fmt,
		_stream->time_base.num, _stream->time_base.den,
		_codec_context->sample_aspect_ratio.num, _codec_context->sample_aspect_ratio.den);
	
	err= avfilter_graph_create_filter(&_filter_context_buffer, _filter_buffer, "in", args, NULL, _filter_graph);
	if (err< 0) {
		cout << "Erreur avfilter_graph_create_filter buffer" << err << "\n";
		return;
	}

	err= avfilter_graph_create_filter(&_filter_context_buffer_sink, _filter_buffer_sink, "out", NULL, NULL, _filter_graph);
	if (err< 0) {
		cout << "Erreur avfilter_graph_create_filter buffer_sink" << err << "\n";
		return;
	}

	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };
	err= av_opt_set_int_list(_filter_context_buffer_sink, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	if (err< 0) {
		cout << "Erreur av_opt_set_int_list" << err << "\n";
		return;
	}

	_filter_out->name= av_strdup("in");
	_filter_out->filter_ctx= _filter_context_buffer;
	_filter_out->pad_idx= 0;
	_filter_out->next= NULL;
 
	_filter_in->name= av_strdup("out");
	_filter_in->filter_ctx= _filter_context_buffer_sink;
	_filter_in->pad_idx= 0;
	_filter_in->next= NULL;

	const char * filters_descr= "scale=78:24,transpose=cclock";
	err= avfilter_graph_parse_ptr(_filter_graph, filters_descr, &_filter_in, &_filter_out, NULL);
	if (err< 0) {
		cout << "Erreur avfilter_graph_parse_ptr\n";
		return;
	}
 
	err= avfilter_graph_config(_filter_graph, NULL);
	if (err< 0) {
		cout << "Erreur avfilter_graph_config\n";
		return;
	}

	avfilter_inout_free(&_filter_in);
	avfilter_inout_free(&_filter_out);
 }


MPEGWriter::~MPEGWriter() {
	if (_frame) {
		av_frame_free(&_frame);
	}
	if (_filtered_frame) {
		av_frame_free(&_filtered_frame);
	}
	if (_codec_context) {
		avcodec_free_context(&_codec_context);
	}
	if (_format_context) {
		avformat_free_context(_format_context);
	}
	if (_sws_context) {
		sws_freeContext(_sws_context);
	}
	av_packet_free(&_pkt);
}


void MPEGWriter::push_frame(unsigned char * data) {
	int err;

	err= av_frame_make_writable(_frame);
	if (err< 0) {
		cout << "Erreur av_frame_make_writable\n";
		return;
	}

	//int buffer_size= av_image_get_buffer_size(AV_PIX_FMT_RGB24, _width, _height, 1);
	//cout << buffer_size << "\n";
	//unsigned char * buffer= (unsigned char *)av_malloc(buffer_size);
	//memcpy(buffer, data, buffer_size);
	//av_image_alloc(_frame_rgb->data, _frame_rgb->linesize, _width, _height, AV_PIX_FMT_RGB24, 32);
	//av_image_fill_arrays(_frame_rgb->data, _frame_rgb->linesize, data, AV_PIX_FMT_RGB24, _width, _height, 32);
	av_frame_get_buffer(_frame_rgb, 0);
	_frame_rgb->data[0]= data;
	_frame_rgb->linesize[0]= (int)(_width)* 3;
	//unsigned char buffer[_width* _height* 3];
	//memcpy(buffer, data, _width* _height* 3);
	//av_freep(_frame_rgb->data[0]);

	//cout << &data << "\n";

	// From RGB to YUV
	//cout << "BEFORE sws_scale\n";
	err= sws_scale(_sws_context, (const unsigned char * const *)&_frame_rgb->data, _frame_rgb->linesize, 0, _height, _frame->data, _frame->linesize);
	//cout << "AFTER sws_scale\n";
	//int linesize[1]= {(int)(_width)* 3};
	//err= sws_scale(_sws_context, (const unsigned char * const *)&data, linesize, 0, _height, _frame->data, _frame->linesize);

	if (err<= 0) {
		cout << "Erreur sws_scale\n";
		return;
	}
	_frame->pts= (1.0/ (double)(_fps))* _stream->time_base.den* (_frame_count++);
	//cout << _frame->pts << "\n";

	err= av_buffersrc_add_frame_flags(_filter_context_buffer, _frame, AV_BUFFERSRC_FLAG_KEEP_REF);
	if (err< 0) {
		cout << "Failed to av_buffersrc_add_frame_flags frame : " << err << "\n";
		return;
	}

	err= av_buffersink_get_frame(_filter_context_buffer_sink, _filtered_frame);
	if (err< 0) {
		cout << "Failed to av_buffersink_get_frame frame : " << err << "\n";
		return;
	}

	if ((err= avcodec_send_frame(_codec_context, _filtered_frame))< 0) {
		cout << "Failed to send frame : " << err << "\n";
		return;
	}

	//AVPacket * pkt= av_packet_alloc();
	if (avcodec_receive_packet(_codec_context, _pkt)== 0) {
		/*int counter= 0;
		if (counter == 0) {
			FILE* fp = fopen("dump_first_frame1.dat", "wb");
			fwrite(pkt->data, pkt->size, 1, fp);
			fclose(fp);
		}*/
		
		//cout << "pkt key: " << (pkt->flags & AV_PKT_FLAG_KEY) << " " << pkt->size << " " << (_frame_count++) << endl;
		
		/*unsigned char * size = ((unsigned char *)pkt->data);
		cout << "first: " << (int)size[0] << " " << (int)size[1] <<
			" " << (int)size[2] << " " << (int)size[3] << " " << (int)size[4] <<
			" " << (int)size[5] << " " << (int)size[6] << " " << (int)size[7] <<
			endl;*/
		
		av_interleaved_write_frame(_format_context, _pkt);
		av_packet_unref(_pkt);
	}
//	av_packet_free(&pkt);

	//av_freep(&_frame_rgb->data[0]);
	//av_free(buffer);
	//delete[] buffer;

	av_frame_unref(_frame);
	av_frame_unref(_filtered_frame);
}


void MPEGWriter::finish() {
	// delayed frames
	//AVPacket * pkt= av_packet_alloc();

	while (true) {
		avcodec_send_frame(_codec_context, NULL);
		if (avcodec_receive_packet(_codec_context, _pkt)== 0) {
			av_interleaved_write_frame(_format_context, _pkt);
			av_packet_unref(_pkt);
		}
		else {
			break;
		}
	}
	//av_packet_free(&pkt);

	av_write_trailer(_format_context);
	if (!(_output_format->flags & AVFMT_NOFILE)) {
		int err= avio_close(_format_context->pb);
		if (err< 0) {
			cout << "Failed to close file" << err << "\n";
		}
	}
}
