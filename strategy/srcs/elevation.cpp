#include "utile.h"

#include "elevation.h"


Elevation::Elevation() {

}


Elevation::Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) :
	_origin(origin), _size(size), _n_ligs(n_ligs), _n_cols(n_cols) 
{
	_resolution = pt_2d(_size.x / (number)(_n_cols- 1), _size.y / (number)(_n_ligs- 1));

	_altis = new number[_n_ligs * _n_cols];
	for (uint i=0; i<_n_ligs * _n_cols; ++i) {
		_altis[i] = 0.0;
	}
}


Elevation::~Elevation() {
	delete _altis;
}


bool Elevation::in_boundaries(int col, int lig) {
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		return false;
	}
	return true;
}


bool Elevation::in_boundaries(pt_2d pt) {
	if (pt.x < _origin.x || pt.x > _origin.x + _size.x || pt.y < _origin.y || pt.y > _origin.y + _size.y) {
		return false;
	}
	return true;
}


std::pair<uint, uint> Elevation::id2col_lig(uint id) {
	return std::make_pair(id % _n_cols, id/ _n_cols);
}


uint Elevation::col_lig2id(uint col, uint lig) {
	return col+ _n_cols* lig;
}


pt_2d Elevation::col_lig2pt(uint col, uint lig) {
	return pt_2d(
		_origin.x+ (number)(col) * _resolution.x,
		_origin.y+ (number)(lig) * _resolution.y
	);
}


pt_2d Elevation::id2pt_2d(uint id) {
	std::pair<uint, uint> col_lig = id2col_lig(id);
	return col_lig2pt(col_lig.first, col_lig.second);
}


pt_3d Elevation::id2pt_3d(uint id) {
	pt_2d pt = id2pt_2d(id);
	return pt_3d(pt.x, pt.y, get_alti(id));
}


std::pair<uint, uint> Elevation::pt2col_lig(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "Elevation::pt2col_lig : " << glm_to_string(pt) << " hors Elevation\n";
		return std::make_pair(0, 0);
	}
	int col= (int)((pt.x- _origin.x) / _resolution.x);
	int lig= (int)((pt.y- _origin.y) / _resolution.y);
	return std::make_pair(col, lig);
}


uint Elevation::pt2id(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	return col_lig2id(col_lig.first, col_lig.second);
}


number Elevation::get_alti(uint id) {
	return _altis[id];
}


number Elevation::get_alti(int col, int lig) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "Elevation::get_alti : (" << col << " ; " << lig << ") hors Elevation (2!)\n";
		return 0.0;
	}
	return _altis[col_lig2id(col, lig)];
}


number Elevation::get_alti(pt_2d pt) {
	//std::pair<uint, uint> col_lig = pt2col_lig(pt);
	//return get_alti(col_lig.first, col_lig.second);
	
	if (!in_boundaries(pt)) {
		std::cerr << "Elevation::get_alti : " << glm_to_string(pt) << " hors Elevation\n";
		return 0.0;
	}
	int col_left= (int)((pt.x- _origin.x) / _resolution.x);
	int lig_bottom= (int)((pt.y- _origin.y) / _resolution.y);
	
	number col_mod = fmod(pt.x- _origin.x, _resolution.x) / _resolution.x;
	number lig_mod = fmod(pt.y- _origin.y, _resolution.y) / _resolution.y;
	number alti_left_bottom, alti_left_top, alti_right_bottom, alti_right_top, alti_left, alti_right;
	
	alti_left_bottom = get_alti(col_left, lig_bottom);
	
	if (lig_bottom == _n_ligs - 1) {
		alti_left_top = alti_left_bottom;
	}
	else {
		alti_left_top = get_alti(col_left, lig_bottom + 1);
	}
	if (col_left == _n_cols - 1) {
		alti_right_bottom = alti_left_bottom;
	}
	else {
		alti_right_bottom = get_alti(col_left + 1, lig_bottom);
	}
	if (lig_bottom == _n_ligs - 1) {
		alti_right_top = alti_right_bottom;
	}
	else if (col_left == _n_cols - 1) {
		alti_right_top = alti_left_top;
	}
	else {
		alti_right_top = get_alti(col_left + 1, lig_bottom + 1);
	}

	alti_left = alti_left_bottom * (1.0 - lig_mod) + alti_left_top * lig_mod;
	alti_right = alti_right_bottom * (1.0 - lig_mod) + alti_right_top * lig_mod;

	return alti_left * (1.0 - col_mod) + alti_right * col_mod;
	//return lig_mod;
}


number Elevation::get_alti_over_polygon(Polygon2D * polygon) {
	number result = 0.0;
	uint n_pts = 0;
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				n_pts++;
				result += get_alti(col, lig);
			}
		}
	}
	if (n_pts > 0) {
		result /= number(n_pts);
		return result;
	}
	else {
		std::cerr << "Elevation::get_alti_over_polygon pas de pt dans polygon\n";
		return 0.0;
	}
}


