#ifndef MPEG_H
#define MPEG_H

#include <string>

extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
}


class MPEGReader {
public:
	MPEGReader();
	MPEGReader(std::string input_path);
	~MPEGReader();
	unsigned char * get_frame(unsigned int frame_idx);


	unsigned int _n_frames;
	unsigned int _fps;
	unsigned char * _data;
	unsigned int _width;
	unsigned int _height;
	unsigned int _frame_size;
};


class MPEGWriter {
public:
	MPEGWriter();
	MPEGWriter(unsigned int width, unsigned int height, unsigned int fps, unsigned int bitrate, std::string output_path);
	~MPEGWriter();
	void push_frame(unsigned char * data);
	void finish();


	unsigned int _width;
	unsigned int _height;
	unsigned int _fps;
	unsigned int _bitrate;
	std::string _output_path;
	
	AVFrame * _frame;
	AVFrame * _frame_rgb;
	AVCodecContext * _codec_context;
	AVStream * _stream;
	SwsContext * _sws_context;
	AVFormatContext * _format_context;
	const AVOutputFormat * _output_format;
	const AVCodec * _codec;
	unsigned int _frame_count;
	//unsigned char * _buffer;
	//unsigned char _buffer[1024*1024*3];
	AVPacket * _pkt;
};

#endif
