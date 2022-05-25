#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>

#include "shared_mem.h"

using namespace std;



bool operator== (const sharedata_type& x, const sharedata_type& y) {
	//return ((x._key== y._key) && (x._t_start== y._t_start) && (x._t_end== y._t_end) && (x._amplitude== y._amplitude));
	return ((x._key== y._key) && (x._t_start== y._t_start) && (x._amplitude== y._amplitude));
}


bool operator!= (const sharedata_type& x, const sharedata_type& y) {
	//return ((x._key!= y._key) || (x._t_start!= y._t_start) || (x._t_end!= y._t_end) || (x._amplitude!= y._amplitude));
	return ((x._key!= y._key) || (x._t_start!= y._t_start) || (x._amplitude!= y._amplitude));
}


ostream & operator << (ostream & os, const sharedata_type & e) {
	os << "key= " << e._key << " ; ";
	os << "t_start= " << time_print(e._t_start) << " ; ";
	os << "t_end= " << time_print(e._t_end) << " ; ";
	os << "amplitude= " << e._amplitude;
	return os;
}


sharedata_type& sharedata_type::operator=(const sharedata_type& other) {
	_key= other._key;
	_t_start= other._t_start;
	_t_end= other._t_end;
	_amplitude= other._amplitude;
	return *this;
}


void sharedata_type::set_null() {
	_key= NULL_KEY;
	_t_start= time_type::zero();
	_t_end= time_type::zero();
	_amplitude= NULL_AMPLITUDE;
}


bool sharedata_type::is_null() {
	return _key== NULL_KEY;
}


// ------------------------------------------------------------------------
unsigned int time_ms(time_type t) {
	return chrono::duration_cast<chrono::milliseconds>(t).count();
}


string time_print(time_type t) {
	return to_string(time_ms(t))+ "ms";
}


// ------------------------------------------------------------------
Emitter::Emitter() {
	//https://linuxhint.com/posix-shared-memory-c-programming/

	// create shared memory object; renvoie un file descriptor
	// O_CREAT : crée si n'existe pas ; O_RDWR : read & write
	// O_EXCL permet de générer une erreur si l'objet existe déjà mais c'est pénible d'avoir à supprimer l'objet à la main
	_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_CREAT | O_RDWR, 0600);
	if (_fd< 0) {
		perror("Sequence : ERROR shm_open");
		return;
	}

	// set size
	ftruncate(_fd, DATA_SIZE);

	// map file descriptor into memory
	// PROT_READ | PROT_WRITE : lecture & écriture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

	for (unsigned idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		_data[idx_track].set_null();
	}
}


Emitter::~Emitter() {
	// suppression du mapping
	munmap(_data, DATA_SIZE);

	// fermeture file
	close(_fd);

	// suppression shared memory object
	shm_unlink(SHARED_MEM_OBJ_NAME.c_str());
}


// ------------------------------------------------------------------
Receiver::Receiver() {
	init_data();

	_data_current= new sharedata_type[N_TRACKS];
	for (unsigned idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		_data_current[idx_track].set_null();
	}

	if (DEBUG) {
		for (unsigned int i=0; i<N_DEBUG; ++i) {
			_debug[i]= time_type::zero();
		}
		_debug_start_point= chrono::system_clock::now();
		// _debug_path doit être spécifié dans les classes filles 
		_debug_path= "";
	}
}

Receiver::~Receiver() {
	close_data();
	
	delete[] _data_current;

	if (DEBUG) {
		if (_debug_path.empty()) {
			cout << "_debug_path n'est pas précisé\n";
			return;
		}
		ofstream myfile;
		myfile.open(_debug_path);
		for (unsigned int i=0; i<N_DEBUG; ++i) {
			if (_debug[i]!= time_type::zero()) {
				myfile << time_ms(_debug[i]) << "\n";
			}
		}
		myfile.close();
	}
}


void Receiver::init_data() {
	// create shared memory object; renvoie un file descriptor
	// O_RDONLY : lecture
	// le while sert ici à retenter tant que le shm_open de looper n'a pas été créé
	_fd= -1;
	while (_fd< 0) {
		_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_RDONLY, 0666);
	}

	// map file descriptor into memory
	// PROT_READ : lecture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ, MAP_SHARED, _fd, 0);
}


void Receiver::close_data() {
	// suppression du mapping
	munmap(_data, DATA_SIZE);

	// fermeture file
	close(_fd);
}


void Receiver::update() {
	for (unsigned idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		if (_data_current[idx_track]!= _data[idx_track]) {
			if (!_data_current[idx_track].is_null()) {
				note_off(idx_track);
			}

			_data_current[idx_track]= _data[idx_track];

			if (!_data_current[idx_track].is_null()) {
				note_on(idx_track);
			}
		}
	}
}
