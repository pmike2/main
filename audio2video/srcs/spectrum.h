#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <map>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "portaudio.h"

#include <fftw3.h>

#include "sndfile.h"

#include "json.hpp"

#include "utile.h"
#include "constantes.h"
#include "thread.h"
#include "repere.h"
#include "objfile.h"
#include "input_state.h"


// renvoie la taille d'un bloc regroupant des fréquences parmi SAMPLES_PER_BUFFER
unsigned int block_width(unsigned int idx_block);


// ------------------------------------------------------------------------------------------------------------
// événement : à _time sera joué _idx_sample qui aura la signature _idx_signature
struct AudioEvent {
	double _time;
	long _idx_sample;
	int _idx_signature;
};


// bloc regroupant une partie du spectre audio après fft
class BlockFFT {
public:
	BlockFFT();
	BlockFFT(unsigned int width);
	~BlockFFT();


	float _instant_energy; // somme des carrés des coeffs fft pour la partie du spectre concernée
	float _history_energy_average; // moyenne de l'historique
	float _cmp_ratio; // facteur multiplicatif pour test déclenchement événement
	float _variance_tresh; // seuil variance pour test déclenchement événement
	float _energy_variance; // variance de l'énergie
	bool _is_triggered; // déclenchement
	int _width; // nombre de coeffs fft regroupés dans ce bloc
	int _idx_signature; // le 1er block d'une bande aura cette valeur fixée à la signature si déclenchement
};


// Signature d'un son triggé pour qu'à un type de son soit associé un objet Connexion
class SpectrumSignature {
public:
	SpectrumSignature();
	SpectrumSignature(unsigned int n_blocks_fft, unsigned int n_blocks_record);
	~SpectrumSignature();
	float mass_center(); // barycentre des fréquences pour tri


	float * _values; // les BlockFFT._instant_energy sur _n_blocks_fft * _n_blocks_record
	unsigned int _n_blocks_fft; // nombre de blocs sur une bande
	unsigned int _n_blocks_record; // nombre de bandes
	bool _active; // actif ou non
};

// permet de faire le tri des SpectrumSignature ; on veut en effet qu'un son plus grave ait un idx_signature plus faible
bool cmp_signatures(SpectrumSignature * ss1, SpectrumSignature * ss2);


// audio
class Audio {
public:
	Audio();
	~Audio();
	void push_event(double t); // callback écrit dans _audio_event_queue
	void update_current_sample(double t); // main lit dans _audio_event_queue et écrit dans _signatures_event_queue
	void record2file(std::string record_path);
	void loadfromfile(std::string load_path);
	void compute_spectrum(long sample);
	void set_n_signatures(unsigned int n_signatures); // suppression et recréation des _signatures
	void reinit_signatures(); // passage à false des _signatures.active
	int get_idx_signature(unsigned int idx_record); // renvoie l'indice de la signature la plus proche
	void sort_signatures(); // tri des signatures
	float event_length(long idx_sample); // renvoie la taille en secondes d'un événement triggé (release)
	void set_block_idx_signature(unsigned int idx_record, int idx_signature); // écrit l'idx signature d'un block
	int get_block_idx_signature(unsigned int idx_record); // lit l'idx signature d'un block


	long _n_samples; // nombre de samples
	float * _amplitudes; // samples
	long _left_selection;
	long _right_selection;
	long _current_sample; // sample en cours de lecture / enregistrement
	long _last_sample; // sert à gérer les intervalles de samples traités dans le callback
	long _current_sample_callback; // sample en cours de traitement dans le callback
	BlockFFT ** _blocks; // _n_blocks_fft * _n_blocks_record blocks
	unsigned int _n_blocks_fft; // nombre de blocs sur une bande
	unsigned int _n_blocks_record; // nombre de bandes
	SafeQueue<AudioEvent> _audio_event_queue; // callback écrit dans cette queue et update_current_sample y lit
	float _playback_amplitude; // volume playback
	std::vector<SpectrumSignature *> _signatures; // signatures
	SafeQueue<AudioEvent> _signatures_event_queue; // update_current_sample y écrit et visu_art.anim() y lit
	PaTime _last_trig_time; // sert à voir si on peut redéclencher un son
	AudioMode _mode;
};


// ------------------------------------------------------------------------------------------------------------
// visu onde sonore
class VisuWave {
public:
	VisuWave();
	VisuWave(GLuint prog_draw_2d, Audio * audio);
	~VisuWave();
	void draw();
	void update_data();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	float * _data;
	unsigned int _n_vertices;
	float _camera2clip[16];

	Audio * _audio;
	long _sample_center;
	long _sample_width;

	bool _mouse_down;
};


