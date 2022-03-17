
#include <SDL2/SDL.h>

#include <iostream>

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


void init() {
	video_sampler= new VideoSampler("../data/video_sampler_01.json");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		cout << "SDL_Init failed: " << SDL_GetError() << "\n";
		return;
	}

	window= SDL_CreateWindow("video_sampler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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
}


void draw() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		if (video_sampler->_track_samples[idx_track]->_playing) {
			key_type key= video_sampler->_track_samples[idx_track]->_info._key;
			VideoSubSample * sub_sample= video_sampler->_map[key];
			//float amplitude= 1.0f;

			VideoSample * video_sample= sub_sample->_sample;
			
			surf= SDL_CreateRGBSurfaceFrom(video_sample->get_frame(video_sampler->_track_samples[idx_track]->_frame_idx),
				video_sample->_width, video_sample->_height, 24, video_sample->_width* 3, rmask, gmask, bmask, amask);
			tex= SDL_CreateTextureFromSurface(renderer, surf);
			SDL_FreeSurface(surf);

			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			unsigned int alpha= 100;
			SDL_SetTextureAlphaMod(tex, alpha);

			SDL_Rect texture_rect;
			/*texture_rect.x = 0;
			texture_rect.y = 0;
			texture_rect.w = SCREEN_WIDTH;
			texture_rect.h = SCREEN_HEIGHT;*/
			texture_rect.x = sub_sample->_x;
			texture_rect.y = sub_sample->_y;
			texture_rect.w = sub_sample->_w;
			texture_rect.h = sub_sample->_h;

			SDL_RenderCopy(renderer, tex, NULL, &texture_rect);
			SDL_DestroyTexture(tex);

			//cout << video_sampler->_track_samples[idx_track]->_frame_idx << "\n";

			video_sampler->_track_samples[idx_track]->_frame_idx++;
			if (video_sampler->_track_samples[idx_track]->_frame_idx>= video_sample->_n_frames) {
				video_sampler->note_off(idx_track);
			}
		}
	}

	SDL_RenderPresent(renderer);
}


void idle() {
	video_sampler->update();
	draw();
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
	init();
	main_loop();
	clean();
	
	return 0;
}
