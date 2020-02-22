
#include "springs.h"

using namespace std;



bool sort_ss(SpringSystem* s1, SpringSystem* s2) {
	return (s1->fitness()> s2->fitness());
}


// ------------------------------------------------------------------------------------------
Force::Force() {
}


Force::Force(glm::vec3 vec, std::string type) : _vec(vec), _type(type) {
	if (_type== "weight") {
		_color[0]= 1.0f; _color[1]= 0.0f; _color[2]= 0.0f; // rouge
	}
	else if (_type== "air_resistance") {
		_color[0]= 0.0f; _color[1]= 1.0f; _color[2]= 0.0f; // vert
	}
	else if (_type== "friction") {
		_color[0]= 0.0f; _color[1]= 0.0f; _color[2]= 1.0f; // bleu
	}
	else if (_type== "spring") {
		_color[0]= 0.0f; _color[1]= 1.0f; _color[2]= 1.0f; // cyan
	}
	else if (_type== "damping") {
		_color[0]= 1.0f; _color[1]= 1.0f; _color[2]= 0.0f; // jaune
	}
	else if (_type== "reaction") {
		_color[0]= 1.0f; _color[1]= 0.0f; _color[2]= 1.0f; // violet
	}
	else if (_type== "contract") {
		_color[0]= 0.3f; _color[1]= 0.5f; _color[2]= 1.0f; // ?
	}
	else {
		_color[0]= 0.0f; _color[1]= 0.0f; _color[2]= 0.0f; // noir
	}
}


void Force::print() {
	cout << _type << " : ";
	cout << "x=" << _vec.x << "; y=" << _vec.y << "; z=" << _vec.z << endl;
}


// ------------------------------------------------------------------------------------------
Vertex::Vertex() {
}


Vertex::Vertex(glm::vec3 p, float mass, float air_resist) :
	_p(p), _p_init(p), _v(glm::vec3(0.0f)), _a(glm::vec3(0.0f)), _mass(mass), _air_resist(air_resist) {
	_forces.clear();
}


void Vertex::reinit() {
	_p= glm::vec3(_p_init);
	_v= glm::vec3(0.0f);
	_a= glm::vec3(0.0f);
	_forces.clear();
}


// ------------------------------------------------------------------------------------------
Spring::Spring() {
}


Spring::Spring(unsigned int idx1, unsigned int idx2, float stiffness, float damping, float relaxed_size) :
	_idx1(idx1), _idx2(idx2), _stiffness(stiffness), _damping(damping), _relaxed_size(relaxed_size),
	_contract_amount(-1.0f), _contract_begin(0.0f), _contract_end(0.0f) {

}


Spring::Spring(unsigned int idx1, unsigned int idx2, float stiffness, float damping, float relaxed_size,
	float contract_amount, float contract_begin, float contract_end) :
	_idx1(idx1), _idx2(idx2), _stiffness(stiffness), _damping(damping), _relaxed_size(relaxed_size),
	_contract_amount(contract_amount), _contract_begin(contract_begin), _contract_end(contract_end) {

}


void Spring::rand_contract() {
	if (rand_float(0.0f, 1.0f)< SPRING_CONTRACT_TRESHOLD) {
		_contract_amount= rand_float(SPRING_CONTRACT_MIN_AMOUNT, SPRING_CONTRACT_MAX_AMOUNT);
		_contract_begin= rand_float(0.0f, 1.0f);
		_contract_end= _contract_begin+ rand_float(0.0f, 1.0f- _contract_begin);
	}
	else {
		_contract_amount= -1.0f;
		_contract_begin= 0.0f;
		_contract_end= 0.0f;
	}
}


// ------------------------------------------------------------------------------------------
SpringSystem::SpringSystem() : _tik_init(SDL_GetTicks()) {
}