// partie GL de VisuSpectrum
class GLSpectrum {
public:
	GLSpectrum();
	GLSpectrum(GLuint prog_draw_3d, Audio * audio);
	~GLSpectrum();
	void draw();
	void anim(glm::mat4 world2camera, glm::mat4 camera2clip);
	void update_data();


	float _width;
	float _height;
	float _width_step;
	float _height_step;
	
	unsigned int _n_faces;
	float * _data;
	unsigned int * _faces;
	GLuint _buffers[2];
	
	glm::mat4 _model2camera;
	glm::mat4 _model2clip;
	glm::mat4 _model2world;
	glm::mat3 _normal;
	glm::vec3 _ambient;
	float _shininess;
	float _alpha;

	GLint _model2clip_loc, _model2camera_loc, _normal_mat_loc;
	GLint _position_loc, _normal_loc;
	GLint _ambient_color_loc, _shininess_loc, _diffuse_color_loc;
	GLint _alpha_loc;
	GLuint _prog_draw;

	Audio * _audio;
};


// visu du spectre
class VisuSpectrum {
public:
	VisuSpectrum();
	VisuSpectrum(GLuint prog_draw_3d, GLuint prog_repere, GLuint prog_select, Audio * audio);
	~VisuSpectrum();
	void draw();
	void anim();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLSpectrum * _gl_spectrum;
	ViewSystem * _view_system;
	Audio * _audio;
	bool _mouse_down;
};


// visu simu
class VisuSimu {
public:
	VisuSimu();
	VisuSimu(GLuint prog_draw_2d, Audio * audio);
	~VisuSimu();
	void draw();
	void update_data();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	unsigned int _n_blocks;
	float * _data;
	float _camera2clip[16];
	Audio * _audio;
	bool _mouse_down;
};


// ------------------------------------------------------------------------------------------------------------
// enveloppe attack-release
struct Envelope {
	Envelope();
	void reinit();
	float get_value();
	void anim(PaTime current_time);
	void trig(PaTime current_time);
	void set_static_release(); // en mode playback le release est dynamique, calculé à partir de l'énergie des blocks triggés ; en mode record release est statique ;
	void randomize(float min_rest_val, float max_rest_val, float min_stimulated_val, float max_stimulated_val, float min_attack, float max_attack, float min_release, float max_release);
	void load(nlohmann::json js);
	void load(std::string ch_json);
	nlohmann::json get_json();
	void save(std::string ch_json);
	void print();


	bool _active;
	float _position;
	float _rest_value; // valeur au repos
	float _stimulated_value; // valeur d'excitation max
	float _attack;
	float _release;
	float _static_release;
	float _attack_power; // 1 -> linéaire ; < 1 -> log ; > 1 -> exp
	float _release_power;
	PaTime _trig_time; // temps ou trig a été appelé
};


// ensemble d'enveloppes susceptibles d'affecter diverses propriétés de model_objs
class MorphingObj {
public:
	MorphingObj();
	~MorphingObj();
	void reinit();
	void anim(PaTime current_time);
	void trig(PaTime current_time);
	void set_release(float release);
	void set_static_release();
	void randomize();
	void compute_mat();
	void load(nlohmann::json js);
	void load(std::string ch_json);
	nlohmann::json get_json();
	void save(std::string ch_json);
	void print();

	std::map<std::string, Envelope *> _envelopes;
	glm::mat4 _mat;
};


// matrice reliant les morphing_objs aux model_objs
class Connexion {
public:
	Connexion();
	Connexion(unsigned int width, unsigned int height);
	~Connexion();
	void reinit();
	bool get(unsigned int x, unsigned int y);
	void set(unsigned int x, unsigned int y, bool b);
	void randomize(unsigned int chance);
	void load(nlohmann::json js);
	void load(std::string ch_json);
	nlohmann::json get_json();
	void save(std::string ch_json);
	void print();


	unsigned int _width;
	unsigned int _height;
	bool * _values;
};


// visu principale
class VisuArt {
public:
	VisuArt();
	VisuArt(GLuint prog_draw_3d, GLuint prog_repere, GLuint prog_select, Audio * audio);
	~VisuArt();
	void reinit();
	void draw();
	void anim(PaTime current_time, AudioMode audio_mode);
	void randomize();
	void load(std::string ch_json);
	nlohmann::json get_json();
	void save(std::string ch_json);
	void print();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);


	std::vector<StaticModel *> _static_models;
	std::vector<StaticInstance *> _static_instances;
	std::vector<MorphingObj *> _morphing_objs;
	std::vector<Connexion *> _connexions;
	Repere * _repere;
	ViewSystem * _view_system;
	Audio * _audio;
	int _last_idx_signature; // pour debug affichage de la dernière signature utilisée dans main.draw()
	bool _mouse_down;
	bool _fullscreen;
};

#endif