std::vector<uint> Elevation::get_ids_over_aabb(AABB_2D * aabb) {
	std::vector<uint> result;
	std::pair<uint, uint> col_lig_min = pt2col_lig(aabb->_pos);
	std::pair<uint, uint> col_lig_max = pt2col_lig(aabb->_pos + aabb->_size);
	for (uint col = col_lig_min.first; col<= col_lig_max.first; ++col) {
		for (uint lig = col_lig_min.second; lig<= col_lig_max.second; ++lig) {
			result.push_back(col_lig2id(col, lig));
		}
	}
	return result;
}


std::vector<uint> Elevation::get_neighbors(uint id) {
	std::vector<uint> result;
	std::pair<uint, uint> col_lig = id2col_lig(id);
	uint col = col_lig.first;
	uint lig = col_lig.second;
	if (col > 0) {
		result.push_back(col_lig2id(col - 1, lig));
		if (lig > 0) {
			result.push_back(col_lig2id(col - 1, lig - 1));
		}
		if (lig < _n_ligs - 1) {
			result.push_back(col_lig2id(col - 1, lig + 1));
		}
	}
	if (col < _n_cols - 1) {
		result.push_back(col_lig2id(col + 1, lig));
		if (lig > 0) {
			result.push_back(col_lig2id(col + 1, lig - 1));
		}
		if (lig < _n_ligs - 1) {
			result.push_back(col_lig2id(col + 1, lig + 1));
		}
	}
	if (lig > 0) {
		result.push_back(col_lig2id(col, lig - 1));
	}
	if (lig < _n_ligs - 1) {
		result.push_back(col_lig2id(col, lig + 1));
	}

	std::sort(result.begin(), result.end());

	return result;
}


pt_3d Elevation::get_normal(uint id) {
	std::pair<uint, uint> col_lig = id2col_lig(id);
	uint col = col_lig.first;
	uint lig = col_lig.second;
	std::vector<std::tuple<uint, uint, uint> > triangles;

	if (col > 0) {
		if (lig > 0) {
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig), col_lig2id(col - 1, lig - 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig - 1), col_lig2id(col, lig - 1), id));
		}
		if (lig < _n_ligs - 1) {
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig + 1), col_lig2id(col - 1, lig), id));
			triangles.push_back(std::make_tuple(col_lig2id(col, lig + 1), col_lig2id(col - 1, lig + 1), id));
		}
	}
	if (col < _n_cols - 1) {
		if (lig > 0) {
			triangles.push_back(std::make_tuple(col_lig2id(col, lig - 1), col_lig2id(col + 1, lig - 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig - 1), col_lig2id(col + 1, lig), id));
		}
		if (lig < _n_ligs - 1) {
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig), col_lig2id(col + 1, lig + 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig + 1), id));
		}
	}

	pt_3d result(0.0);
	for (auto & triangle : triangles) {
		pt_3d u = glm::normalize(id2pt_3d(std::get<1>(triangle)) - id2pt_3d(std::get<0>(triangle)));
		pt_3d v = glm::normalize(id2pt_3d(std::get<2>(triangle)) - id2pt_3d(std::get<0>(triangle)));
		result += glm::cross(u, v);
	}
	result = glm::normalize(result);
	
	return result;
}


std::vector<uint> Elevation::lowest_gradient(uint id_src) {
	std::vector<uint> result;
	uint id_current = id_src;
	
	while (true) {
		result.push_back(id_current);
		number alti_current = get_alti(id_current);
		std::vector<uint> neighbors = get_neighbors(id_current);
		number alti_min = 1e7;
		uint id_next = 0;
		for (auto & id : neighbors) {
			if (std::find(result.begin(), result.end(), id) != result.end()) {
				continue;
			}
			number alti_id = get_alti(id);
			if (alti_id < alti_min) {
				alti_min = alti_id;
				id_next = id;
			}
		}
		
		if (alti_min <= 0.0) {
			result.push_back(id_next);
			break;
		}

		if (alti_min > alti_current) {
			break;
		}

		//std::cout << id_current << " ; " << alti_current << " ; " << id_next << " ; " << alti_min << "\n";
		id_current = id_next;
	}

	return result;
}


void Elevation::set_alti(int col, int lig, number alti) {
	_altis[col_lig2id(col, lig)] = alti;
}


void Elevation::set_alti_over_polygon(Polygon2D * polygon, number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				set_alti(col, lig, alti);
			}
		}
	}
}


void Elevation::set_alti_all(number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			set_alti(col, lig, alti);
		}
	}
}


void Elevation::set_negative_alti_2zero() {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			if (_altis[id]< 0.0) {
				_altis[id] = 0.0;
			}
		}
	}
}


