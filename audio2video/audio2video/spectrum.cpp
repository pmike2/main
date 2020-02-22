
#include "spectrum.h"

using namespace std;
using json = nlohmann::json;


unsigned int block_width(unsigned int idx_block) {
	// calcul de 2 coeffs utilisés par _width, de sorte que le 1er bloc (le + basse fréquence) regroupe N_COEFFS_IN_FIRST_BLOCK coeffs fft et que la somme des 
	// _width s'approche sans dépasser (!) FFT_OUTPUT_SIZE
	float a, b;
	a= ((float)(FFT_OUTPUT_SIZE)- (float)(N_COEFFS_IN_FIRST_BLOCK)* (float)(N_BLOCKS_FFT))/ ((float)(N_BLOCKS_FFT)* ((float)(N_BLOCKS_FFT)- 1.0f)* 0.5f);
	b= N_COEFFS_IN_FIRST_BLOCK- a;

	return (unsigned int)(a* ((float)(idx_block)+ 1.0f)+ b);
}


bool cmp_signatures(SpectrumSignature * ss1, SpectrumSignature * ss2) {
	return ss1->mass_center() < ss2->mass_center();
}


// ------------------------------------------------------------------------------------------------------------------------
BlockFFT::BlockFFT() {

}


BlockFFT::BlockFFT(unsigned int width) : _instant_energy(0.0f), _history_energy_average(0.0f),
	_cmp_ratio(CMP_RATIO), _variance_tresh(VARIANCE_TRESH), _energy_variance(0.0f), _is_triggered(false), _width(width), _idx_signature(-1) {
}


BlockFFT::~BlockFFT() {
}


// ------------------------------------------------------------------------------------------------------------------------
SpectrumSignature::SpectrumSignature() {
	
}

SpectrumSignature::SpectrumSignature(unsigned int n_blocks_fft, unsigned int n_blocks_record) : _n_blocks_fft(n_blocks_fft), _n_blocks_record(n_blocks_record) {
	_values= new float[_n_blocks_fft* _n_blocks_record];
	for (unsigned int i=0; i<_n_blocks_fft* _n_blocks_record; ++i) {
		_values[i]= 0.0f;
	}
	_active= false;
}


SpectrumSignature::~SpectrumSignature() {
	delete[] _values;
}


float SpectrumSignature::mass_center() {
	// on calcule la somme pondérée des fréquences
	unsigned int idx_fft_current= 0;
	float mc= 0.0f;
	for (unsigned int idx_block=0; idx_block<_n_blocks_fft; ++idx_block) {
		unsigned int width= block_width(idx_block);
		unsigned int idx_fft_min= idx_fft_current;
		unsigned int idx_fft_max= idx_fft_current+ width;
		float freq_min= (float)(idx_fft_min* SAMPLE_RATE)/ float(FFT_INPUT_SIZE);
		float freq_max= (float)(idx_fft_max* SAMPLE_RATE)/ float(FFT_INPUT_SIZE);
		float freq_moy= (freq_min+ freq_max)* 0.5f;
		for (unsigned int idx_record=0; idx_record<_n_blocks_record; ++idx_record) {
			mc+= _values[idx_record* _n_blocks_fft+ idx_block]* freq_moy;
		}
		idx_fft_current+= width;
	}
	mc/= _n_blocks_fft* _n_blocks_record;
	return mc;
}


// ------------------------------------------------------------------------------------------------------------------------
Audio::Audio() :
	_current_sample(0), _current_sample_callback(0), _n_samples(N_SAMPLES_RECORD), _left_selection(0), _right_selection(N_SAMPLES_RECORD),
	_n_blocks_fft(N_BLOCKS_FFT), _n_blocks_record(N_SAMPLES_RECORD/ SAMPLES_PER_BUFFER), _playback_amplitude(INIT_PLAYBACK_AMPLITUDE), _last_trig_time(0),
	_mode(AUDIO_RECORD)
{
	_amplitudes= new float[_n_samples* 2]; // stereo
	for (unsigned int i=0; i<_n_samples* 2; ++i)
		_amplitudes[i]= 0.0f;
	
	_blocks= new BlockFFT*[_n_blocks_fft* _n_blocks_record];
	for (unsigned int idx_record=0; idx_record<_n_blocks_record; ++idx_record)
		for (unsigned int idx_block=0; idx_block<_n_blocks_fft; ++idx_block) {
			unsigned int width= block_width(idx_block);

			// les blocks n'ont pas tous la meme taille ; cf http://archive.gamedev.net/archive/reference/programming/features/beatdetection/index.html
			//if (idx_record== 0) { cout << idx_block << " : " << width << endl; }
			_blocks[idx_record* _n_blocks_fft+ idx_block]= new BlockFFT(width);
		}
}


Audio::~Audio() {
	delete[] _amplitudes;
	delete[] _blocks;
	for (unsigned int i=0; i<_signatures.size(); ++i) {
		delete _signatures[i];
	}
}


void Audio::push_event(double t) {
	// _idx_signature à -1 pour l'instant
	AudioEvent audio_event= {t, _current_sample_callback, -1};
	_audio_event_queue.push(audio_event);
}


void Audio::update_current_sample(double t) {
	AudioEvent audio_event;
	_last_sample= _current_sample;
	// on met à jour _current_sample avec celui le + proche du présent
	while (_audio_event_queue.next(audio_event)) {
		_current_sample= audio_event._idx_sample;
		if (audio_event._time> t) {
			break;
		}
	}

	// on cherche si entre _last_sample et _current_sample il y a eu un trig
	if (_signatures.size()> 0) {
		bool found= false;
		for (unsigned int idx_time=_last_sample/ SAMPLES_PER_BUFFER; idx_time<_current_sample/ SAMPLES_PER_BUFFER; ++idx_time) {
			for (unsigned int idx_freq=0; idx_freq<_n_blocks_fft; ++idx_freq) {
				// on ignore si trig a eu lieu trop récemment
				if ((_blocks[idx_time* _n_blocks_fft+ idx_freq]->_is_triggered) && (t- _last_trig_time> TRIG_MIN_DIST)) {
					_last_trig_time= t;
					// calcul signature et stockage pour réutilisation dans event_length()
					audio_event._idx_signature= get_idx_signature(idx_time);
					set_block_idx_signature(idx_time, audio_event._idx_signature);

					//cout << idx_time << ":" << idx_signature << endl;
					_signatures_event_queue.push(audio_event);
					found= true;
					break;
				}
			}
			// ici break, donc l'utilisation d'une queue n'est pas forcément justifiée ; à revoir peut-être
			if (found)
				break;
		}
	}
}


void Audio::record2file(string record_path) {
	SF_INFO sfinfo;
	sfinfo.channels= 2;
	sfinfo.samplerate= SAMPLE_RATE;
	sfinfo.format= SF_FORMAT_WAV | SF_FORMAT_FLOAT;
	SNDFILE* record= sf_open(record_path.c_str(), SFM_WRITE, &sfinfo);
	sf_write_float(record, _amplitudes, _n_samples* 2);
	sf_close(record);
}


void Audio::loadfromfile(std::string load_path) {
	SF_INFO info;
	SNDFILE* file= sf_open(load_path.c_str(), SFM_READ, &info);
	if (!file) {
		cerr << "sf_open erreur" << endl;
		return;
	}

	float buff[info.channels* info.frames];
	sf_read_float(file, buff, info.channels* info.frames);
	sf_close(file);

	for (long i=0; i<_n_samples; ++i) {
		if (i< info.frames) {
			if (info.channels== 1) {
				_amplitudes[2* i+ 0]= buff[i];
				_amplitudes[2* i+ 1]= buff[i];
			}
			else {
				_amplitudes[2* i+ 0]= buff[2* i+ 0];
				_amplitudes[2* i+ 1]= buff[2* i+ 1];
			}
		}
		else {
			_amplitudes[2* i+ 0]= 0.0f;
			_amplitudes[2* i+ 1]= 0.0f;
		}
	}

	for (long i=0; i<_n_blocks_record; ++i) {
		compute_spectrum(i* SAMPLES_PER_BUFFER);
	}
}


