#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include<libavutil/imgutils.h>
}


int fps = 10;
int width = 1024;
int height = 1024;
int bitrate = 2000;
int n_frames= 100;
std::string output_path= "test.mp4";

AVFrame * videoFrame= 0;
AVCodecContext * cctx= 0;
AVStream * stream= 0;
SwsContext * swsCtx= 0;
int frameCounter= 0;
AVFormatContext * ofctx= 0;
const AVOutputFormat * oformat= 0;


static void pushFrame(unsigned char * data) {
	int err;
	
	AVFrame * frame= av_frame_alloc();
	//int buffer_size= av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
	//int frame_size= buffer_size* sizeof(unsigned char);
	//unsigned char * buffer= (unsigned char *)av_malloc(frame_size);

	//int inLinesize[4];
	av_image_fill_arrays(frame->data, frame->linesize, data, AV_PIX_FMT_RGB24, width, height, 32);
	//std::cout << frame->linesize[0] << " ; " << frame->linesize[1] << " ; " << frame->linesize[2] << " ; " << frame->linesize[3] << "\n";


	/*FILE* fp = fopen("tmp.dat", "wb");
	fwrite(frame->data, width* height* 3, 1, fp);
	fclose(fp);*/


	//int inLinesize[1] = { 3 * cctx->width };
	//int inLinesize= 4 * cctx->width;
	
	// From RGB to YUV
	//frame->linesize[0]= 192;
	sws_scale(swsCtx, (const unsigned char * const *)&frame->data, frame->linesize, 0, cctx->height, videoFrame->data, videoFrame->linesize);
	videoFrame->pts = (1.0 / (double)(fps)) * stream->time_base.den * (frameCounter++);
	
	//std::cout << videoFrame->pts << " " << cctx->time_base.num << " " << cctx->time_base.den << " " << frameCounter << std::endl;
	
	if ((err = avcodec_send_frame(cctx, videoFrame)) < 0) {
		std::cout << "Failed to send frame" << err << std::endl;
		return;
	}

	AVPacket * pkt = av_packet_alloc();
	
	if (avcodec_receive_packet(cctx, pkt) == 0) {
		/*static int counter = 0;
		if (counter == 0) {
			FILE* fp = fopen("dump_first_frame1.dat", "wb");
			fwrite(pkt->data, pkt->size, 1, fp);
			fclose(fp);
		}*/
		
		//std::cout << "pkt key: " << (pkt.flags & AV_PKT_FLAG_KEY) << " " << pkt.size << " " << (counter++) << std::endl;
		
		/*unsigned char * size = ((unsigned char *)pkt.data);
		std::cout << "first: " << (int)size[0] << " " << (int)size[1] <<
			" " << (int)size[2] << " " << (int)size[3] << " " << (int)size[4] <<
			" " << (int)size[5] << " " << (int)size[6] << " " << (int)size[7] <<
			std::endl;*/
		
		av_interleaved_write_frame(ofctx, pkt);
		av_packet_unref(pkt);
	}
}


static void finish() {
	//DELAYED FRAMES
	AVPacket * pkt = av_packet_alloc();

	for (;;) {
		avcodec_send_frame(cctx, NULL);
		if (avcodec_receive_packet(cctx, pkt) == 0) {
			av_interleaved_write_frame(ofctx, pkt);
			av_packet_unref(pkt);
		}
		else {
			break;
		}
	}

	av_write_trailer(ofctx);
	if (!(oformat->flags & AVFMT_NOFILE)) {
		int err = avio_close(ofctx->pb);
		if (err < 0) {
			std::cout << "Failed to close file" << err << std::endl;
		}
	}
}


static void free() {
	if (videoFrame) {
		av_frame_free(&videoFrame);
	}
	if (cctx) {
		avcodec_free_context(&cctx);
	}
	if (ofctx) {
		avformat_free_context(ofctx);
	}
	if (swsCtx) {
		sws_freeContext(swsCtx);
	}
}


