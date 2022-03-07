#include "looper_sdl.h"

using namespace std;


LooperSDL::LooperSDL() {

}


LooperSDL::~LooperSDL() {
	
}


void LooperSDL::key_down(SDL_Keycode key) {
	//cout << "key down : " << key << "\n";
	if (key== SDLK_SPACE) {
		debug();
		return;
	}
	insert_event(key);
}


void LooperSDL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";
	
}