ostream & operator << (ostream & out_stream, SpringSystem ss) {
	unsigned int i;
	const char DELIM= ';';

	out_stream << "SS" << DELIM;
	out_stream << ss._vertices.size() << DELIM;
	for (i=0; i<ss._vertices.size(); i++) {
		out_stream << ss._vertices[i]._p.x << DELIM << ss._vertices[i]._p.y << DELIM << ss._vertices[i]._p.z << DELIM;
		out_stream << ss._vertices[i]._mass << DELIM << ss._vertices[i]._air_resist << DELIM;
	}
	out_stream << ss._springs.size() << DELIM;
	for (i=0; i<ss._springs.size(); i++) {
		out_stream << ss._springs[i]._idx1 << DELIM << ss._springs[i]._idx2 << DELIM;
		out_stream << ss._springs[i]._stiffness << DELIM << ss._springs[i]._damping << DELIM << ss._springs[i]._relaxed_size << DELIM;
		out_stream << ss._springs[i]._contract_amount << DELIM << ss._springs[i]._contract_begin << DELIM << ss._springs[i]._contract_end << DELIM;
	}

	return out_stream;
}


istream & operator >> (istream & in_stream, SpringSystem & ss) {
	string type;
	char c;
	bool ok= 1;
	const char DELIM= ';';
	unsigned int i, j, i1, i2;
	float t[6];
	SpringSystem ss_tmp;

	ok= bool(getline(in_stream, type, DELIM));
	if (type!= "SS")
		ok= 0;

	in_stream >> j >> c;
	if (c!= DELIM) ok= 0;
	ss_tmp._vertices.clear();
	for (i=0; i<j; i++) {
		in_stream >> t[0] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[1] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[2] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[3] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[4] >> c;
		if (c!= DELIM) ok= 0;
		ss_tmp._vertices.push_back(Vertex(glm::vec3(t[0], t[1], t[2]), t[3], t[4]));
	}

	in_stream >> j >> c;
	if (c!= DELIM) ok= 0;
	ss_tmp._springs.clear();
	for (i=0; i<j; i++) {
		in_stream >> i1 >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> i2 >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[0] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[1] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[2] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[3] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[4] >> c;
		if (c!= DELIM) ok= 0;
		in_stream >> t[5] >> c;
		if (c!= DELIM) ok= 0;
		ss_tmp._springs.push_back(Spring(i1, i2, t[0], t[1], t[2], t[3], t[4], t[5]));
	}

	if (ok) {
		ss._vertices.clear();
		for (i=0; i<ss_tmp._vertices.size(); i++)
			ss._vertices.push_back(ss_tmp._vertices[i]);

		ss._springs.clear();
		for (i=0; i<ss_tmp._springs.size(); i++)
			ss._springs.push_back(ss_tmp._springs[i]);
	}
	else {
		cout << "Erreur >> SpringSystem" << endl;
	}

	return in_stream;
}


void SpringSystem::save(string ch) {
	ofstream ofs(ch.c_str(), ios::out);
	ofs << *this;
	ofs.close();
}


void SpringSystem::load(string ch) {
	ifstream ifs(ch.c_str(), ios::in);
	if (ifs) {
		ifs >> *this;
		ifs.close();
	}
	else {
		cout << "erreur lecture " << ch << endl;
	}
}


