#include "utile.h"

#include "elevation.h"


Elevation::Elevation() {

}


Elevation::Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) : GraphGrid(origin, size, n_ligs, n_cols)
{
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		ElevationVertexData * data = new ElevationVertexData();
		data->_normal = pt_3d(0.0);
		_it_v->second._data = data;
		_it_v++;
	}

	_n_pts = 6 * (_n_ligs - 1) * (_n_cols - 1);
	_n_attrs_per_pts = 10;
	_data = new float[_n_pts * _n_attrs_per_pts];
	//std::cout << _n_pts * _n_attrs_per_pts << "\n";
}


Elevation::~Elevation() {
	delete[] _data;
}


number Elevation::get_alti(uint id) {
	if (!in_boundaries(id)) {
		std::cerr << "Elevation::get_alti : " << id << " hors grille\n";
		return 0.0;
	}
	return _vertices[id]._pos.z;
}


number Elevation::get_alti(int col, int lig) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "Elevation::get_alti : (" << col << " ; " << lig << ") hors grille\n";
		return 0.0;
	}
	return _vertices[col_lig2id(col, lig)]._pos.z;
}


number Elevation::get_alti(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "Elevation::get_alti : " << glm_to_string(pt) << " hors grille\n";
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
}


number Elevation::get_alti_over_polygon(Polygon2D * polygon) {
	number result = 0.0;
	uint n_pts = 0;
	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt_2d(col, lig);
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


number Elevation::get_max_alti_along_segment(pt_2d pt1, pt_2d pt2) {
	const number step = std::max(_resolution.x, _resolution.y);
	number dist = glm::length(pt1 - pt2);
	if (dist <= step) {
		return get_alti(0.5 * (pt1 + pt2));
	}
	pt_2d direction = (pt2 - pt1) / dist;
	uint n_steps = uint(dist / step) + 1;
	number result = -1e9;
	for (uint i = 0; i<n_steps; ++i) {
		number alti = get_alti(pt1 + number(i) * step * direction);
		if (alti > result) {
			result = alti;
		}
	}
	return result;
}


pt_3d Elevation::compute_normal(uint id) {
	std::vector<int_pair > ids;
	int_pair col_lig = id2col_lig(id);
	int col = col_lig.first;
	int lig = col_lig.second;

	if (col > 0) {
		if (lig > 0) {
			ids.push_back(std::make_pair(col_lig2id(col - 1, lig), col_lig2id(col - 1, lig - 1)));
			ids.push_back(std::make_pair(col_lig2id(col - 1, lig - 1), col_lig2id(col, lig - 1)));
		}
		if (lig < _n_ligs - 1) {
			ids.push_back(std::make_pair(col_lig2id(col - 1, lig + 1), col_lig2id(col - 1, lig)));
			ids.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col - 1, lig + 1)));
		}
	}
	if (col < _n_cols - 1) {
		if (lig > 0) {
			ids.push_back(std::make_pair(col_lig2id(col, lig - 1), col_lig2id(col + 1, lig - 1)));
			ids.push_back(std::make_pair(col_lig2id(col + 1, lig - 1), col_lig2id(col + 1, lig)));
		}
		if (lig < _n_ligs - 1) {
			ids.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col + 1, lig + 1)));
			ids.push_back(std::make_pair(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig + 1)));
		}
	}

	pt_3d pt = col_lig2pt_3d(col, lig);
	pt_3d result(0.0);
	for (auto & id_pair : ids) {
		pt_3d u = glm::normalize(id2pt_3d(id_pair.first) - pt);
		pt_3d v = glm::normalize(id2pt_3d(id_pair.second) - pt);
		result += glm::cross(u, v);
	}
	result = glm::normalize(result);
	
	return result;
}


pt_3d Elevation::get_normal(uint id) {
	ElevationVertexData * data = (ElevationVertexData *)(_vertices[id]._data);
	return data->_normal;
}


pt_3d Elevation::get_normal(int col, int lig) {
	return get_normal(col_lig2id(col, lig));
}


void Elevation::update_normal(uint id) {
	ElevationVertexData * data = (ElevationVertexData *)(_vertices[id]._data);
	data->_normal = compute_normal(id);
}


void Elevation::update_normal(int col, int lig) {
	update_normal(col_lig2id(col, lig));
}


void Elevation::update_normals() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		update_normal(_it_v->first);
		_it_v++;
	}
}


void Elevation::update_normals(int col_min, int col_max, int lig_min, int lig_max) {
	for (int col=col_min; col<=col_max; ++col) {
		for (int lig=lig_min; lig<=lig_max; ++lig) {
			update_normal(col_lig2id(col, lig));
		}
	}
}


