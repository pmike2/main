
#include <SDL2/SDL.h>

#include <iostream>
#include <thread>
#include <chrono>

#include "video_sampler.h"

using namespace std;


int SCREEN_WIDTH= 512;
int SCREEN_HEIGHT= 512;


SDL_Window * window= 0;
SDL_Renderer * renderer= 0;
SDL_Surface * surf;
SDL_Texture * tex;
Uint32 rmask, gmask, bmask, amask;
bool done= false;

VideoSampler * video_sampler;
thread thr;
atomic_bool stop_thr= ATOMIC_VAR_INIT(false);
mutex mtx;
chrono::system_clock::time_point saved_tp;
chrono::system_clock::time_point recent_tp;


void update_thread() {
	while (true) {
		if (stop_thr) {
			break;
		}
		//mtx.lock();
		video_sampler->update();
		//mtx.unlock();
	}
}


void init(string json_path) {
	video_sampler= new VideoSampler(json_path);

	SDL_Init(SDL_INIT_EVERYTHING);

	window= SDL_CreateWindow("video_sampler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

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

	//thr= thread(update_thread);

	saved_tp= chrono::system_clock::now();
}


void draw() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		if (video_sampler->_track_samples[idx_track]->_playing) {
			
			if (DEBUG) {
				if (video_sampler->_track_samples[idx_track]->_frame_idx== 0) {
					time_type t= chrono::system_clock::now()- video_sampler->_debug_start_point;
					video_sampler->_debug[video_sampler->_compt_debug++]= t;
					if (video_sampler->_compt_debug>= N_DEBUG) {
						video_sampler->_compt_debug= 0;
					}
				}
			}

			key_type key= video_sampler->_track_samples[idx_track]->_info._key;
			amplitude_type amplitude= video_sampler->_track_samples[idx_track]->_info._amplitude;
			VideoSubSample * sub_sample= video_sampler->get_subsample(key);
			if (!sub_sample) {
				continue;
			}

			VideoSample * video_sample= sub_sample->_sample;

			surf= SDL_CreateRGBSurfaceFrom(video_sample->get_frame(video_sampler->_track_samples[idx_track]->_frame_idx),
				video_sample->_width, video_sample->_height, 24, video_sample->_width* 3, rmask, gmask, bmask, amask);
			tex= SDL_CreateTextureFromSurface(renderer, surf);
			SDL_FreeSurface(surf);

			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			unsigned int alpha= (unsigned int)(255.0f* amplitude);
			SDL_SetTextureAlphaMod(tex, alpha);

			SDL_Rect texture_rect;
			if ((sub_sample->_w== 0) || (sub_sample->_h== 0)) {
				texture_rect.x= 0;
				texture_rect.y= 0;
				texture_rect.w= SCREEN_WIDTH;
				texture_rect.h= SCREEN_HEIGHT;
			}
			else {
				texture_rect.x= sub_sample->_x;
				texture_rect.y= sub_sample->_y;
				texture_rect.w= sub_sample->_w;
				texture_rect.h= sub_sample->_h;
			}

			SDL_RenderCopy(renderer, tex, NULL, &texture_rect);
			SDL_DestroyTexture(tex);

			recent_tp= chrono::system_clock::now();
			unsigned int duration_ms= chrono::duration_cast<chrono::milliseconds>(recent_tp- saved_tp).count();
			saved_tp= recent_tp;

			video_sampler->_track_samples[idx_track]->_frame_idx_inc+= (double)(video_sample->_fps* duration_ms)/ 1000.0;
			unsigned int int_part= (unsigned int)(video_sampler->_track_samples[idx_track]->_frame_idx_inc);
			video_sampler->_track_samples[idx_track]->_frame_idx+= int_part;
			video_sampler->_track_samples[idx_track]->_frame_idx_inc-= int_part;
			if (video_sampler->_track_samples[idx_track]->_frame_idx>= video_sample->_n_frames) {
				video_sampler->note_off(idx_track);
			}

			/*cout << "-------------\n";
			cout << duration_ms << "\n";
			cout << video_sampler->_track_samples[idx_track]->_frame_idx << "\n";
			cout << video_sampler->_track_samples[idx_track]->_frame_idx_inc << "\n";*/
		}
	}

	SDL_RenderPresent(renderer);
}


void idle() {
	//mtx.lock();
	video_sampler->update();
	draw();
	//mtx.unlock();
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
	//stop_thr= true;
	//thr.join();

	delete video_sampler;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main(int argc, char **argv) {
	if (argc!= 2) {
		cout << "donner le chemin d'un json en entrée\n";
		return 1;
	}
	init(string(argv[1]));
	main_loop();
	clean();
	
	return 0;
}