void SpringSystem::anim() {
	unsigned int i, j;

	for (i=0; i<_vertices.size(); i++)
		_vertices[i]._forces.clear();

	unsigned int tik= SDL_GetTicks();
	tik-= _tik_init;

	for (i=0; i<_springs.size(); i++) {
		if (_springs[i]._contract_amount> 0.0f) {
			float cycle_pos= float(tik)/ float(SPRING_CONTRACT_CYCLE_TIME); // position au sein du cycle
			cycle_pos-=floor(cycle_pos); // on récupère la partie décimale pour etre 0 et 1
			if ((cycle_pos> _springs[i]._contract_begin) && (cycle_pos< _springs[i]._contract_end)) {
				glm::vec3 contract_force= _springs[i]._contract_amount* normalize(_vertices[_springs[i]._idx2]._p- _vertices[_springs[i]._idx1]._p);
				_vertices[_springs[i]._idx1]._forces.push_back(Force( contract_force, "contract"));
				_vertices[_springs[i]._idx2]._forces.push_back(Force(-contract_force, "contract"));
			}
		}
	}

	for (i=0; i<_vertices.size(); i++) {

		glm::vec3 weight= glm::vec3(0.0f, 0.0f, -_vertices[i]._mass* GRAVITY);
		_vertices[i]._forces.push_back(Force(weight, "weight"));

		glm::vec3 air_resistance= glm::vec3(0.0f);
		if (_vertices[i]._p.z> GROUND_TRESHOLD)
			air_resistance= -_vertices[i]._air_resist* _vertices[i]._v;
		_vertices[i]._forces.push_back(Force(air_resistance, "air_resistance"));

		glm::vec3 friction= glm::vec3(0.0f);
		if (_vertices[i]._p.z< GROUND_TRESHOLD) {
			friction= -FRICTION* glm::vec3(_vertices[i]._v.x, _vertices[i]._v.y, 0.0f);
		}
		_vertices[i]._forces.push_back(Force(friction, "friction"));

		vector<unsigned int> adjs= get_ajdacents_springs(i);
		for (j=0; j<adjs.size(); j++) {
			unsigned int adj_vertex;
			if (_springs[adjs[j]]._idx1== i)
				adj_vertex= _springs[adjs[j]]._idx2;
			else
				adj_vertex= _springs[adjs[j]]._idx1;
			glm::vec3 spring_force= _springs[adjs[j]]._stiffness* (glm::distance(_vertices[i]._p, _vertices[adj_vertex]._p)-
				_springs[adjs[j]]._relaxed_size)* glm::normalize(_vertices[adj_vertex]._p- _vertices[i]._p);

			glm::vec3 damping_force= glm::vec3(0.0f);
			float spring_norm= float(glm::l2Norm(spring_force));
			if (spring_norm> SPRING_DAMP_TRESHOLD) {
				glm::vec3 spring_normalized= (1.0f/ spring_norm)* spring_force;
				float v_scal_spring= dot(_vertices[i]._v, spring_force);
				if (v_scal_spring> 0.0f)
					damping_force= -_springs[adjs[j]]._damping* v_scal_spring* spring_normalized;
			}

			_vertices[i]._forces.push_back(Force(spring_force, "spring"));
			_vertices[i]._forces.push_back(Force(damping_force, "damping"));
		}

		// a faire en dernier !
		if (_vertices[i]._p.z< GROUND_TRESHOLD) {
			glm::vec3 forces_sum= glm::vec3(0.0f);
			for (j=0; j<_vertices[i]._forces.size(); j++)
				forces_sum+= _vertices[i]._forces[j]._vec;

			if (forces_sum.z< 0.0f) {
				glm::vec3 reaction= glm::vec3(0.0f, 0.0f, -forces_sum.z- (_vertices[i]._mass/ ANIM_STEP)* _vertices[i]._v.z);
				_vertices[i]._forces.push_back(Force(reaction, "reaction"));
			}
		}

	}

	for (i=0; i<_vertices.size(); i++) {
		glm::vec3 forces_sum= glm::vec3(0.0f);
		for (j=0; j<_vertices[i]._forces.size(); j++)
			forces_sum+= _vertices[i]._forces[j]._vec;

		_vertices[i]._a= forces_sum/ _vertices[i]._mass;
		_vertices[i]._v= _vertices[i]._v+ ANIM_STEP* _vertices[i]._a;
		_vertices[i]._p= _vertices[i]._p+ ANIM_STEP* _vertices[i]._v;

		if (_vertices[i]._p.z< 0.0f)
			_vertices[i]._p.z= 0.0f;
	}
}


void SpringSystem::add_vertex(glm::vec3 p, float mass, float air_resist) {
	if (_vertices.size()>= MAX_VERTICES) {
		cout << "TOO MANY VERTICES" << endl;
		return;
	}

	_vertices.push_back(Vertex(p, mass, air_resist));
}


void SpringSystem::add_spring(unsigned int idx1, unsigned int idx2, float stiffness, float damping, float relaxed_size) {
	if (_springs.size()>= MAX_SPRINGS) {
		cout << "TOO MANY SPRINGS" << endl;
		return;
	}

	if (idx1== idx2) {
		cout << "BAD SPRING" << endl;
		return;
	}

	if (relaxed_size< 0.0f)
		relaxed_size= glm::distance(_vertices[idx1]._p, _vertices[idx2]._p);
	_springs.push_back(Spring(idx1, idx2, stiffness, damping, relaxed_size));
}