void Elevation::update_normals(AABB_2D * aabb) {
	std::pair<int_pair, int_pair> col_lig_min_max = aabb2col_lig_min_max(aabb);
	uint col_min = col_lig_min_max.first.first;
	uint lig_min = col_lig_min_max.first.second;
	uint col_max = col_lig_min_max.second.first;
	uint lig_max = col_lig_min_max.second.second;
	update_normals(col_min, col_max, lig_min, lig_max);
}


std::vector<uint> Elevation::lowest_gradient(uint id_src) {
	std::vector<uint> result;
	uint id_current = id_src;
	
	while (true) {
		result.push_back(id_current);
		number alti_current = get_alti(id_current);
		std::vector<uint> l_neighbors = neighbors(id_current);
		number alti_min = 1e7;
		uint id_next = 0;
		for (auto & id : l_neighbors) {
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


uint Elevation::lowest_neighbor(uint id) {
	std::vector<uint> l_neighbors = neighbors(id);
	number alti_min = 1e7;
	uint result = 0;
	for (auto & id_n : l_neighbors) {
		number alti_n = get_alti(id_n);
		if (alti_n < alti_min) {
			alti_min = alti_n;
			result = id_n;
		}
	}
	return result;
}


void Elevation::set_alti(uint id, number alti) {
	_vertices[id]._pos.z = alti;
	//update_normal(id);
}


void Elevation::set_alti(int col, int lig, number alti) {
	set_alti(col_lig2id(col, lig), alti);
}


void Elevation::set_alti_over_polygon(Polygon2D * polygon, number alti) {
	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt_2d(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				set_alti(col, lig, alti);
			}
		}
	}
}


void Elevation::set_alti_all(number alti) {
	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			set_alti(col, lig, alti);
		}
	}
	update_normals();
	update_data();
}


void Elevation::set_negative_alti_2zero() {
	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			if (get_alti(col, lig) < 0.0) {
				set_alti(col, lig, 0.0);
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
	number max_factor = 40.0;
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

	std::vector<number> amplitudes {1.0, 0.5, 0.25, 0.12, 0.06};
	//std::vector<number> amplitudes {1.0, 0.5, 0.33, 0.25, 0.2};
	number amp_sum = 0.0;
	for (auto & a : amplitudes) {
		amp_sum += a;
	}

	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 1000.0);
			//pt_2d pt = col_lig2pt(col, lig);
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 100.0) * exp(-1.0 * pow(glm::distance(pt, _origin + 0.5 * _size) / (0.5 * _size.x), 2.0));
			//_altis[col_lig2id(col, lig)] = perlin(col, lig, gradient, gradient_w, gradient_h);
			//number p = perlin(col, lig, gradient, gradient_w, gradient_h);
			//std::cout << col << " ; " << lig << " ; " << p << "\n";

			//_altis[col_lig2id(col, lig)] = alti_offset;
			set_alti(col, lig, alti_offset);
		}
	}

	for (uint level=0; level<n_levels; ++level) {
		srand(time(NULL));

		uint gradient_w= gradient_base_size* (level+ 1);
		uint gradient_h= gradient_base_size* (level+ 1);
		number * gradient = perlin_gradient(gradient_w, gradient_h);
		//number factor= max_factor* pow(2.0, -1.0 * number(level)) / amp_sum;
		number factor = max_factor* amplitudes[level] / amp_sum;
	
		for (int col=0; col< _n_cols; ++col) {
			for (int lig=0; lig< _n_ligs; ++lig) {
				number ii= number(col)* (gradient_w- 1)/ _n_cols;
				number jj= number(lig)* (gradient_h- 1)/ _n_ligs;
				//_altis[col_lig2id(col, lig)] += factor* perlin(ii, jj, gradient, gradient_w, gradient_h);
				set_alti(col, lig, get_alti(col, lig) + factor* perlin(ii, jj, gradient, gradient_w, gradient_h));
			}
		}
	}

	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			//uint id = col_lig2id(col, lig);
			/*if (_altis[id]> 0.0) {
				_altis[id] = pow(_altis[id] * fudge_factor, redistribution_power);
			}*/
			if (get_alti(col, lig) > 0.0) {
				set_alti(col, lig, pow(get_alti(col, lig) * fudge_factor, redistribution_power));
			}
		}
	}
	

	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			//uint id = col_lig2id(col, lig);
			number nx = 2.0 * number(col) / _n_cols - 1.0;
			number ny = 2.0 * number(lig) / _n_ligs - 1.0;
			number d = 1.0 - (1.0 - nx * nx) * (1.0 - ny * ny);
			//_altis[id] = (1.0 - mix_island) * _altis[id] + mix_island * (1.0 - d) * island_max_alti;
			set_alti(col, lig, (1.0 - mix_island) * get_alti(col, lig) + mix_island * (1.0 - d) * island_max_alti);
		}
	}

	number * gradient = perlin_gradient(terrace_gradient_w, terrace_gradient_h);
	//number damp = 0.9;
	for (int col=0; col< _n_cols; ++col) {
		for (int lig=0; lig< _n_ligs; ++lig) {
			//uint id = col_lig2id(col, lig);
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
			
			/*if (terrace_hmin + hm < _altis[id] && terrace_hmax + hm > _altis[id]) {
				_altis[id] = round(_altis[id] * 2.0) / 2.0;
			}*/
			
			/*if (terrace_hmin + hm < get_alti(col, lig) && terrace_hmax + hm > get_alti(col, lig)) {
				set_alti(col, lig, round(get_alti(col, lig) * 2.0) / 2.0);
			}*/
		}
	}
	delete gradient;

	//set_negative_alti_2zero();

	update_normals();
	update_data();

	//alti2pbm("../data/test.pgm");
}