void Audio::compute_spectrum(long sample) {
	double in_fft[FFT_INPUT_SIZE];
	double fft_energies[FFT_OUTPUT_SIZE];
	fftw_complex *out_fft;
	fftw_plan p_fft;

	// calcul de la fft
	out_fft= (fftw_complex *) fftw_malloc(sizeof(fftw_complex)* FFT_OUTPUT_SIZE);
	p_fft= fftw_plan_dft_r2c_1d(FFT_INPUT_SIZE, in_fft, out_fft, FFTW_ESTIMATE);
	
	// fenetrage pour gérer effets de bord FFT
	for (unsigned int i=0; i<FFT_INPUT_SIZE; i++) {
		double hanning= 0.5* (1.0- cos(2.0* M_PI* (double)(i)/ (double)(FFT_INPUT_SIZE- 1)));
		// * sqrt(2)/ 2 pour conserver une energie constante
		double mix= (double)(_amplitudes[2* (sample+ i)+ 0]+ _amplitudes[2* (sample+ i)+ 1])* 0.71;
		in_fft[i]= mix* hanning;
	}

	fftw_execute(p_fft);

	// calcul des énergies a partir des coeffs fft + passage en décibels ; facteur multiplicatif normalement = 20 ?
	for (unsigned int i=0; i<FFT_OUTPUT_SIZE; i++) {
		fft_energies[i]= 20.0* log(out_fft[i][0]* out_fft[i][0]+ out_fft[i][1]* out_fft[i][1])/ log(10.0);
		if (fft_energies[i]< 0.0)
			fft_energies[i]= 0.0;
		//fft_energies[i]= 10.0* sqrt(out_fft[i][0]* out_fft[i][0]+ out_fft[i][1]* out_fft[i][1]);
	}

	// nettoyage fft
	fftw_destroy_plan(p_fft);
	fftw_free(out_fft);

	unsigned int idx_fft= 0;
	for (unsigned int i=0; i<N_BLOCKS_FFT; ++i) {
		long idx_record= sample/ SAMPLES_PER_BUFFER;
		BlockFFT* block_fft= _blocks[idx_record* N_BLOCKS_FFT+ i];

		// calcul de _instant_energy
		block_fft->_instant_energy= 0.0f;
		for (unsigned int j=idx_fft; j<idx_fft+ block_fft->_width; ++j)
			block_fft->_instant_energy+= (float)(fft_energies[j]);
		idx_fft+= block_fft->_width;
		
		// calcul de _history_energy_average
		block_fft->_history_energy_average= 0.0f;
		for (unsigned int j=1; j<=HISTORY_SIZE; ++j) {
			long idx_history= idx_record- j;
			if (idx_history< 0)
				idx_history+= _n_blocks_record;
			block_fft->_history_energy_average+= _blocks[idx_history* _n_blocks_fft+ i]->_instant_energy;
		}
		block_fft->_history_energy_average/= HISTORY_SIZE;

		// calcul de _energy_variance
		block_fft->_energy_variance= 0.0f;
		for (unsigned int j=0; j<=HISTORY_SIZE; ++j) {
			long idx_history= idx_record- j;
			if (idx_history< 0)
				idx_history+= _n_blocks_record;
			block_fft->_energy_variance+= pow(_blocks[idx_history* _n_blocks_fft+ i]->_instant_energy- block_fft->_history_energy_average, 2);
		}
		block_fft->_energy_variance/= HISTORY_SIZE;

		//cout << block_fft->_instant_energy << ";" << block_fft->_cmp_ratio* block_fft->_history_energy_average << ";" << block_fft->_energy_variance << ";" << block_fft->_variance_tresh << endl;

		// trig si l'énergie courante est significativement plus grande que l'historique et si grande variance
		if ((block_fft->_instant_energy> block_fft->_cmp_ratio* block_fft->_history_energy_average) && (block_fft->_energy_variance> block_fft->_variance_tresh)) {
			block_fft->_is_triggered= true;
		}
		else {
			block_fft->_is_triggered= false;
		}
	}
}


void Audio::set_n_signatures(unsigned int n_signatures) {
	for (unsigned int idx_signature=0; idx_signature<_signatures.size(); ++idx_signature) {
		delete _signatures[idx_signature];
	}
	_signatures.clear();

	unsigned int n_blocks_record_signature= SIGNATURE_TIME* (float)(SAMPLE_RATE)/ (float)(SAMPLES_PER_BUFFER);
	for (unsigned int idx_signature=0; idx_signature<n_signatures; ++idx_signature) {
		SpectrumSignature * ss= new SpectrumSignature(_n_blocks_fft, n_blocks_record_signature);
		_signatures.push_back(ss);
	}
}


void Audio::reinit_signatures() {
	for (unsigned int idx_signature=0; idx_signature<_signatures.size(); ++idx_signature) {
		_signatures[idx_signature]->_active= false;
	}
}


int Audio::get_idx_signature(unsigned int idx_record) {
	unsigned int n_blocks_record_signature= 0;
	// en mode record on ne connait pas le futur...
	if (_mode== AUDIO_RECORD) {
		n_blocks_record_signature= 1;
	}
	else {
		n_blocks_record_signature= SIGNATURE_TIME* (float)(SAMPLE_RATE)/ (float)(SAMPLES_PER_BUFFER);
	}

	// si on en trouve une inactive on la prend
	for (unsigned int idx_signature=0; idx_signature<_signatures.size(); ++idx_signature) {
		if (!_signatures[idx_signature]->_active) {
			for (unsigned int idx_fft=0; idx_fft<_n_blocks_fft; ++idx_fft) {
				for (unsigned int idx_record_offset=0; idx_record_offset<n_blocks_record_signature; ++idx_record_offset) {
					unsigned int idx_block= ((idx_record+ idx_record_offset)* _n_blocks_fft+ idx_fft) % (_n_blocks_fft* _n_blocks_record);
					_signatures[idx_signature]->_values[idx_record_offset* _n_blocks_fft+ idx_fft]= _blocks[idx_block]->_instant_energy;
				}
			}
			_signatures[idx_signature]->_active= true;
			
			sort_signatures();

			return idx_signature;
		}
	}

	// calcul des distances du nouveau son avec les signatures existantes
	float min_dist_new= 1e9;
	int closest2new= -1;
	float dists2new[_signatures.size()];
	for (unsigned int idx_signature=0; idx_signature<_signatures.size(); ++idx_signature) {
		dists2new[idx_signature]= 0.0f;
		// on moyenne sur le temps pour une bande FFT donnée, et on somme par bande FFT
		for (unsigned int idx_fft=0; idx_fft<_n_blocks_fft; ++idx_fft) {
			float d= 0.0f;
			for (unsigned int idx_record_offset=0; idx_record_offset<n_blocks_record_signature; ++idx_record_offset) {
				unsigned int idx_block= ((idx_record+ idx_record_offset)* _n_blocks_fft+ idx_fft) % (_n_blocks_fft* _n_blocks_record);
				d+= pow(_signatures[idx_signature]->_values[idx_record_offset* _n_blocks_fft+ idx_fft]- _blocks[idx_block]->_instant_energy, 2);
			}
			d/= n_blocks_record_signature;
			dists2new[idx_signature]+= d;
		}
		dists2new[idx_signature]= sqrt(dists2new[idx_signature]);
		if (dists2new[idx_signature]< min_dist_new) {
			min_dist_new= dists2new[idx_signature];
			closest2new= idx_signature;
		}
	}

	// quelles sont les 2 signatures existantes les + proches l'une de l'autre
	float min_dist_current= 1e9;
	int closest1= -1;
	int closest2= -1;
	for (unsigned int i1=0; i1<_signatures.size()- 1; ++i1) {
		for (unsigned int i2=i1+ 1; i2<_signatures.size(); ++i2) {
			float dist= 0.0f;
			for (unsigned int idx_fft=0; idx_fft<_n_blocks_fft; ++idx_fft) {
				float d= 0.0f;
				for (unsigned int idx_record_offset=0; idx_record_offset<n_blocks_record_signature; ++idx_record_offset) {
					d+= pow(_signatures[i1]->_values[idx_record_offset* _n_blocks_fft+ idx_fft]- _signatures[i2]->_values[idx_record_offset* _n_blocks_fft+ idx_fft], 2);
				}
				d/= n_blocks_record_signature;
				dist+= d;
			}
			dist= sqrt(dist);

			if (dist< min_dist_current) {
				min_dist_current= dist;
				closest1= i1;
				closest2= i2;
			}
		}
	}

	// si le son courant est + proche d'une signature que les 2 existantes les + proches on renvoie cette signature
	if (min_dist_new< min_dist_current) {
		return closest2new;
	}
	// sinon le son courant va prendre la place de la signature la + proche de lui, entre les 2 signatures les + proches
	// le but est d'avoir la + petite distance entre 2 signatures la plus grande possible
	else {
		int idx2modify= -1;
		if (dists2new[closest1]< dists2new[closest2]) {
			idx2modify= closest1;
		}
		else {
			idx2modify= closest2;
		}
		for (unsigned int idx_fft=0; idx_fft<_n_blocks_fft; ++idx_fft) {
			for (unsigned int idx_record_offset=0; idx_record_offset<n_blocks_record_signature; ++idx_record_offset) {
				unsigned int idx_block= ((idx_record+ idx_record_offset)* _n_blocks_fft+ idx_fft) % (_n_blocks_fft* _n_blocks_record);
				_signatures[idx2modify]->_values[idx_record_offset* _n_blocks_fft+ idx_fft]= _blocks[idx_block]->_instant_energy;
			}
		}
		
		sort_signatures();
		
		return idx2modify;
	}
}


void Audio::sort_signatures() {
	sort(_signatures.begin(), _signatures.end(), cmp_signatures);
}