int main(int argc, char* argv[]) {
	// verbosité
	av_log_set_level(AV_LOG_ERROR);

	oformat = av_guess_format(nullptr, output_path.c_str(), nullptr);
	if (!oformat) {
		std::cout << "can't create output format" << std::endl;
		return -1;
	}

	int err = avformat_alloc_output_context2(&ofctx, oformat, nullptr, output_path.c_str());
	if (err) {
		std::cout << "can't create output context" << std::endl;
		return -1;
	}

	const AVCodec* codec = avcodec_find_encoder(oformat->video_codec);
	if (!codec) {
		std::cout << "can't create codec" << std::endl;
		return -1;
	}

	stream = avformat_new_stream(ofctx, codec);
	if (!stream) {
		std::cout << "can't find format" << std::endl;
		return -1;
	}

	cctx = avcodec_alloc_context3(codec);
	if (!cctx) {
		std::cout << "can't create codec context" << std::endl;
		return -1;
	}

	stream->codecpar->codec_id = oformat->video_codec;
	stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	stream->codecpar->width = width;
	stream->codecpar->height = height;
	stream->codecpar->format = AV_PIX_FMT_YUV420P;
	stream->codecpar->bit_rate = bitrate * 1000;

	avcodec_parameters_to_context(cctx, stream->codecpar);

	cctx->time_base = {1,1};
	cctx->max_b_frames = 2;
	cctx->gop_size = 12;
	cctx->framerate = { fps, 1 };

	if (stream->codecpar->codec_id == AV_CODEC_ID_H264) {
		av_opt_set(cctx, "preset", "ultrafast", 0);
	}
	else if (stream->codecpar->codec_id == AV_CODEC_ID_H265) {
		av_opt_set(cctx, "preset", "ultrafast", 0);
	}

	avcodec_parameters_from_context(stream->codecpar, cctx);

	if ((err = avcodec_open2(cctx, codec, NULL)) < 0) {
		std::cout << "Failed to open codec" << err << std::endl;
		return -1;
	}

	if (!(oformat->flags & AVFMT_NOFILE)) {
		if ((err = avio_open(&ofctx->pb, output_path.c_str(), AVIO_FLAG_WRITE)) < 0) {
			std::cout << "Failed to open file" << err << std::endl;
			return -1;
		}
	}

	if ((err = avformat_write_header(ofctx, NULL)) < 0) {
		std::cout << "Failed to write header" << err << std::endl;
		return -1;
	}

	av_dump_format(ofctx, 0, output_path.c_str(), 1);

	videoFrame = av_frame_alloc();
	videoFrame->format = AV_PIX_FMT_YUV420P;
	videoFrame->width = cctx->width;
	videoFrame->height = cctx->height;
	if ((err = av_frame_get_buffer(videoFrame, 0)) < 0) {
		std::cout << "Failed to allocate picture" << err << std::endl;
		return 1;
	}

	swsCtx= sws_getContext(cctx->width, cctx->height, AV_PIX_FMT_RGB24, cctx->width, cctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);

	unsigned char* frameraw = new unsigned char[width * height * 3];
	for (int i=0; i<width * height; ++i) {
		frameraw[i* 3+ 0]= 255;
		frameraw[i* 3+ 1]= 255;
		frameraw[i* 3+ 2]= 0;
	}

	unsigned char* frameraw2 = new unsigned char[width * height * 3];
	for (int i=0; i<width * height; ++i) {
		frameraw2[i* 3+ 0]= 255;
		frameraw2[i* 3+ 1]= 0;
		frameraw2[i* 3+ 2]= 0;
	}

	int s= av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);

	for (int i=0; i<n_frames; ++i) {
		pushFrame(frameraw);
	}
	for (int i=0; i<n_frames; ++i) {
		pushFrame(frameraw2);
	}

	delete[] frameraw;
	delete[] frameraw2;
	finish();
	free();
	return 0;
}