void SpringSystem::delete_vertex(unsigned int idx) {
	if (idx> _vertices.size()) {
		cout << "NOT ENOUGH VERTICES" << endl;
		return;
	}

	_vertices.erase(_vertices.begin()+ idx);

	for (int i=_springs.size()- 1; i>=0; i--) {
		if ((_springs[i]._idx1== idx) || (_springs[i]._idx2== idx))
			delete_spring(i);
		else {
			if (_springs[i]._idx1> idx)
				_springs[i]._idx1--;
			if (_springs[i]._idx2> idx)
				_springs[i]._idx2--;
		}
	}
}


void SpringSystem::delete_spring(unsigned int idx) {
	if (idx> _springs.size()) {
		cout << "NOT ENOUGH SPRINGS" << endl;
		return;
	}

	_springs.erase(_springs.begin()+ idx);
}


void SpringSystem::delete_disconnected_vertices() {
	int i, j;
	bool ok;
	for (i=_vertices.size()- 1; i>=0; i--) {
		ok= false;
		for (j=0; j<_springs.size(); j++)
			if ((_springs[j]._idx1== i) || (_springs[j]._idx2== i)) {
				ok= true;
				break;
			}
		if (!ok)
			_vertices.erase(_vertices.begin()+ i);
	}
}


void SpringSystem::print() {
	cout << "_vertices.size()=" << _vertices.size() << " ; _springs.size()=" << _springs.size() << endl;
}


void SpringSystem::rand_contracts() {
	for (unsigned int i=0; i<_springs.size(); i++)
		_springs[i].rand_contract();
}


vector<unsigned int> SpringSystem::get_ajdacents_vertices(unsigned int idx) {
	vector<unsigned int> res;
	unsigned int i;
	for (i=0; i<_springs.size(); i++) {
		if (_springs[i]._idx1== idx)
			res.push_back(_springs[i]._idx2);
		if (_springs[i]._idx2== idx)
			res.push_back(_springs[i]._idx1);
	}
	return res;
}


vector<unsigned int> SpringSystem::get_ajdacents_springs(unsigned int idx) {
	vector<unsigned int> res;
	unsigned int i;
	for (i=0; i<_springs.size(); i++) {
		if ((_springs[i]._idx1== idx) || (_springs[i]._idx2== idx))
			res.push_back(i);
	}
	return res;
}


glm::vec3 SpringSystem::bary(bool is_init) {
	glm::vec3 res(0.0f);
	for (int idx_vertex=0; idx_vertex<_vertices.size(); ++idx_vertex) {
		if (is_init)
			res+= _vertices[idx_vertex]._p_init;
		else
			res+= _vertices[idx_vertex]._p;
	}
	res/= _vertices.size();

	return res;
}


float SpringSystem::fitness() {
	return glm::distance(bary(true), bary(false));
}


void SpringSystem::reinit_vertices() {
	for (unsigned int idx_vertex=0; idx_vertex<_vertices.size(); ++idx_vertex) {
		_vertices[idx_vertex].reinit();
	}
	_tik_init= SDL_GetTicks();
}


// méthodes abstraites appelées par les classes filles
void SpringSystem::init_vertices_springs() {
	cout << "appel a init_vertices_springs non valide" << endl;
}


void SpringSystem::rand_disposition(unsigned int n_cubes) {
	cout << "appel a rand_disposition non valide" << endl;
}


// ------------------------------------------------------------------------------------------
CubeSystem::CubeSystem() : _mass(VERTEX_MASS), _air_resist(VERTEX_AIR_RESIST), _spring_size(SPRING_SIZE),
	_stiffness(SPRING_STIFFNESS), _damping(SPRING_DAMPING) {
	SpringSystem();

}