float Audio::event_length(long idx_sample) {
	// recherche de la longueur en secondes d'un son triggé
	unsigned int min_offset= (unsigned int)(TRIG_MIN_LENGTH* (float)(SAMPLE_RATE)/ (float)(SAMPLES_PER_BUFFER));
	unsigned int max_offset= (unsigned int)(TRIG_MAX_LENGTH* (float)(SAMPLE_RATE)/ (float)(SAMPLES_PER_BUFFER));
	unsigned int idx_record_start= idx_sample/ SAMPLES_PER_BUFFER;
	unsigned int idx_record_end= idx_record_start+ max_offset;
	bool triggered= false;

	// si l'energie devient trop faible ou si un son de meme signature est triggé, stop
	for (unsigned int idx_record_offset=min_offset; idx_record_offset<max_offset; ++idx_record_offset) {
		if (idx_record_start+ idx_record_offset>= _n_blocks_record)
			break;

		float mean_energy= 0.0f;
		for (unsigned int idx_fft=0; idx_fft<_n_blocks_fft; ++idx_fft) {
			unsigned int idx_block= (idx_record_start+ idx_record_offset)* _n_blocks_fft+ idx_fft;
			mean_energy+= _blocks[idx_block]->_instant_energy;

			if ((_blocks[idx_block]->_is_triggered) && (get_block_idx_signature(idx_record_start+ idx_record_offset)== get_block_idx_signature(idx_record_start))) {
				triggered= true;
				break;
			}
		}
		mean_energy/= _n_blocks_fft;
		if ((mean_energy< MEAN_ENERGY_TRESHOLD) || (triggered)) {
			//cout << idx_record_offset << " ; " << triggered << " ; " << mean_energy << endl;
			idx_record_end= idx_record_start+ idx_record_offset;
			break;
		}
	}
	return (float)((idx_record_end- idx_record_start)* SAMPLES_PER_BUFFER)/ (float)(SAMPLE_RATE);
}


// seul le block d'indice fft = 0 de la bande a son _idx_signature écrit et lu. un peu moche mais bon...
void Audio::set_block_idx_signature(unsigned int idx_record, int idx_signature) {
	_blocks[idx_record* _n_blocks_fft+ 0]->_idx_signature= idx_signature;
}


int Audio::get_block_idx_signature(unsigned int idx_record) {
	return _blocks[idx_record* _n_blocks_fft+ 0]->_idx_signature;
}


// ------------------------------------------------------------------------------------------------------------------------
VisuWave::VisuWave() {

}


VisuWave::VisuWave(GLuint prog_draw_2d, Audio * audio) :
	 _prog_draw(prog_draw_2d), _sample_center(0), _sample_width(WAVE_SAMPLE_WIDTH_INIT), _n_vertices(WAVE_N_VERTICES), _audio(audio), _mouse_down(false)
{
	_data= new float[5* _n_vertices* 2]; // stereo

	glGenBuffers(1, &_buffer);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float) * 16);
	glm::mat4 glm_ortho= glm::ortho(-WAVE_WIDTH* 0.5f, WAVE_WIDTH* 0.5f, -WAVE_HEIGHT* 0.5f, WAVE_HEIGHT* 0.5f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float) * 16);
}


VisuWave::~VisuWave() {
	delete[] _data;
}


void VisuWave::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	// left puis right channel
	glDrawArrays(GL_LINE_STRIP, 0, _n_vertices);
	glDrawArrays(GL_LINE_STRIP, _n_vertices, _n_vertices);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VisuWave::update_data() {
	// stereo
	for (unsigned int j=0; j<2; ++j)
		for (unsigned int i=0; i<_n_vertices; ++i) {
			int idx= 5* (i+ j* _n_vertices);
			float x= -WAVE_WIDTH* 0.5f+ (float)(i)* WAVE_WIDTH/ (float)(_n_vertices);
			long idx_sample= _sample_center+ (long)(x* (float)(_sample_width)/ (WAVE_WIDTH* 0.5f));
			float y= (-(float)(j)* 0.5f+ 0.25f)* WAVE_HEIGHT;
			if ((idx_sample>= 0) && (idx_sample< _audio->_n_samples)) {
				// affichage max == * 0.25f, mais illisible, donc on met moins
				y+= _audio->_amplitudes[2* idx_sample+ j]* WAVE_HEIGHT* 0.15f;
			}
			
			_data[idx+ 0]= x;
			_data[idx+ 1]= y;

			float color_strength= WAVE_UNSELECTED_STRENGTH;
			if ((idx_sample>= _audio->_left_selection) && (idx_sample< _audio->_right_selection))
				color_strength= WAVE_SELECTED_STRENGTH;
			if ((idx_sample>= _audio->_current_sample) && (idx_sample< _audio->_current_sample+ SAMPLES_PER_BUFFER)) {
				_data[idx+ 2]= WAVE_SELECTED_COLOR[0]* color_strength;
				_data[idx+ 3]= WAVE_SELECTED_COLOR[1]* color_strength;
				_data[idx+ 4]= WAVE_SELECTED_COLOR[2]* color_strength;
			}
			else {
				_data[idx+ 2]= WAVE_UNSELECTED_COLOR[0]* color_strength;
				_data[idx+ 3]= WAVE_UNSELECTED_COLOR[1]* color_strength;
				_data[idx+ 4]= WAVE_UNSELECTED_COLOR[2]* color_strength;
			}
		}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_vertices* 5* 2* sizeof(float), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool VisuWave::mouse_motion(InputState * input_state) {
	if (_mouse_down) {
		if (input_state->_left_mouse) {
			float xx= 2.0f* (float(input_state->_x- WAVE_WIN_X)/ float(WAVE_WIN_WIDTH))- 1.0f;
			_audio->_right_selection= _sample_center+ (long)(xx* (float)(_sample_width));
			if (_audio->_right_selection< _audio->_left_selection) {
				_audio->_right_selection= _audio->_left_selection;
			}
			if (_audio->_right_selection> _audio->_n_samples)
				_audio->_right_selection= _audio->_n_samples;
			update_data();
			return true;
		}
		else if (input_state->_middle_mouse) {
			
		}
		else if (input_state->_right_mouse) {
			_sample_width+= (long)((float)(input_state->_yrel)* 600.0f);
			if (_sample_width< WAVE_SAMPLE_WIDTH_MIN)
				_sample_width= WAVE_SAMPLE_WIDTH_MIN;
			if (_sample_width> WAVE_SAMPLE_WIDTH_MAX)
				_sample_width= WAVE_SAMPLE_WIDTH_MAX;

			_sample_center-= (long)((float)(input_state->_xrel)* (float)(_sample_width)* 0.005f);
			update_data();
			return true;
		}
	}
	return false;
}


bool VisuWave::mouse_button_down(InputState * input_state) {
	if ((input_state->_x>= WAVE_WIN_X) && (input_state->_x< WAVE_WIN_X+ WAVE_WIN_WIDTH) &&
		(input_state->_y>= MAIN_WIN_HEIGHT- WAVE_WIN_Y- WAVE_WIN_HEIGHT) && (input_state->_y< MAIN_WIN_HEIGHT- WAVE_WIN_Y)) {
		_mouse_down= true;
		if (input_state->_left_mouse) {
			float xx= 2.0f* (float(input_state->_x- WAVE_WIN_X)/ float(WAVE_WIN_WIDTH))- 1.0f;
			_audio->_left_selection= _sample_center+ (long)(xx* (float)(_sample_width));
			if (_audio->_left_selection< 0)
				_audio->_left_selection= 0;
			if (_audio->_left_selection> _audio->_n_samples)
				_audio->_left_selection= _audio->_n_samples;
			_audio->_right_selection= _audio->_left_selection;
			update_data();
		}
		return true;
	}
	return false;
}


bool VisuWave::mouse_button_up(InputState * input_state) {
	if (_audio->_left_selection== _audio->_right_selection) {
		_audio->_left_selection= 0;
		_audio->_right_selection= _audio->_n_samples;
		update_data();
	}
	_mouse_down= false;
	return true;
}


bool VisuWave::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample- SAMPLES_PER_BUFFER>= 0) {
				//audio->_current_sample-= SAMPLES_PER_BUFFER; // fait dans VisuArt
				update_data();
			}
		}
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample+ SAMPLES_PER_BUFFER< _audio->_n_samples) {
				//_audio->_current_sample+= SAMPLES_PER_BUFFER; // fait dans VisuArt
				update_data();
			}
		}
		return true;
	}
	return false;
}


// ------------------------------------------------------------------------------------------------------------------------
GLSpectrum::GLSpectrum() {

}


