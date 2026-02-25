#include "fps_count.h"


FPSCount::FPSCount() {

}


FPSCount::FPSCount(SDL_Window * window) : _window(window) {

}


FPSCount::~FPSCount() {

}


void FPSCount::add_frame() {
	_compt_fps++;
}


void FPSCount::update() {
	_tikfps2 = SDL_GetTicks();
	if (_tikfps2 - _tikfps1 > 1000) {
		char s_fps[256];
		_tikfps1 = SDL_GetTicks();
		_val_fps = _compt_fps;
		_compt_fps = 0;
		sprintf(s_fps, "%d", _val_fps);
		SDL_SetWindowTitle(_window, s_fps);
	}
}