void CubeSystem::init_vertices_springs() {
	float z0= 0.01f; // pour ne pas avoir de bug de z < 0

	vector< vector<int> > idxs_springs= {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{4, 5}, {5, 7}, {7, 6}, {6, 4},
		{0, 4}, {1, 5}, {3, 7}, {2, 6},
		{0, 6}, {2, 4},
		{1, 7}, {3, 5},
		{0, 3}, {1, 2},
		{5, 6}, {4, 7},
		{0, 5}, {1, 4},
		{3, 6}, {2, 7},
		{0, 7}, {3, 4},
		{1, 6}, {2, 5}
	};

	_vertices.clear();
	_springs.clear();

	for (unsigned int idx_cube=0; idx_cube<_cubes_idx.size(); ++idx_cube) {
		float x= float(_cubes_idx[idx_cube][0])* _spring_size;
		float y= float(_cubes_idx[idx_cube][1])* _spring_size;
		float z= float(_cubes_idx[idx_cube][2])* _spring_size;
		vector<int> idxs_vertex;
		for (unsigned int k=0; k<2; ++k)
			for (unsigned int j=0; j<2; ++j)
				for (unsigned int i=0; i<2; ++i) {
					glm::vec3 vertex= glm::vec3(x+ _spring_size* float(i), y+ _spring_size* float(j), z+ _spring_size* float(k)+ z0);
					int idx_vertex_ok= -1;
					for (int idx_vertex=0; idx_vertex<_vertices.size(); ++idx_vertex)
						if (glm::distance(vertex, _vertices[idx_vertex]._p)< SAME_VERTICES_TRESHOLD) {
							idx_vertex_ok= idx_vertex;
							break;
						}
					if (idx_vertex_ok< 0) {
						add_vertex(vertex, _mass, _air_resist);
						idx_vertex_ok= _vertices.size()- 1;
					}
					idxs_vertex.push_back(idx_vertex_ok);
				}

		for (unsigned int idx_spring=0; idx_spring<idxs_springs.size(); ++idx_spring) {
			unsigned int idx1= idxs_vertex[idxs_springs[idx_spring][0]];
			unsigned int idx2= idxs_vertex[idxs_springs[idx_spring][1]];
			bool already= false;
			for (unsigned int idx_spring=0; idx_spring<_springs.size(); ++idx_spring)
				if ( ((_springs[idx_spring]._idx1== idx1) && (_springs[idx_spring]._idx2== idx2)) ||
					 ((_springs[idx_spring]._idx1== idx2) && (_springs[idx_spring]._idx2== idx1)) ) {
					already= true;
					break;
				}
			if (!already)
				add_spring(idx1, idx2, _stiffness, _damping);
		}
	}

	//cout << _vertices.size() << ";" << _springs.size() << endl;
	/*for (int idx_vertex=0; idx_vertex<_vertices.size(); ++idx_vertex)
		cout << _vertices[idx_vertex]._p[0] << ";" << _vertices[idx_vertex]._p[1] << ";" << _vertices[idx_vertex]._p[2] << endl;
	for (unsigned int idx_spring=0; idx_spring<_springs.size(); ++idx_spring)
		cout << _springs[idx_spring]._idx1 << " ; " << _springs[idx_spring]._idx2 << endl;*/
}