GLSpectrum::GLSpectrum(GLuint prog_draw_3d, Audio * audio) :
	_prog_draw(prog_draw_3d), _alpha(1.0f), _shininess(SPECTRUM_SHININESS), _audio(audio), _ambient(SPECTRUM_AMBIENT_COLOR),
	_model2world(glm::mat4(1.0f)), _model2camera(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f)) {

	_width_step= SPECTRUM_WIDTH_STEP_INIT;
	_height_step= SPECTRUM_HEIGHT_STEP_INIT;
	_width= (_audio->_n_blocks_fft- 1)* _width_step;
	_height= (_audio->_n_blocks_record- 1)* _height_step;

	_n_faces= (_audio->_n_blocks_fft- 1)* (_audio->_n_blocks_record- 1)* 2;
	_faces= new unsigned int[3* _n_faces];
	_data= new float[(3+ 3+ 3)* 3* _n_faces];

	// Buffer d'indices : puisque l'on duplique tous les sommets pour ne pas avoir de normale partagée, 
	// faces = { 0,1,2,3,4,5,6,7,8,9,10,... }
	for (unsigned int i=0; i<3* _n_faces; i++) {
		_faces[i]= i;
	}
	
	/* buffers est un tableau de 2 indices qui nous permettra de rappeler le tableau de données
		(sommets, couleurs, normales, ...) et le tableau d'indices des triangles */
	glGenBuffers(2, _buffers);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3* _n_faces* sizeof(unsigned int), _faces, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(_prog_draw);
	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc       = glGetAttribLocation(_prog_draw, "normal_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");

	_ambient_color_loc= glGetUniformLocation(_prog_draw, "ambient_color");
	_shininess_loc    = glGetUniformLocation(_prog_draw, "shininess");
	
	_model2clip_loc  = glGetUniformLocation(_prog_draw, "model2clip_matrix");
	_model2camera_loc= glGetUniformLocation(_prog_draw, "model2camera_matrix");
	_normal_mat_loc  = glGetUniformLocation(_prog_draw, "normal_matrix");
	
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");
	glUseProgram(0);
}


GLSpectrum::~GLSpectrum() {
	delete[] _faces;
	delete[] _data;
}


void GLSpectrum::draw() {
	glUseProgram(_prog_draw);
	// On précise les données que l'on souhaite utiliser
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	// On précise le tableau d'indices de triangle à utiliser
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	
	glUniformMatrix4fv(_model2clip_loc  , 1, GL_FALSE, glm::value_ptr(_model2clip));
	glUniformMatrix4fv(_model2camera_loc, 1, GL_FALSE, glm::value_ptr(_model2camera));
	glUniformMatrix3fv(_normal_mat_loc  , 1, GL_FALSE, glm::value_ptr(_normal));
	glUniform3fv(_ambient_color_loc, 1, glm::value_ptr(_ambient));
	glUniform1f(_shininess_loc, _shininess);
	glUniform1f(_alpha_loc, _alpha);

	// Enables the attribute indices
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_normal_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	// Modifie les tableaux associés au buffer en cours d'utilisation
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), 0);
	glVertexAttribPointer(_normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)(3* sizeof(float)));
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
	
	// Rendu de notre geometrie
	glDrawElements(GL_TRIANGLES, _n_faces* 3, GL_UNSIGNED_INT, 0);

	// Disables the attribute indices
	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_normal_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);
	
	// on réinit à 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void GLSpectrum::anim(glm::mat4 world2camera, glm::mat4 camera2clip) {
	_model2camera= world2camera* _model2world;
	_model2clip= camera2clip* _model2camera;
	// theoriquement il faudrait prendre la transposee de l'inverse mais si model2camera est 
	// une matrice orthogonale, TRANS(INV(M)) == M, ce qui est le cas lorsqu'elle ne comprend que 
	// des translations et rotations
	_normal= glm::mat3(_model2camera);
}


void GLSpectrum::update_data() {
	for (unsigned int idx_record=0; idx_record<_audio->_n_blocks_record- 1; ++idx_record)
		for (unsigned int idx_fft=0; idx_fft<_audio->_n_blocks_fft- 1; ++idx_fft) {
			
			float xmin= -_width * 0.5f+ (float)(idx_fft)* _width_step;
			//float ymin= -_audio->_current_sample* _height_step/ SAMPLES_PER_BUFFER+ (float)(idx_record)* _height_step;
			float ymin= (float)(idx_record)* _height_step;
			float xmax= xmin+ _width_step;
			float ymax= ymin+ _height_step;
			float z0= _audio->_blocks[(idx_fft+ 0)+ _audio->_n_blocks_fft* (idx_record+ 0)]->_instant_energy* SPECTRUM_Z_FACTOR;
			float z1= _audio->_blocks[(idx_fft+ 1)+ _audio->_n_blocks_fft* (idx_record+ 0)]->_instant_energy* SPECTRUM_Z_FACTOR;
			float z2= _audio->_blocks[(idx_fft+ 1)+ _audio->_n_blocks_fft* (idx_record+ 1)]->_instant_energy* SPECTRUM_Z_FACTOR;
			float z3= _audio->_blocks[(idx_fft+ 0)+ _audio->_n_blocks_fft* (idx_record+ 1)]->_instant_energy* SPECTRUM_Z_FACTOR;

			unsigned int idx_face= idx_fft+ (_audio->_n_blocks_fft- 1)* idx_record;
			
			float coord[4][3]= { {xmin, ymin, z0}, {xmax, ymin, z1}, {xmax, ymax, z2}, {xmin, ymax, z3} };
			//cout << xmin << " ; " << ymin << " ; " << z0 << " ; " << xmax << " ; " << ymin << " ; " << z1 << " ; " << xmax << " ; " << ymax << " ; " << z2 << " ; " << xmin << " ; " << ymax << " ; " << z3 << endl;
			
			float color[4][3];

			for (int ii=0; ii<2; ++ii)
				for (int jj=0; jj<2; ++jj) {
					float color_strength= SPECTRUM_UNSELECTED_STRENGTH;
					if (idx_record+ jj== _audio->_current_sample/ SAMPLES_PER_BUFFER)
						color_strength= SPECTRUM_SELECTED_STRENGTH;

					int idx= 0; 
					if ((ii==0) && (jj==0)) idx= 0;
					else if ((ii==1) && (jj==0)) idx= 1;
					else if ((ii==1) && (jj==1)) idx= 2;
					else if ((ii==0) && (jj==1)) idx= 3;
					
					if (_audio->_blocks[(idx_fft+ ii)+ _audio->_n_blocks_fft* (idx_record+ jj)]->_is_triggered) {
						color[idx][0]= SPECTRUM_TRIGGERED_COLOR[0]* color_strength;
						color[idx][1]= SPECTRUM_TRIGGERED_COLOR[1]* color_strength;
						color[idx][2]= SPECTRUM_TRIGGERED_COLOR[2]* color_strength;
					} else {
						color[idx][0]= SPECTRUM_UNTRIGGERED_COLOR[0]* color_strength;
						color[idx][1]= SPECTRUM_UNTRIGGERED_COLOR[1]* color_strength;
						color[idx][2]= SPECTRUM_UNTRIGGERED_COLOR[2]* color_strength;
					}
				}

			float norm[3];
			
			calculate_normal(coord[0], coord[1], coord[2], norm);
			
			_data[27* (2* idx_face+ 0)+  0]= coord[0][0];
			_data[27* (2* idx_face+ 0)+  1]= coord[0][1];
			_data[27* (2* idx_face+ 0)+  2]= coord[0][2];
			_data[27* (2* idx_face+ 0)+  3]= norm[0];
			_data[27* (2* idx_face+ 0)+  4]= norm[1];
			_data[27* (2* idx_face+ 0)+  5]= norm[2];
			_data[27* (2* idx_face+ 0)+  6]= color[0][0];
			_data[27* (2* idx_face+ 0)+  7]= color[0][1];
			_data[27* (2* idx_face+ 0)+  8]= color[0][2];

			_data[27* (2* idx_face+ 0)+  9]= coord[1][0];
			_data[27* (2* idx_face+ 0)+ 10]= coord[1][1];
			_data[27* (2* idx_face+ 0)+ 11]= coord[1][2];
			_data[27* (2* idx_face+ 0)+ 12]= norm[0];
			_data[27* (2* idx_face+ 0)+ 13]= norm[1];
			_data[27* (2* idx_face+ 0)+ 14]= norm[2];
			_data[27* (2* idx_face+ 0)+ 15]= color[1][0];
			_data[27* (2* idx_face+ 0)+ 16]= color[1][1];
			_data[27* (2* idx_face+ 0)+ 17]= color[1][2];

			_data[27* (2* idx_face+ 0)+ 18]= coord[2][0];
			_data[27* (2* idx_face+ 0)+ 19]= coord[2][1];
			_data[27* (2* idx_face+ 0)+ 20]= coord[2][2];
			_data[27* (2* idx_face+ 0)+ 21]= norm[0];
			_data[27* (2* idx_face+ 0)+ 22]= norm[1];
			_data[27* (2* idx_face+ 0)+ 23]= norm[2];
			_data[27* (2* idx_face+ 0)+ 24]= color[2][0];
			_data[27* (2* idx_face+ 0)+ 25]= color[2][1];
			_data[27* (2* idx_face+ 0)+ 26]= color[2][2];
			
			// -------
			
			calculate_normal(coord[0], coord[2], coord[3], norm);
			
			_data[27* (2* idx_face+ 1)+  0]= coord[0][0];
			_data[27* (2* idx_face+ 1)+  1]= coord[0][1];
			_data[27* (2* idx_face+ 1)+  2]= coord[0][2];
			_data[27* (2* idx_face+ 1)+  3]= norm[0];
			_data[27* (2* idx_face+ 1)+  4]= norm[1];
			_data[27* (2* idx_face+ 1)+  5]= norm[2];
			_data[27* (2* idx_face+ 1)+  6]= color[0][0];
			_data[27* (2* idx_face+ 1)+  7]= color[0][1];
			_data[27* (2* idx_face+ 1)+  8]= color[0][2];

			_data[27* (2* idx_face+ 1)+  9]= coord[2][0];
			_data[27* (2* idx_face+ 1)+ 10]= coord[2][1];
			_data[27* (2* idx_face+ 1)+ 11]= coord[2][2];
			_data[27* (2* idx_face+ 1)+ 12]= norm[0];
			_data[27* (2* idx_face+ 1)+ 13]= norm[1];
			_data[27* (2* idx_face+ 1)+ 14]= norm[2];
			_data[27* (2* idx_face+ 1)+ 15]= color[2][0];
			_data[27* (2* idx_face+ 1)+ 16]= color[2][1];
			_data[27* (2* idx_face+ 1)+ 17]= color[2][2];

			_data[27* (2* idx_face+ 1)+ 18]= coord[3][0];
			_data[27* (2* idx_face+ 1)+ 19]= coord[3][1];
			_data[27* (2* idx_face+ 1)+ 20]= coord[3][2];
			_data[27* (2* idx_face+ 1)+ 21]= norm[0];
			_data[27* (2* idx_face+ 1)+ 22]= norm[1];
			_data[27* (2* idx_face+ 1)+ 23]= norm[2];
			_data[27* (2* idx_face+ 1)+ 24]= color[3][0];
			_data[27* (2* idx_face+ 1)+ 25]= color[3][1];
			_data[27* (2* idx_face+ 1)+ 26]= color[3][2];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 3)* 3* _n_faces* sizeof(float), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ------------------------------------------------------------------------------------------------------------------------
VisuSpectrum::VisuSpectrum() {

}


VisuSpectrum::VisuSpectrum(GLuint prog_draw_3d, GLuint prog_repere, Audio * audio) : _mouse_down(false), _audio(audio) {
	_gl_spectrum= new GLSpectrum(prog_draw_3d, audio);

	_view_system= new ViewSystem(prog_repere, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	_view_system->_repere->_is_repere= false;
	_view_system->_repere->_is_ground= false;
	_view_system->_repere->_is_box= false;
	_view_system->move_rho(500.0f);
}


VisuSpectrum::~VisuSpectrum() {
	delete _gl_spectrum;
	delete _view_system;
}


void VisuSpectrum::draw() {
	_gl_spectrum->draw();
	_view_system->draw();
}


void VisuSpectrum::anim() {
	_gl_spectrum->anim(_view_system->_world2camera, _view_system->_camera2clip);
}


bool VisuSpectrum::mouse_motion(InputState * input_state) {
	if (_mouse_down) {
		if (input_state->_left_mouse) {
			_view_system->move_target(glm::vec3((float)(input_state->_xrel)* 1.0f, (float)(input_state->_yrel)* 1.0f, 0.0f));
			return true;
		}
		else if (input_state->_middle_mouse) {
			_view_system->move_rho(-(float)(input_state->_yrel)* 1.0f);
			return true;
		}
		else if (input_state->_right_mouse) {
			_gl_spectrum->_height_step+= (float)(input_state->_xrel)* 0.01f;
			if (_gl_spectrum->_height_step< SPECTRUM_HEIGHT_STEP_MIN)
				_gl_spectrum->_height_step= SPECTRUM_HEIGHT_STEP_MIN;
			if (_gl_spectrum->_height_step> SPECTRUM_HEIGHT_STEP_MAX)
				_gl_spectrum->_height_step= SPECTRUM_HEIGHT_STEP_MAX;

			_gl_spectrum->update_data();
			return true;
		}
	}
	return false;
}


bool VisuSpectrum::mouse_button_down(InputState * input_state) {
	if ((input_state->_x>= SPECTRUM_WIN_X) && (input_state->_x< SPECTRUM_WIN_X+ SPECTRUM_WIN_WIDTH) &&
		(input_state->_y> MAIN_WIN_HEIGHT- SPECTRUM_WIN_Y- SPECTRUM_WIN_HEIGHT) && (input_state->_y< MAIN_WIN_HEIGHT- SPECTRUM_WIN_Y)) {
		_mouse_down= true;
		return true;
	}
	return false;
}


bool VisuSpectrum::mouse_button_up(InputState * input_state) {
	_mouse_down= false;
	return true;
}


bool VisuSpectrum::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample- SAMPLES_PER_BUFFER>= 0) {
				//audio->_current_sample-= SAMPLES_PER_BUFFER; // fait dans VisuArt
				_gl_spectrum->update_data();
			}
		}
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample+ SAMPLES_PER_BUFFER< _audio->_n_samples) {
				//_audio->_current_sample+= SAMPLES_PER_BUFFER; // fait dans VisuArt
				_gl_spectrum->update_data();
			}
		}
		return true;
	}
	return false;
}