void Elevation::randomize() {
	// https://www.redblobgames.com/maps/Elevation-from-noise/
	// https://gamedev.stackexchange.com/questions/116205/terracing-mountain-features/188513

	number alti_offset = -5.0;
	uint n_levels = 5;
	uint gradient_base_size = 10;
	number max_factor = 20.0;
	number redistribution_power = 0.8;
	number fudge_factor = 1.5;
	number mix_island = 0.4;
	number island_max_alti = 20.0;
	number terrace_factor = 0.5;
	number terrace_hmin = 1.0;
	number terrace_hmax = 4.0;
	number terrace_perlin_factor = 20.0;
	uint terrace_gradient_w= 4;
	uint terrace_gradient_h= 4;

	//std::vector<number> amplitudes {1.0, 0.5, 0.25, 0.12, 0.06};
	std::vector<number> amplitudes {1.0, 0.5, 0.33, 0.25, 0.2};
	number amp_sum = 0.0;
	for (auto & a : amplitudes) {
		amp_sum += a;
	}

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 1000.0);
			//pt_2d pt = col_lig2pt(col, lig);
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 100.0) * exp(-1.0 * pow(glm::distance(pt, _origin + 0.5 * _size) / (0.5 * _size.x), 2.0));
			//_altis[col_lig2id(col, lig)] = perlin(col, lig, gradient, gradient_w, gradient_h);
			//number p = perlin(col, lig, gradient, gradient_w, gradient_h);
			//std::cout << col << " ; " << lig << " ; " << p << "\n";

			_altis[col_lig2id(col, lig)] = alti_offset;
		}
	}

	for (uint level=0; level<n_levels; ++level) {
		srand(time(NULL));

		uint gradient_w= gradient_base_size* (level+ 1);
		uint gradient_h= gradient_base_size* (level+ 1);
		number * gradient = perlin_gradient(gradient_w, gradient_h);
		//number factor= max_factor* pow(2.0, -1.0 * number(level)) / amp_sum;
		number factor = max_factor* amplitudes[level] / amp_sum;
	
		for (uint col=0; col< _n_cols; ++col) {
			for (uint lig=0; lig< _n_ligs; ++lig) {
				number ii= number(col)* (gradient_w- 1)/ _n_cols;
				number jj= number(lig)* (gradient_h- 1)/ _n_ligs;
				_altis[col_lig2id(col, lig)] += factor* perlin(ii, jj, gradient, gradient_w, gradient_h);
			}
		}
	}

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			if (_altis[id]> 0.0) {
				_altis[id] = pow(_altis[id] * fudge_factor, redistribution_power);
			}
		}
	}
	

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			number nx = 2.0 * number(col) / _n_cols - 1.0;
			number ny = 2.0 * number(lig) / _n_ligs - 1.0;
			number d = 1.0 - (1.0 - nx * nx) * (1.0 - ny * ny);
			_altis[id] = (1.0 - mix_island) * _altis[id] + mix_island * (1.0 - d) * island_max_alti;
		}
	}

	number * gradient = perlin_gradient(terrace_gradient_w, terrace_gradient_h);
	//number damp = 0.9;
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			number ii= number(col)* (terrace_gradient_w- 1)/ _n_cols;
			number jj= number(lig)* (terrace_gradient_h- 1)/ _n_ligs;
			number hm = terrace_perlin_factor * perlin(ii, jj, gradient, terrace_gradient_w, terrace_gradient_h);
			
			/*if (h1 + hm < _altis[id] && h2 + hm > _altis[id]) {
				_altis[id] *= damp;
			}
			else if (h2 + hm < _altis[id]) {
				_altis[id] -= (h2 - h1) * damp;
			}*/

			/*number k = floor(_altis[id] / terrace_factor);
			number f = (_altis[id] - k * terrace_factor) / terrace_factor;
			number s = std::min(2.0 * f, 1.0);
			if (terrace_hmin + hm < _altis[id] && terrace_hmax + hm > _altis[id]) {
				_altis[id] = (k + s) * terrace_factor;
			}*/
			if (terrace_hmin + hm < _altis[id] && terrace_hmax + hm > _altis[id]) {
				_altis[id] = round(_altis[id] * 2.0) / 2.0;
			}
		}
	}
	delete gradient;

	set_negative_alti_2zero();

	//alti2pbm("../data/test.pgm");
}


void Elevation::alti2pbm(std::string pbm_path) {
	FILE *f;
	f= fopen(pbm_path.c_str(), "wb");
	//fprintf(f, "P1\n%d %d\n", _n_cols, _n_ligs);
	fprintf(f, "P2\n%d %d\n1\n", _n_cols, _n_ligs);
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			int v = 0;
			if (get_alti(col, lig) < 0.1) {
				v = 1;
			}
			fprintf(f, "%d ", v);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