void CubeSystem::rand_disposition(unsigned int n_cubes) {

	_cubes_idx.clear();
	vector<int> origin= {0, 0, 0};
	_cubes_idx.push_back(origin);

	for (unsigned int idx_iter=0; idx_iter<n_cubes- 1; ++idx_iter) {
		//cout << "idx_iter=" << idx_iter << endl;
		vector< vector<int> > v;
		for (unsigned int idx_cube=0; idx_cube<_cubes_idx.size(); ++idx_cube) {
			//cout << "idx_cube=" << idx_cube << endl;
			for (int i=-1; i<2; ++i)
				for (int j=-1; j<2; ++j)
					for (int k=-1; k<2; ++k) {
						if ((i* j!= 0) || (i* k!= 0) || (j* k!= 0))
							continue;
						if ((i== 0) && (j== 0) && (k== 0))
							continue;
						// pas de z négatif
						if (_cubes_idx[idx_cube][2]+ k< 0)
							continue;

						bool already_in_cubes= false;
						for (unsigned int idx_cube_2=0; idx_cube_2<_cubes_idx.size(); ++idx_cube_2) {
							if ((_cubes_idx[idx_cube_2][0]== _cubes_idx[idx_cube][0]+ i)
								&& (_cubes_idx[idx_cube_2][1]== _cubes_idx[idx_cube][1]+ j)
								&& (_cubes_idx[idx_cube_2][2]== _cubes_idx[idx_cube][2]+ k)) {
									already_in_cubes= true;
									break;
								}
						}
						if (already_in_cubes)
							continue;

						vector<int> w= {_cubes_idx[idx_cube][0]+ i, _cubes_idx[idx_cube][1]+ j, _cubes_idx[idx_cube][2]+ k};
						v.push_back(w);
					}
		}
		int n= rand_int(0, v.size()- 1);
		_cubes_idx.push_back(v[n]);
	}

	/*for (unsigned int idx_cube=0; idx_cube<_cubes_idx.size(); ++idx_cube) {
		cout << _cubes_idx[idx_cube][0] << " ; " << _cubes_idx[idx_cube][1] << " ; " << _cubes_idx[idx_cube][2] << endl;
	}*/

	init_vertices_springs();
}


// ------------------------------------------------------------------------------------------
SpringSystemGenetic::SpringSystemGenetic() {
	for (unsigned int i=0; i<N_POPULATION; i++) {
		CubeSystem* cs= new CubeSystem();
		cs->rand_disposition(N_CUBES);
		cs->rand_contracts();
		_sss.push_back(cs);
	}
}


void SpringSystemGenetic::next_gen() {
	int i, j;
	int idx1, idx2;
	float r;

	// anim
	for (i=0; i<N_POPULATION; i++) {
		_sss[i]->reinit_vertices();

		for (j=0; j<N_ANIM_PER_GEN; j++)
			_sss[i]->anim();
	}

	// tri
	sort(_sss.begin(), _sss.end(), sort_ss);

	// suppression de la moitié faible
	_sss.erase(_sss.begin()+ N_POPULATION/ 2, _sss.end());

	// croisement
	for (i=0; i<N_POPULATION/ 2; i++) {
		CubeSystem* cs= new CubeSystem();
		cs->rand_disposition(N_CUBES);
		cs->rand_contracts();

		idx1= rand_int(0, N_POPULATION/ 2- 1);
		idx2= rand_int(0, N_POPULATION/ 2- 1);

		for (j=0; j<cs->_springs.size()/ 2; j++) {
			cs->_springs[j]._contract_amount= _sss[idx1]->_springs[j]._contract_amount;
			cs->_springs[j]._contract_begin= _sss[idx1]->_springs[j]._contract_begin;
			cs->_springs[j]._contract_end= _sss[idx1]->_springs[j]._contract_end;
		}
		for (j=cs->_springs.size()/ 2; j<cs->_springs.size(); j++) {
			cs->_springs[j]._contract_amount= _sss[idx2]->_springs[j]._contract_amount;
			cs->_springs[j]._contract_begin= _sss[idx2]->_springs[j]._contract_begin;
			cs->_springs[j]._contract_end= _sss[idx2]->_springs[j]._contract_end;
		}

		_sss.push_back(cs);
	}

	// mutation
	// on épargne le meilleur
	for (i=1; i<_sss.size(); i++) {
		if (rand_float(0.0f, 1.0f)< 0.1f) {
			idx1= rand_int(0, _sss[i]->_springs.size()- 1);
			idx2= idx1+ rand_int(0, _sss[i]->_springs.size()- idx1- 1);
			for (j=idx1; j<=idx2; j++) {
				_sss[i]->_springs[j].rand_contract();
			}
		}
	}

	float dist_max= -1e8;
	int idx_max= -1;
	for (int idx=0; idx<_sss.size(); idx++) {
		float dist= _sss[idx]->fitness();
		//cout << dist << endl;
		if (dist> dist_max) {
			dist_max= dist;
			idx_max= idx;
		}
	}
	cout << "idx_max=" << idx_max << " ; dist_max=" << dist_max << endl;
	//cout << _sss.size() << endl;
}