// ------------------------------------------------------------------------------------------------------------------------
VisuSimu::VisuSimu() {

}


VisuSimu::VisuSimu(GLuint prog_draw_2d, Audio * audio) : _prog_draw(prog_draw_2d), _audio(audio) {
	_data= new float[30* _audio->_n_blocks_fft];

	glGenBuffers(1, &_buffer);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float)* 16);
	glm::mat4 glm_ortho= glm::ortho(-0.5f* SIMU_WIDTH, 0.5f* SIMU_WIDTH, -0.5f* SIMU_HEIGHT, 0.5f* SIMU_HEIGHT, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);

	float width_step= SIMU_WIDTH/ (float)(_audio->_n_blocks_fft)- 2.0f* SIMU_WIDTH_MARGIN;
	for (unsigned int i=0; i<_audio->_n_blocks_fft; ++i) {
		float xmin= -0.5f* SIMU_WIDTH+ SIMU_WIDTH_MARGIN+ (float)(i)* (width_step+ 2.0f* SIMU_WIDTH_MARGIN);
		float xmax= xmin+ width_step;
		float ymin= -0.5f* SIMU_HEIGHT+ SIMU_HEIGHT_MARGIN;
		float ymax= 0.5f* SIMU_HEIGHT- SIMU_HEIGHT_MARGIN;
		
		_data[30* i+ 0]= xmin;
		_data[30* i+ 1]= ymin;
		_data[30* i+ 2]= 0.0f;
		_data[30* i+ 3]= 0.0f;
		_data[30* i+ 4]= 0.0f;

		_data[30* i+ 5]= xmax;
		_data[30* i+ 6]= ymin;
		_data[30* i+ 7]= 0.0f;
		_data[30* i+ 8]= 0.0f;
		_data[30* i+ 9]= 0.0f;

		_data[30* i+ 10]= xmax;
		_data[30* i+ 11]= ymax;
		_data[30* i+ 12]= 0.0f;
		_data[30* i+ 13]= 0.0f;
		_data[30* i+ 14]= 0.0f;

		_data[30* i+ 15]= xmin;
		_data[30* i+ 16]= ymin;
		_data[30* i+ 17]= 0.0f;
		_data[30* i+ 18]= 0.0f;
		_data[30* i+ 19]= 0.0f;

		_data[30* i+ 20]= xmax;
		_data[30* i+ 21]= ymax;
		_data[30* i+ 22]= 0.0f;
		_data[30* i+ 23]= 0.0f;
		_data[30* i+ 24]= 0.0f;

		_data[30* i+ 25]= xmin;
		_data[30* i+ 26]= ymax;
		_data[30* i+ 27]= 0.0f;
		_data[30* i+ 28]= 0.0f;
		_data[30* i+ 29]= 0.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, 30* _audio->_n_blocks_fft* sizeof(float), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


VisuSimu::~VisuSimu() {
	delete[] _data;
}


void VisuSimu::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _audio->_n_blocks_fft* 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void VisuSimu::update_data() {
	for (unsigned int idx_time=_audio->_last_sample/ SAMPLES_PER_BUFFER; idx_time<_audio->_current_sample/ SAMPLES_PER_BUFFER; ++idx_time) {
		for (unsigned int idx_freq=0; idx_freq<_audio->_n_blocks_fft; ++idx_freq) {
			BlockFFT* block= _audio->_blocks[idx_freq+ _audio->_n_blocks_fft* idx_time];
			if (block->_is_triggered) {
				for (unsigned int j=0; j<6; ++j) {
					_data[30* idx_freq+ 2+ j* 5]= 1.0f;
				}
			}
			else {
				// les couleurs tendent vers le noir a la vitesse VISU_SIMU_DECREASING_AMOUNT
				for (unsigned int j=0; j<6; ++j) {
					_data[30* idx_freq+ 2+ j* 5]-= VISU_SIMU_DECREASING_AMOUNT;
					if (_data[30* idx_freq+ 2+ j* 5]< 0.0f)
						_data[30* idx_freq+ 2+ j* 5]= 0.0f;
				}
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, 30* _audio->_n_blocks_fft* sizeof(float), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool VisuSimu::mouse_motion(InputState * input_state) {
	return false;
}


bool VisuSimu::mouse_button_down(InputState * input_state) {
	return false;
}


bool VisuSimu::mouse_button_up(InputState * input_state) {
	return false;
}


bool VisuSimu::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample- SAMPLES_PER_BUFFER>= 0) {
				//audio->_current_sample-= SAMPLES_PER_BUFFER; // fait dans VisuArt
				update_data();
			}
		}
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample+ SAMPLES_PER_BUFFER< _audio->_n_samples) {
				//_audio->_current_sample+= SAMPLES_PER_BUFFER; // fait dans VisuArt
				update_data();
			}
		}
		return true;
	}
	return false;
}


