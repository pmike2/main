#include <iostream>
#include <cstdlib>
#include "RtMidi.h"


// cf https://www.music.mcgill.ca/~gary/rtmidi


RtMidiIn *midiin;


// callback de lecture midi
void mycallback(double deltatime, std::vector<unsigned char> * message, void *userData) {
	unsigned int nBytes = message->size();
	for (unsigned int i=0; i<nBytes; i++) {
		std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
	}
	if (nBytes > 0) {
		std::cout << "stamp = " << deltatime << std::endl;
	}
}


void cleanup() {
	delete midiin;
}


int main() {
	midiin= new RtMidiIn();
	
	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if ( nPorts == 0 ) {
		std::cout << "No ports available!\n";
		cleanup();
		return 1;
	}
	
	// on prend le 1er
	midiin->openPort(0);
	
	// Set our callback function.  This should be done immediately after
	// opening the port to avoid having incoming messages written to the
	// queue.
	midiin->setCallback(&mycallback);
	
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(false, false, false);
	
	std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
	char input;
	std::cin.get(input);

	cleanup();

	return 0;
}
