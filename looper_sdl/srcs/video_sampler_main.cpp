
#include <SDL2/SDL.h>

#include <iostream>

using namespace std;


int SCREEN_WIDTH= 512;
int SCREEN_HEIGHT= 512;


SDL_Window * window= 0;
SDL_Renderer * renderer= 0;
SDL_Surface * surf;
SDL_Texture * tex;
Uint32 rmask, gmask, bmask, amask;
bool done= false;


Sampler * video_sampler;


int init(const char * file_in) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		cout << "SDL_Init failed: " << SDL_GetError() << "\n";
		return 1;
	}

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

	video_sampler= new Sampler("../data/video_sampler_01.json");
}


void idle() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	current_idx++;
	if (current_idx>= n_frames) {
		current_idx= 0;
	}

	int depth = 24;
	int pitch = 3* width;
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
	//SDL_Delay(100);
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
	// fichier mp4 en entrée
	const char * file_in= argv[1];
	
	ret= init(file_in);
	main_loop();
	clean();
	
	return 0;
}