// ------------------------------------------------------------------------------------------------------------------------
Envelope::Envelope() {
	reinit();
}


void Envelope::reinit() {
	_active= false;
	_trig_time= 0.0;
	_position= 0.0f;
	_rest_value= 0.0f;
	_stimulated_value= 0.0f;
	_attack= 0.0f;
	_static_release= 0.0f;
	_attack_power= 1.0f; // lineaire
	_release_power= 1.0f;

	set_static_release();
}


float Envelope::get_value() {
	if (!_active) {
		return _rest_value;
	}
	else if ((_attack> 0.0f) && (_position< _attack)) {
		return _rest_value+ (_stimulated_value- _rest_value)* pow(_position/ _attack, _attack_power);
	}
	else if (_release> 0.0f) {
		return _stimulated_value+ (_rest_value- _stimulated_value)* pow((_position- _attack)/ _release, _release_power);
	}
	else {
		return _rest_value;
	}
}


void Envelope::anim(PaTime current_time) {
	if (!_active)
		return;
	
	_position= current_time- _trig_time;
	if (_position> _attack+ _release) {
		_active= false;
	}
}


void Envelope::trig(PaTime current_time) {
	_active= true;
	_trig_time= current_time;
	_position= 0.0f;
}


// en mode record les enveloppes ont un release statique
void Envelope::set_static_release() {
	_release= _static_release;
}


void Envelope::randomize(float min_rest_val, float max_rest_val, float min_diff_val, float max_diff_val, float min_attack, float max_attack, float min_release, float max_release) {
	_rest_value= rand_float(min_rest_val, max_rest_val);
	_stimulated_value= _rest_value+ rand_float(min_diff_val, max_diff_val);
	_attack= rand_float(min_attack, max_attack);
	_static_release= rand_float(min_release, max_release);
	_attack_power= rand_float(MINMAXS_ENV.at("attack_power")._min, MINMAXS_ENV.at("attack_power")._max);
	_release_power= rand_float(MINMAXS_ENV.at("release_power")._min, MINMAXS_ENV.at("release_power")._max);

	set_static_release();
}


void Envelope::load(json js) {
	reinit();
	
	for (json::iterator it= js.begin(); it!= js.end(); ++it) {
		string key= it.key();
		float val= it.value();

		if (key== "attack") {
			_attack= val;
		}
		else if (key== "release") {
			_static_release= val;
		}
		else if (key== "rest_value") {
			_rest_value= val;
		}
		else if (key== "stimulated_value") {
			_stimulated_value= val;
		}
		else if (key== "attack_power") {
			_attack_power= val;
		}
		else if (key== "release_power") {
			_release_power= val;
		}
	}

	set_static_release();
}


void Envelope::load(string ch_json) {
	ifstream istr(ch_json);
	json js;
	istr >> js;
	load(js);
}


json Envelope::get_json() {
	json js;
	
	js["attack"]= _attack;
	js["release"]= _static_release;
	js["rest_value"]= _rest_value;
	js["stimulated_value"]= _stimulated_value;
	js["attack_power"]= _attack_power;
	js["release_power"]= _release_power;
	
	return js;
}


void Envelope::save(string ch_json) {
	ofstream file_json(ch_json);
	file_json << get_json().dump(4);
}


void Envelope::print() {
	cout << get_json().dump(4) << endl;
}


// ------------------------------------------------------------------------------------------------------------------------
MorphingObj::MorphingObj() {
	vector<string> env_keys{"scale_x", "scale_y", "scale_z", "translate_x", "translate_y", "translate_z",
		"rot1_x", "rot1_y", "rot1_z", "rot2_x", "rot2_y", "rot2_z", "alpha", "shininess", "ambient_x", "ambient_y", "ambient_z"};
	
	for (auto & env : env_keys) {
		_envelopes[env]= new Envelope();
	}

	reinit();
	compute_mat();
}


MorphingObj::~MorphingObj() {
	for (auto & env : _envelopes) {
		delete env.second;
	}
}


void MorphingObj::reinit() {
	for (auto & env : _envelopes) {
		env.second->reinit();
	}
	
	// valeurs par défaut différentes de 0.0f ; utilisées par ex dans un json qui ne précise pas ces attributs
	_envelopes["scale_x"]->_rest_value= 1.0f;
	_envelopes["scale_y"]->_rest_value= 1.0f;
	_envelopes["scale_z"]->_rest_value= 1.0f;
	_envelopes["alpha"]->_rest_value= 0.5f;
	_envelopes["shininess"]->_rest_value= 1.0f;
	_envelopes["ambient_x"]->_rest_value= 0.5f;
	_envelopes["ambient_y"]->_rest_value= 0.5f;
	_envelopes["ambient_z"]->_rest_value= 0.5f;
}


void MorphingObj::anim(PaTime current_time) {
	for (auto & env : _envelopes) {
		env.second->anim(current_time);
	}
	compute_mat();
}


void MorphingObj::trig(PaTime current_time) {
	for (auto & env : _envelopes) {
		env.second->trig(current_time);
	}
}


void MorphingObj::set_release(float release) {
	for (auto & env : _envelopes) {
		env.second->_release= release;
	}
}


void MorphingObj::set_static_release() {
	for (auto & env : _envelopes) {
		env.second->set_static_release();
	}
}