void SpringSystemGenetic::save_best(std::string ch) {
	_sss[0]->save(ch);
}


// ------------------------------------------------------------------------------------------
SpringSystemGL::SpringSystemGL() {
}


SpringSystemGL::SpringSystemGL(GLuint prog_draw) :
	_is_draw_springs(true), _is_draw_forces(false), _is_draw_accel_speed(false), _prog_draw(prog_draw), _is_paused(false) {
	unsigned int i;

	for (i=0; i<MAX_DATA; i++)
		_data[i]= 0.;

	for (i=0; i<MAX_DATA_FORCES; i++)
		_data_forces[i]= 0.;

	for (i=0; i<MAX_DATA_ACCEL_SPEED; i++)
		_data_accel_speed[i]= 0.;

	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &_buffer_forces);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_forces);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_forces), _data_forces, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &_buffer_accel_speed);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_accel_speed);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_accel_speed), _data_accel_speed, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


void SpringSystemGL::draw(glm::mat4 world2clip) {
	glUseProgram(_prog_draw);
	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	if (_is_draw_springs) {
		glBindBuffer(GL_ARRAY_BUFFER, _buffer);
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
		glDrawArrays(GL_LINES, 0, _ss->_springs.size()* 2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (_is_draw_forces) {
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_forces);
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
		unsigned int n_forces= 0;
		for (unsigned int i=0; i<_ss->_vertices.size(); i++)
			n_forces+= _ss->_vertices[i]._forces.size();
		glDrawArrays(GL_LINES, 0, n_forces* 2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (_is_draw_accel_speed) {
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_accel_speed);
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
		glDrawArrays(GL_LINES, 0, _ss->_vertices.size()* 4);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
}


void SpringSystemGL::anim() {
	_ss->anim();
	update_data();
	update_data_forces();
	update_data_accel_speed();
}


void SpringSystemGL::update_data() {
	unsigned int i;
	for (i=0; i<MAX_DATA; i++)
		_data[i]= 0.;

	glm::vec3 data_color;
	for (i=0; i<_ss->_springs.size(); i++) {
		data_color= glm::vec3(1.0f, 1.0f, 1.0f);

		_data[i* 12+ 0]= _ss->_vertices[_ss->_springs[i]._idx1]._p.x;
		_data[i* 12+ 1]= _ss->_vertices[_ss->_springs[i]._idx1]._p.y;
		_data[i* 12+ 2]= _ss->_vertices[_ss->_springs[i]._idx1]._p.z;
		_data[i* 12+ 3]= data_color.x;
		_data[i* 12+ 4]= data_color.y;
		_data[i* 12+ 5]= data_color.z;
		_data[i* 12+ 6]= _ss->_vertices[_ss->_springs[i]._idx2]._p.x;
		_data[i* 12+ 7]= _ss->_vertices[_ss->_springs[i]._idx2]._p.y;
		_data[i* 12+ 8]= _ss->_vertices[_ss->_springs[i]._idx2]._p.z;
		_data[i* 12+ 9]= data_color.x;
		_data[i* 12+10]= data_color.y;
		_data[i* 12+11]= data_color.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void SpringSystemGL::update_data_forces() {
	unsigned int i, j, k;
	for (i=0; i<MAX_DATA_FORCES; i++)
		_data_forces[i]= 0.0f;

	const float EPS_DECAL= 1e-3; // pour mieux visualiser on décale

	k= 0;
	for (i=0; i<_ss->_vertices.size(); i++) {
		for (j=0; j<_ss->_vertices[i]._forces.size(); j++) {
			_data_forces[k* 12+ 0]= _ss->_vertices[i]._p.x+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 1]= _ss->_vertices[i]._p.y+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 2]= _ss->_vertices[i]._p.z+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 3]= _ss->_vertices[i]._forces[j]._color[0];
			_data_forces[k* 12+ 4]= _ss->_vertices[i]._forces[j]._color[1];
			_data_forces[k* 12+ 5]= _ss->_vertices[i]._forces[j]._color[2];
			_data_forces[k* 12+ 6]= _ss->_vertices[i]._p.x+ _ss->_vertices[i]._forces[j]._vec.x+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 7]= _ss->_vertices[i]._p.y+ _ss->_vertices[i]._forces[j]._vec.y+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 8]= _ss->_vertices[i]._p.z+ _ss->_vertices[i]._forces[j]._vec.z+ float(j)* EPS_DECAL;
			_data_forces[k* 12+ 9]= _ss->_vertices[i]._forces[j]._color[0];
			_data_forces[k* 12+10]= _ss->_vertices[i]._forces[j]._color[1];
			_data_forces[k* 12+11]= _ss->_vertices[i]._forces[j]._color[2];
			k++;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_forces);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_forces), _data_forces, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void SpringSystemGL::update_data_accel_speed() {
	unsigned int i, j, k;
	for (i=0; i<MAX_DATA_ACCEL_SPEED; i++)
		_data_accel_speed[i]= 0.;

	const float ACCEL_COLOR[3]= {0.5f, 0.0f, 0.0f};
	const float SPEED_COLOR[3]= {0.0f, 0.5f, 0.0f};
	const float EPS_DECAL= 1e-3; // pour mieux visualiser on décale

	for (i=0; i<_ss->_vertices.size(); i++) {
		_data_accel_speed[i* 24+ 0]= _ss->_vertices[i]._p.x- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 1]= _ss->_vertices[i]._p.y- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 2]= _ss->_vertices[i]._p.z- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 3]= ACCEL_COLOR[0];
		_data_accel_speed[i* 24+ 4]= ACCEL_COLOR[1];
		_data_accel_speed[i* 24+ 5]= ACCEL_COLOR[2];
		_data_accel_speed[i* 24+ 6]= _ss->_vertices[i]._p.x+ _ss->_vertices[i]._a.x- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 7]= _ss->_vertices[i]._p.y+ _ss->_vertices[i]._a.y- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 8]= _ss->_vertices[i]._p.z+ _ss->_vertices[i]._a.z- 1.* EPS_DECAL;
		_data_accel_speed[i* 24+ 9]= ACCEL_COLOR[0];
		_data_accel_speed[i* 24+10]= ACCEL_COLOR[1];
		_data_accel_speed[i* 24+11]= ACCEL_COLOR[2];

		_data_accel_speed[i* 24+12]= _ss->_vertices[i]._p.x- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+13]= _ss->_vertices[i]._p.y- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+14]= _ss->_vertices[i]._p.z- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+15]= SPEED_COLOR[0];
		_data_accel_speed[i* 24+16]= SPEED_COLOR[1];
		_data_accel_speed[i* 24+17]= SPEED_COLOR[2];
		_data_accel_speed[i* 24+18]= _ss->_vertices[i]._p.x+ _ss->_vertices[i]._v.x- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+19]= _ss->_vertices[i]._p.y+ _ss->_vertices[i]._v.y- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+20]= _ss->_vertices[i]._p.z+ _ss->_vertices[i]._v.z- 2.* EPS_DECAL;
		_data_accel_speed[i* 24+21]= SPEED_COLOR[0];
		_data_accel_speed[i* 24+22]= SPEED_COLOR[1];
		_data_accel_speed[i* 24+23]= SPEED_COLOR[2];
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_accel_speed);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_accel_speed), _data_accel_speed, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool SpringSystemGL::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_SPACE) {
		_ss->rand_disposition(N_CUBES);
		return true;
	}
	else if (key== SDLK_a) {
		_is_draw_accel_speed= !_is_draw_accel_speed;
		return true;
	}
	else if (key== SDLK_f) {
		_is_draw_forces= !_is_draw_forces;
		return true;
	}
	else if (key== SDLK_p) {
		_is_paused= !_is_paused;
		return true;
	}
	else if (key== SDLK_s) {
		_is_draw_springs= !_is_draw_springs;
		return true;
	}
	else if (key== SDLK_z) {
		_ss->rand_contracts();
		return true;
	}

	return false;
}