glm::vec4 Elevation::alti2color(number alti) {
	if (alti < 0.01) {
		return glm::vec4(0.4f, 0.5f, 1.0f, 1.0f);
	}
	else if (alti < 0.3) {
		return glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (alti > 7.0) {
		return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else  {
		//return glm::vec4(0.4f, 1.0f, 0.3f, 1.0f);
		//return glm::vec4(0.1f + rand_float(0.0f, 0.3f), 0.7f + rand_float(0.0f, 0.3f), 0.0f + rand_float(0.0f, 0.3f), 1.0f);
		return glm::vec4(0.4f - float(alti) * 0.02f, 1.0f - float(alti) * 0.05f, 0.3f + float(alti) * 0.02f, 1.0f);
	}
}


void Elevation::update_data() {
	update_data(0, int(_n_cols - 1), 0, int(_n_ligs - 1));
}


void Elevation::update_data(int col_min, int col_max, int lig_min, int lig_max) {
	//std::cout << col_min << " ; " << col_max << " ; " << lig_min << " ; " << lig_max << "\n";
	uint idx_tris[6] = {0, 1, 2, 0, 2, 3};
	
	for (int lig = lig_min; lig < lig_max; ++lig) {
		for (int col = col_min; col < col_max; ++col) {
			// attention on ne peut pas utiliser col_lig2id ici
			float * ptr = _data + 60 * ((_n_cols - 1) * lig + col);
			
			pt_3d pts[4] = {
				col_lig2pt_3d(col, lig),
				col_lig2pt_3d(col + 1, lig),
				col_lig2pt_3d(col + 1, lig + 1),
				col_lig2pt_3d(col, lig + 1)
			};

			pt_3d normals[4] = {
				get_normal(col, lig),
				get_normal(col + 1, lig),
				get_normal(col + 1, lig + 1),
				get_normal(col, lig + 1)
			};
			

			for (uint i=0; i<6; ++i) {
				number alti = pts[idx_tris[i]].z;
				glm::vec4 color = alti2color(alti);

				ptr[0] = float(pts[idx_tris[i]].x);
				ptr[1] = float(pts[idx_tris[i]].y);
				ptr[2] = float(pts[idx_tris[i]].z);
				ptr[3] = color.r;
				ptr[4] = color.g;
				ptr[5] = color.b;
				ptr[6] = color.a;
				ptr[7] = float(normals[idx_tris[i]].x);
				ptr[8] = float(normals[idx_tris[i]].y);
				ptr[9] = float(normals[idx_tris[i]].z);

				ptr += 10;
			}
		}
	}
}


void Elevation::update_data(AABB_2D * aabb) {
	std::pair<int_pair, int_pair> col_lig_min_max = aabb2col_lig_min_max(aabb);
	uint col_min = col_lig_min_max.first.first;
	uint lig_min = col_lig_min_max.first.second;
	uint col_max = col_lig_min_max.second.first;
	uint lig_max = col_lig_min_max.second.second;
	update_data(col_min, col_max, lig_min, lig_max);
}


void Elevation::alti2pbm(std::string pbm_path) {
	FILE *f;
	f= fopen(pbm_path.c_str(), "wb");
	//fprintf(f, "P1\n%d %d\n", _n_cols, _n_ligs);
	fprintf(f, "P2\n%d %d\n1\n", _n_cols, _n_ligs);
	for (int lig=0; lig<_n_ligs; ++lig) {
		for (int col=0; col<_n_cols; ++col) {
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