void MorphingObj::randomize() {
	vector<string> scale_keys{"scale_x", "scale_y", "scale_z"};
	vector<string> translate_keys{"translate_x", "translate_y", "translate_z"};
	vector<string> rotate_keys{"rot1_x", "rot1_y", "rot1_z", "rot2_x", "rot2_y", "rot2_z"};
	vector<string> ambient_keys{"ambient_x", "ambient_y", "ambient_z"};

	for (auto & it : scale_keys)
		_envelopes[it]->randomize(MINMAXS_ENV.at("scale_base")._min, MINMAXS_ENV.at("scale_base")._max, MINMAXS_ENV.at("scale_diff")._min, MINMAXS_ENV.at("scale_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);
	
	for (auto & it : translate_keys)
		_envelopes[it]->randomize(MINMAXS_ENV.at("translate_base")._min, MINMAXS_ENV.at("translate_base")._max, MINMAXS_ENV.at("translate_diff")._min, MINMAXS_ENV.at("translate_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);

	for (auto & it : rotate_keys)
		_envelopes[it]->randomize(MINMAXS_ENV.at("rotate_base")._min, MINMAXS_ENV.at("rotate_base")._max, MINMAXS_ENV.at("rotate_diff")._min, MINMAXS_ENV.at("rotate_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);

	_envelopes["alpha"]->randomize(MINMAXS_ENV.at("alpha_base")._min, MINMAXS_ENV.at("alpha_base")._max, MINMAXS_ENV.at("alpha_diff")._min, MINMAXS_ENV.at("alpha_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);
	_envelopes["shininess"]->randomize(MINMAXS_ENV.at("shininess_base")._min, MINMAXS_ENV.at("shininess_base")._max, MINMAXS_ENV.at("shininess_diff")._min, MINMAXS_ENV.at("shininess_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);

	for (auto & it : ambient_keys)
		_envelopes[it]->randomize(MINMAXS_ENV.at("ambient_base")._min, MINMAXS_ENV.at("ambient_base")._max, MINMAXS_ENV.at("ambient_diff")._min, MINMAXS_ENV.at("ambient_diff")._max, MINMAXS_ENV.at("attack")._min, MINMAXS_ENV.at("attack")._max, MINMAXS_ENV.at("release")._min, MINMAXS_ENV.at("release")._max);

	compute_mat();
}


void MorphingObj::compute_mat() {
	// les matrices de transfo sont de la forme : rotation2 x translate x rotation1 x scale
	_mat= 
		glm::rotate(_envelopes["rot2_x"]->get_value(), glm::vec3(1.0f, 0.0f, 0.0f))* glm::rotate(_envelopes["rot2_y"]->get_value(), glm::vec3(0.0f, 1.0f, 0.0f))* glm::rotate(_envelopes["rot2_z"]->get_value(), glm::vec3(0.0f, 0.0f, 1.0f))* 
		glm::translate(glm::mat4(1.0f), glm::vec3(_envelopes["translate_x"]->get_value(), _envelopes["translate_y"]->get_value(), _envelopes["translate_z"]->get_value()))*
		glm::rotate(_envelopes["rot1_x"]->get_value(), glm::vec3(1.0f, 0.0f, 0.0f))* glm::rotate(_envelopes["rot1_y"]->get_value(), glm::vec3(0.0f, 1.0f, 0.0f))* glm::rotate(_envelopes["rot1_z"]->get_value(), glm::vec3(0.0f, 0.0f, 1.0f))* 
		glm::scale(glm::vec3(_envelopes["scale_x"]->get_value(), _envelopes["scale_y"]->get_value(), _envelopes["scale_z"]->get_value()));

	//cout << glm::to_string(_mat) << endl;
}


void MorphingObj::load(json js) {
	reinit();

	// les valeurs peuvent etre un chemin relatif vers un json
	for (json::iterator it= js.begin(); it!= js.end(); ++it) {
		string key= it.key();
		json js2;
		try {
			string ch_json2= it.value();
			ifstream istr2(ch_json2);
			istr2 >> js2;
		}
		catch(...) {
			js2= it.value();
		}

		_envelopes[key]->load(js2);
	}
}


void MorphingObj::load(string ch_json) {
	ifstream istr(ch_json);
	json js;
	istr >> js;
	
	load(js);
}


json MorphingObj::get_json() {
	json js;
	
	for (auto & env : _envelopes) {
		js[env.first]= env.second->get_json();
	}

	return js;
}


void MorphingObj::save(string ch_json) {
	json js= get_json();
	ofstream file_json(ch_json);
	file_json << js.dump(4);
}


void MorphingObj::print() {
	cout << get_json().dump(4) << endl;
}


// ------------------------------------------------------------------------------------------------------------------------
Connexion::Connexion() {

}


Connexion::Connexion(unsigned int width, unsigned int height) : _width(width), _height(height) {
	_values= new bool[_width* _height];
	reinit();
}


Connexion::~Connexion() {
	delete[] _values;
}


void Connexion::reinit() {
	for (unsigned int i=0; i<_width* _height; ++i) {
		_values[i]= false;
	}
}


bool Connexion::get(unsigned int x, unsigned int y) {
	if ((x>= _width) || (y>= _height))
		return false;
	return _values[_width* y+ x];
}


void Connexion::set(unsigned int x, unsigned int y, bool b) {
	if ((x>= _width) || (y>= _height))
		return;
	_values[_width* y+ x]= b;
}


// proba 1 / chance que la connexion soit faite
void Connexion::randomize(unsigned int chance) {
	for (unsigned int i=0; i<_width* _height; ++i) {
		if (rand_int(0, chance)) {
			_values[i]= false;
		}
		else {
			_values[i]= true;
		}
	}
}


void Connexion::load(json js) {
	reinit();

	for (json::iterator it= js.begin(); it!= js.end(); ++it) {
		string key= it.key();
		if (key== "width") {
			_width= it.value();
		}
		else if (key== "height") {
			_height= it.value();
		}
		else if (key== "true_indices") {
			json true_indices= it.value();
			for (json::iterator it2= true_indices.begin(); it2!= true_indices.end(); ++it2) {
				unsigned int i= it2->at(0);
				unsigned int j= it2->at(1);
				set(i, j, true);
			}
		}
	}
}


void Connexion::load(string ch_json) {
	ifstream istr(ch_json);
	json js;
	istr >> js;
	load(js);
}


json Connexion::get_json() {
	json js;

	js["width"]= _width;
	js["height"]= _height;
	auto true_indices= json::array();
	for (unsigned int i=0; i<_width; ++i)
		for (unsigned int j=0; j<_height; ++j)
			if (get(i, j)) {
				auto idx= json::array();
				idx.push_back(i);
				idx.push_back(j);
				true_indices.push_back(idx);
			}
	js["true_indices"]= true_indices;

	return js;
}


void Connexion::save(string ch_json) {
	json js= get_json();
	ofstream file_json(ch_json);
	file_json << js.dump(4);
}


void Connexion::print() {
	cout << get_json().dump(4) << endl;
}


// ------------------------------------------------------------------------------------------------------------------------
VisuArt::VisuArt() {

}


VisuArt::VisuArt(GLuint prog_draw_3d, GLuint prog_repere, Audio * audio) :
	_prog_draw_3d(prog_draw_3d), _prog_repere(prog_repere), _audio(audio), _last_idx_signature(-1), _mouse_down(false), _fullscreen(false) {
	
	_view_system= new ViewSystem(prog_repere, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	_view_system->_repere->_is_repere= true;
	_view_system->_repere->_is_ground= false;
	_view_system->_repere->_is_box= true;
	_view_system->move_rho(30.0f);

	vector<string> ch_models= list_files("../data/", "xml");
	for (auto & ch_model : ch_models) {
		_static_models.push_back(new StaticModel("../data/"+ ch_model, _prog_draw_3d));
	}
}


VisuArt::~VisuArt() {
	delete _view_system;
	reinit();
}


void VisuArt::reinit() {
	for (auto it_obj : _static_instances) {
		delete it_obj;
	}
	for (auto it_morph : _morphing_objs) {
		delete it_morph;
	}
	for (auto it_connexion : _connexions) {
		delete it_connexion;
	}
	_static_instances.clear();
	_morphing_objs.clear();
	_connexions.clear();

	_audio->set_n_signatures(0);
}


void VisuArt::draw() {
	for (auto it_obj : _static_instances) {
		it_obj->draw();
	}
	
	_view_system->draw();
}


void VisuArt::anim(PaTime current_time, AudioMode audio_mode) {
	AudioEvent audio_event= {0, 0, -1};

	// on récupère le dernier audio_event pushé dans la queue
	while (_audio->_signatures_event_queue.next(audio_event)) {
	}

	if (audio_event._idx_signature>= 0) {
		//cout << "trig\n";
		_last_idx_signature= audio_event._idx_signature; // pour affichage dans main()

		// une signature == une connexion
		if (audio_event._idx_signature>= _connexions.size()) {
			cout << "ERREUR idx_signature = " << audio_event._idx_signature << " > _connexions.size() = " << _connexions.size() << endl;
			return;
		}
		
		for (unsigned int idx_morph=0; idx_morph<_morphing_objs.size(); ++idx_morph) {
			for (unsigned int idx_obj=0; idx_obj<_static_instances.size(); ++idx_obj) {
				if (_connexions[audio_event._idx_signature]->get(idx_obj, idx_morph)) {
					// si morph est connecté à au moins 1 obj pour cette signature, on set le release et on trig
					if (audio_mode== AUDIO_PLAYBACK) {
						_morphing_objs[idx_morph]->set_release(_audio->event_length(audio_event._idx_sample));
					}
					else {
						_morphing_objs[idx_morph]->set_static_release();
					}
					_morphing_objs[idx_morph]->trig(current_time);
					break;
				}
			}
		}
	}

	// anim des morphs
	for (auto it_morph : _morphing_objs) {
		it_morph->anim(current_time);
	}

	// pour chaque _static_instance , on cumule l'effet de tous les morphs de toutes les connexions
	for (unsigned int idx_obj=0; idx_obj<_static_instances.size(); ++idx_obj) {
		unsigned int n_morph= 0;

		MorphingObj * m= new MorphingObj();

		for (unsigned int idx_conn=0; idx_conn<_connexions.size(); ++idx_conn) {
			for (unsigned int idx_morph=0; idx_morph<_morphing_objs.size(); ++idx_morph) {
				if (!_connexions[idx_conn]->get(idx_obj, idx_morph)) {
					continue;
				}

				n_morph++;

				map<string, Envelope *>::iterator env;
				for (env= m->_envelopes.begin(); env!= m->_envelopes.end(); env++) {
					// pour scale on multiplie
					if ((env->first== "scale_x") || (env->first== "scale_y") || (env->first== "scale_z")) {
						m->_envelopes[env->first]->_rest_value*= _morphing_objs[idx_morph]->_envelopes[env->first]->get_value();
					}
					else {
						m->_envelopes[env->first]->_rest_value+= _morphing_objs[idx_morph]->_envelopes[env->first]->get_value();
					}
				}
			}
		}
		
		m->compute_mat();
		//cout << "--------\n";
		//cout << glm::to_string(m->_mat) << endl;
		_static_instances[idx_obj]->set_pos_rot_scale(m->_mat);
		_static_instances[idx_obj]->anim(_view_system);

		// pour ces attributs on prend la moyenne
		if (n_morph> 0) {
			/*_static_instances[idx_obj]->_alpha= m->_envelopes["alpha"]->_rest_value/ n_morph;
			_static_instances[idx_obj]->_shininess= m->_envelopes["shininess"]->_rest_value/ n_morph;
			_static_instances[idx_obj]->_ambient[0]= m->_envelopes["ambient_x"]->_rest_value/ n_morph;
			_static_instances[idx_obj]->_ambient[1]= m->_envelopes["ambient_y"]->_rest_value/ n_morph;
			_static_instances[idx_obj]->_ambient[2]= m->_envelopes["ambient_z"]->_rest_value/ n_morph;*/
		}

		delete m;
	}
}


void VisuArt::randomize() {
	reinit();

	unsigned int n_objs= rand_int(MINMAX_N_OBJS._min, MINMAX_N_OBJS._max);
	for (unsigned int i=0; i<n_objs; ++i) {
		unsigned int idx_model= rand_int(0, _static_models.size()- 1);
		StaticInstance * model_obj= new StaticInstance(_static_models[idx_model], glm::vec3(1.0f));
		model_obj->_pos_rot->_active= true;
		_static_instances.push_back(model_obj);
	}

	unsigned int n_morphs= rand_int(MINMAX_N_MORPHS._min, MINMAX_N_MORPHS._max);
	for (unsigned int i=0; i<n_morphs; ++i) {
		MorphingObj * morphing_obj= new MorphingObj();
		morphing_obj->randomize();
		_morphing_objs.push_back(morphing_obj);
	}

	unsigned int n_conns= rand_int(MINMAX_N_CONNS._min, MINMAX_N_CONNS._max);
	for (unsigned int i=0; i<n_conns; ++i) {
		Connexion * connexion= new Connexion(_static_instances.size(), _morphing_objs.size());
		connexion->randomize(CONN_CHANCE);
		_connexions.push_back(connexion);
	}

	// 1 signature == 1 connexion
	_audio->set_n_signatures(n_conns);
}


void VisuArt::load(string ch_json) {
	reinit();

	ifstream istr(ch_json);
	json js;
	istr >> js;

	for (json::iterator it= js.begin(); it!= js.end(); ++it) {
		string key= it.key();
		json value= it.value();
		//cout << key << endl;

		if (key== "model_objs") {
			for (json::iterator it2= value.begin(); it2!= value.end(); ++it2) {
				string ch_cfg= (*it2)["cfg"];
				StaticModel * model_ok= NULL;
				for (auto model : _static_models) {
					if (model->_ch_config_file== ch_cfg) {
						model_ok= model;
						break;
					}
				}
				if (!model_ok) {
					cout << "Modele " << ch_cfg << " non trouvé" << endl;
					continue;
				}
				glm::vec3 scale= glm::vec3((*it2)["scale_x"], (*it2)["scale_y"], (*it2)["scale_z"]);
				StaticInstance * model_obj= new StaticInstance(model_ok, scale);
				model_obj->_pos_rot->_active= true;
				_static_instances.push_back(model_obj);
			}
		}
		else if (key== "morphing_objs") {
			for (json::iterator it2= value.begin(); it2!= value.end(); ++it2) {
				json js2;
				try {
					string ch_json2= it2.value();
					ifstream istr2(ch_json2);
					istr2 >> js2;
				}
				catch(...) {
					js2= it2.value();
				}
				
				MorphingObj * morphing_obj= new MorphingObj();
				morphing_obj->load(js2);
				_morphing_objs.push_back(morphing_obj);
			}
		}
		else if (key== "connexions") {
			for (json::iterator it2= value.begin(); it2!= value.end(); ++it2) {
				json js2;
				try {
					string ch_json2= it2.value();
					ifstream istr2(ch_json2);
					istr2 >> js2;
				}
				catch(...) {
					js2= it2.value();
				}

				unsigned int width= js2["width"];
				unsigned int height= js2["height"];
				Connexion * connexion= new Connexion(width, height);
				connexion->load(js2);
				_connexions.push_back(connexion);
			}
		}
	}

	// 1 signature == 1 connexion
	_audio->set_n_signatures(_connexions.size());
}


json VisuArt::get_json() {
	json js;

	auto model_objs= json::array();
	for (auto it_obj : _static_instances) {
		json js2;
		js2["cfg"]= it_obj->_model->_ch_config_file;
		js2["scale_x"]= it_obj->_pos_rot->_scale.x;
		js2["scale_y"]= it_obj->_pos_rot->_scale.y;
		js2["scale_z"]= it_obj->_pos_rot->_scale.z;
		model_objs.push_back(js2);
	}
	js["model_objs"]= model_objs;

	auto morphing_objs= json::array();
	for (auto it_morph : _morphing_objs) {
		morphing_objs.push_back(it_morph->get_json());
	}
	js["morphing_objs"]= morphing_objs;

	auto connexions= json::array();
	for (auto it_conn : _connexions) {
		connexions.push_back(it_conn->get_json());
	}
	js["connexions"]= connexions;

	return js;
}


void VisuArt::save(string ch_json) {
	json js= get_json();
	ofstream file_json(ch_json);
	file_json << js.dump(4);
}



void VisuArt::print() {
	cout << get_json().dump(4) << endl;
	cout << _static_instances.size() << ";" << _morphing_objs.size() << ";" << _connexions.size() << endl;
}


bool VisuArt::mouse_motion(InputState * input_state) {
	if (_mouse_down) {
		if (input_state->_left_mouse) {
			_view_system->move_target(glm::vec3((float)(input_state->_xrel)* 0.5f, (float)(input_state->_yrel)* 0.5f, 0.0f));
			return true;
		}
		else if (input_state->_middle_mouse) {
			_view_system->move_rho(-(float)(input_state->_yrel)* 1.0f);
			return true;
		}
		else if (input_state->_right_mouse) {
			_view_system->move_phi(-(float)(input_state->_xrel)* 0.01f);
			_view_system->move_theta(-(float)(input_state->_yrel)* 0.01f);
			return true;
		}
	}
	return false;
}


bool VisuArt::mouse_button_down(InputState * input_state) {
	if ((input_state->_x>= ART_WIN_X) && (input_state->_x< ART_WIN_X+ ART_WIN_WIDTH) && 
		(input_state->_y> MAIN_WIN_HEIGHT- ART_WIN_Y- ART_WIN_HEIGHT) && (input_state->_y< MAIN_WIN_HEIGHT- ART_WIN_Y)) {
		_mouse_down= true;
		return true;
	}
	return false;
}


bool VisuArt::mouse_button_up(InputState * input_state) {
	_mouse_down= false;
	return true;
}


bool VisuArt::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_c) {
		randomize();
		return true;
	}
	else if (key== SDLK_d) {
		print();
		return true;
	}
	else if (key== SDLK_g) {
		load("../configs/visu_simple_1.json");
		return true;
	}
	else if (key== SDLK_h) {
		save("../configs/visu_test.json");
		return true;
	}
	else if (key== SDLK_f) {
		_fullscreen= !_fullscreen;
		return true;
	}
	else if (key== SDLK_SPACE) {
		if ((_audio->_mode== AUDIO_PLAYBACK) || (_audio->_mode== AUDIO_RECORD)) {
			_audio->_mode= AUDIO_STOP;
		}
		else if (_audio->_mode== AUDIO_STOP) {
			_audio->_mode= AUDIO_PLAYBACK;
		}
		return true;
	}
	else if (key== SDLK_a) {
		if (_audio->_mode== AUDIO_RECORD) {
			_audio->_mode= AUDIO_PLAYBACK;
		}
		else if (_audio->_mode== AUDIO_PLAYBACK) {
			_audio->_mode= AUDIO_RECORD;
		}
		return true;
	}
	else if (key== SDLK_i) {
		_audio->reinit_signatures();
		return true;
	}
	else if (key== SDLK_l) {
		_audio->_mode= AUDIO_PLAYBACK;
		_audio->loadfromfile("../data/record.wav");
		return true;
	}
	else if (key== SDLK_r) {
		_audio->record2file("../data/record.wav");
		return true;
	}
	else if (key== SDLK_LEFT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample- SAMPLES_PER_BUFFER>= 0) {
				_audio->_current_sample-= SAMPLES_PER_BUFFER;
			}
		}
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample+ SAMPLES_PER_BUFFER< _audio->_n_samples) {
				_audio->_current_sample+= SAMPLES_PER_BUFFER;
			}
		}
		return true;
	}
	else if (key== SDLK_UP) {
		_audio->_playback_amplitude+= 0.05f;
		if (_audio->_playback_amplitude> 1.0f) {
			_audio->_playback_amplitude= 1.0f;
		}
		return true;
	}
	else if (key== SDLK_DOWN) {
		_audio->_playback_amplitude-= 0.05f;
		if (_audio->_playback_amplitude< 0.0f) {
			_audio->_playback_amplitude= 0.0f;
		}
		return true;
	}
	return false;
}

