#include <queue>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>

#include "path_find.h"
#include "utile.h"
#include "shapefile.h"


using namespace std;



// ----------------------------------------------------------------------------------------
GraphGrid::GraphGrid() {

}


GraphGrid::GraphGrid(unsigned int n_ligs, unsigned int n_cols, const glm::vec2 & origin, const glm::vec2 & size, bool is8connex) : _n_ligs(n_ligs), _n_cols(n_cols), _origin(origin), _size(size) {
	for (unsigned int lig=0; lig<_n_ligs; ++lig) {
		for (unsigned int col=0; col<_n_cols; ++col) {
			unsigned int id= col_lig2id(col, lig);
			//float weight= rand_float(1.0f, 10.0f);
			float weight= 1.0f;
			/*if (rand_bool()) {
				weight= 10.0f;
			}*/
			float x= _origin.x+ ((float)(col)/ (float)(_n_cols))* _size.x;
			float y= _origin.y+ ((float)(lig)/ (float)(_n_ligs))* _size.y;
			add_vertex(id, glm::vec3(x, y, 0.0f), weight);
		}
	}
	
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		unsigned int col= id2col_lig(_it_v->first).first;
		unsigned int lig= id2col_lig(_it_v->first).second;
		if (col> 0) {
			add_edge(_it_v->first, _it_v->first- 1);
		}
		if (col< _n_cols- 1) {
			add_edge(_it_v->first, _it_v->first+ 1);
		}
		if (lig> 0) {
			add_edge(_it_v->first, _it_v->first- _n_cols);
		}
		if (lig< _n_ligs- 1) {
			add_edge(_it_v->first, _it_v->first+ _n_cols);
		}

		if (is8connex) {
			if ((col> 0) && (lig> 0)) {
				add_edge(_it_v->first, _it_v->first- 1- _n_cols);
			}
			if ((col< _n_cols- 1) && (lig> 0)) {
				add_edge(_it_v->first, _it_v->first+ 1- _n_cols);
			}
			if ((col> 0) && (lig< _n_ligs- 1)) {
				add_edge(_it_v->first, _it_v->first- 1+ _n_cols);
			}
			if ((col< _n_cols- 1) && (lig< _n_ligs- 1)) {
				add_edge(_it_v->first, _it_v->first+ 1+ _n_cols);
			}
		}

		_it_v++;
	}

	_aabb= new AABB_2D(_origin, _size);
}


GraphGrid::~GraphGrid() {
	delete _aabb;
}


pair<unsigned int, unsigned int> GraphGrid::id2col_lig(unsigned int id) {
	return make_pair(id % _n_cols, id/ _n_cols);
}


unsigned int GraphGrid::col_lig2id(unsigned int col, unsigned int lig) {
	return col+ _n_cols* lig;
}


void GraphGrid::set_heavy_weight(AABB_2D * aabb) {
	int col_min= (int)((aabb->_pos.x- _origin.x)* (float)(_n_cols)/ _size.x);
	int col_max= (int)((aabb->_pos.x+ aabb->_size.x- _origin.x)* (float)(_n_cols)/ _size.x);
	int lig_min= (int)((aabb->_pos.y- _origin.y)* (float)(_n_ligs)/ _size.y);
	int lig_max= (int)((aabb->_pos.y+ aabb->_size.y- _origin.y)* (float)(_n_ligs)/ _size.y);

	for (int col=col_min; col<=col_max; ++col) {
		for (int lig=lig_min; lig<=lig_max; ++lig) {
			if ((col< 0) || (col>= _n_cols) || (lig< 0) || (lig>= _n_ligs)) {
				continue;
			}
			_vertices[col_lig2id(col, lig)]._weight= 10.0f;
			//_vertices[col_lig2id(col, lig)]._active= false;
		}
	}
}


ostream & operator << (ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


// ----------------------------------------------------------------------------------------
bool frontier_cmp(pair<unsigned int, float> x, pair<unsigned int, float> y) {
	return x.second> y.second;
}


PathFinder::PathFinder() {

}


PathFinder::PathFinder(unsigned int n_ligs, unsigned int n_cols, const glm::vec2 & origin, const glm::vec2 & size, bool is8connex) {
	_grid= new GraphGrid(n_ligs, n_cols, origin, size, is8connex);
}


PathFinder::~PathFinder() {
	delete _grid;
}


void PathFinder::update_grid() {
	vector<unsigned int> vertices_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		for (auto poly : _polygons) {
			glm::vec2 v(_grid->_it_v->second._pos);
			if (is_pt_inside_poly(v, poly)) {
			//if (distance_poly_pt(poly, _grid->_it_v->second._pos, NULL)< 0.1f) {
				vertices_to_erase.push_back(_grid->_it_v->first);
				break;
			}
		}
		_grid->_it_v++;
	}
	for (auto it_erase : vertices_to_erase) {
		_grid->remove_vertex(it_erase);
	}

	vector<pair<unsigned int, unsigned int> > edges_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			glm::vec2 pt_begin= _grid->_it_v->second._pos;
			glm::vec2 pt_end= _grid->_vertices[_grid->_it_e->first]._pos;
			for (auto poly : _polygons) {
				if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
					edges_to_erase.push_back(make_pair(_grid->_it_v->first, _grid->_it_e->first));
					break;
				}
			}
			_grid->_it_e++;
		}
		_grid->_it_v++;
	}
	for (auto it_erase : edges_to_erase) {
		_grid->remove_edge(it_erase.first, it_erase.second);
	}
}


void PathFinder::read_shapefile(string shp_path, glm::vec2 origin, glm::vec2 size, bool reverse_y) {
	vector<Polygon2D *> polygons;
	read_shp(shp_path, polygons);
	for (auto poly : polygons) {
		Polygon2D * poly_reproj= new Polygon2D();
		float pts[poly->_pts.size()* 2];
		for (unsigned int i=0; i<poly->_pts.size(); ++i) {
			pts[2* i]= ((poly->_pts[i].x- origin.x)/ size.x)* _grid->_size.x+ _grid->_origin.x;
			if (reverse_y) {
				pts[2* i+ 1]= ((origin.y- poly->_pts[i].y)/ size.y)* _grid->_size.y+ _grid->_origin.y;
			}
			else {
				pts[2* i+ 1]= ((poly->_pts[i].y- origin.y)/ size.y)* _grid->_size.y+ _grid->_origin.y;
			}
		}
		poly_reproj->set_points(pts, poly->_pts.size());
		//poly_reproj->print();
		_polygons.push_back(poly_reproj);
		delete poly;
	}
}


void PathFinder::rand(unsigned int n_polys, unsigned int n_pts_per_poly, float poly_radius) {
	for (unsigned int i=0; i<n_polys; ++i) {
		Polygon2D * poly= new Polygon2D();
		float x= rand_float(_grid->_origin.x, _grid->_origin.x+ _grid->_size.x);
		float y= rand_float(_grid->_origin.y, _grid->_origin.y+ _grid->_size.y);
		poly->randomize(n_pts_per_poly, poly_radius, glm::vec2(x, y));
		_polygons.push_back(poly);
	}

	update_grid();
}


float PathFinder::cost(unsigned int i, unsigned int j) {
	//return _grid->_vertices[i]._edges[j]._weight;
	// theta * empeche d'utiliser edge._weight car i et j ne sont pas forcement voisins
	//return glm::distance(_grid->_vertices[i]._pos, _grid->_vertices[j]._pos)+ _grid->_vertices[j]._weight;
	return _grid->_vertices[j]._weight;
}


float PathFinder::heuristic(unsigned int i, unsigned int j) {
	//return abs(_grid->_vertices[i]._pos.x- _grid->_vertices[j]._pos.x)+ abs(_grid->_vertices[i]._pos.y- _grid->_vertices[j]._pos.y);
	return glm::distance(_grid->_vertices[i]._pos, _grid->_vertices[j]._pos);
}


bool PathFinder::line_of_sight(unsigned int i, unsigned int j) {
	glm::vec2 pt_begin= _grid->_vertices[i]._pos;
	glm::vec2 pt_end= _grid->_vertices[j]._pos;
	for (auto poly : _polygons) {
		if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
		//if (distance_poly_segment(poly, pt_begin, pt_end, NULL)< 0.1f) {
			return false;
		}
	}
	return true;
}


bool PathFinder::path_find_nodes(unsigned int start, unsigned int goal, std::vector<unsigned int> & path, std::vector<unsigned int> & visited) {
	priority_queue< pair<unsigned int, float>, vector<pair<unsigned int, float> >, decltype(&frontier_cmp) > frontier(frontier_cmp);
	unordered_map<unsigned int, unsigned int> came_from;
	unordered_map<unsigned int, float> cost_so_far;

	frontier.emplace(start, 0.0f);
	came_from[start]= start;
	cost_so_far[start]= 0.0f;

	while (!frontier.empty()) {
		unsigned int current= frontier.top().first;
		frontier.pop();
		
		visited.push_back(current);

		if (current== goal) {
			break;
		}

		vector<unsigned int> nexts= _grid->neighbors(current);
		for (auto next : nexts) {
			unsigned int theta= current;
			/*if (l->line_of_sight(came_from[current], next)) {
				theta= came_from[current];
			}*/
			float new_cost= cost_so_far[theta]+ cost(theta, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= theta;
				//float priority= new_cost; // dijkstra
				//float priority= heuristic(next, goal); // greedy best first search
				float priority= new_cost+ heuristic(next, goal); // A *
				frontier.emplace(next, priority);
			}
		}
	}

	if (!came_from.count(goal)) {
		//cout << "disconnected\n";
		return false;
	}

	unsigned int current= goal;
	while (current!= start) {
		path.push_back(current);
		current= came_from[current];
	}
	path.push_back(start);
	reverse(path.begin(), path.end());

	return true;
}


bool PathFinder::path_find(glm::vec2 start, glm::vec2 goal, vector<glm::vec2> & path, vector<unsigned int> & visited) {
	if ((!point_in_aabb(start, _grid->_aabb)) || (!point_in_aabb(goal, _grid->_aabb))) {
		return false;
	}

	unsigned int start_col_min= (unsigned int)(((start.x- _grid->_origin.x)/ _grid->_size.x)* (float)(_grid->_n_cols- 1));
	unsigned int start_col_max= start_col_min+ 1;
	unsigned int start_lig_min= (unsigned int)(((start.y- _grid->_origin.y)/ _grid->_size.y)* (float)(_grid->_n_ligs- 1));
	unsigned int start_lig_max= start_lig_min+ 1;
	vector<unsigned int> start_ids;
	start_ids.push_back(_grid->col_lig2id(start_col_min, start_lig_min));
	start_ids.push_back(_grid->col_lig2id(start_col_max, start_lig_min));
	start_ids.push_back(_grid->col_lig2id(start_col_max, start_lig_max));
	start_ids.push_back(_grid->col_lig2id(start_col_min, start_lig_max));

	unsigned int goal_col_min= (unsigned int)(((goal.x- _grid->_origin.x)/ _grid->_size.x)* (float)(_grid->_n_cols- 1));
	unsigned int goal_col_max= goal_col_min+ 1;
	unsigned int goal_lig_min= (unsigned int)(((goal.y- _grid->_origin.y)/ _grid->_size.y)* (float)(_grid->_n_ligs- 1));
	unsigned int goal_lig_max= goal_lig_min+ 1;
	vector<unsigned int> goal_ids;
	goal_ids.push_back(_grid->col_lig2id(goal_col_min, goal_lig_min));
	goal_ids.push_back(_grid->col_lig2id(goal_col_max, goal_lig_min));
	goal_ids.push_back(_grid->col_lig2id(goal_col_max, goal_lig_max));
	goal_ids.push_back(_grid->col_lig2id(goal_col_min, goal_lig_max));

	std::vector<unsigned int> nodes;
	bool is_path_ok= path_find_nodes(start_ids[0], goal_ids[0], nodes, visited);
	if (!is_path_ok) {
		return false;
	}
	
	unsigned int start_idx= 0;
	for (unsigned int i=1; i<nodes.size(); ++i) {
		if (find(start_ids.begin(), start_ids.end(), nodes[i])== start_ids.end()) {
			start_idx= i- 1;
			break;
		}
	}
	unsigned int goal_idx= nodes.size()- 1;
	for (unsigned int i=nodes.size()-2; i>=0; --i) {
		if (find(goal_ids.begin(), goal_ids.end(), nodes[i])== goal_ids.end()) {
			goal_idx= i+ 1;
			break;
		}
	}

	path.push_back(start);
	for (unsigned int i=start_idx; i<=goal_idx; ++i) {
		path.push_back(_grid->_vertices[nodes[i]]._pos);
	}
	path.push_back(goal);

	return true;
}


void PathFinder::draw_svg(const vector<unsigned int> & path, const vector<unsigned int> & visited, string svg_path) {
	ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << _grid->_origin.x << " " << _grid->_origin.y << " " << _grid->_size.x << " " << _grid->_size.y << "\">\n";

	if (path.size()) {
		for (unsigned int i=0; i<path.size()- 1; ++i) {
			float x1= _grid->_vertices[path[i]]._pos.x;
			float y1= _grid->_vertices[path[i]]._pos.y;
			float x2= _grid->_vertices[path[i+ 1]]._pos.x;
			float y2= _grid->_vertices[path[i+ 1]]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"red\" stroke-width=\"0.06\" />\n";
		}
	}

	for (auto poly : _polygons) {
		f << "<polygon points=\"";
		for (auto pt : poly->_pts) {
			f << pt.x << "," << pt.y << " ";
		}
		f << "\" fill=\"none\" stroke=\"purple\" stroke-width=\"0.02\" />\n";
	}

	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		float x1= _grid->_it_v->second._pos.x;
		float y1= _grid->_it_v->second._pos.y;
		string color= "black";
		if (find(visited.begin(), visited.end(), _grid->_it_v->first)!= visited.end()) {
			color= "cyan";
		}
		float radius= 0.01* _grid->_it_v->second._weight;
		f << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << radius << "\" fill=\"" << color << "\" />\n";
		//f << "<text x=\"" << x1+ 0.15f << "\" y=\"" << y1- 0.15f << "\" fill=\"black\" font-size=\"0.2px\">" << to_string(_grid->_it_v->first) << "</text>\n";

		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			float x2= _grid->_vertices[_grid->_it_e->first]._pos.x;
			float y2= _grid->_vertices[_grid->_it_e->first]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"black\" stroke-width=\"0.01\" />\n";
			_grid->_it_e++;
		}

		_grid->_it_v++;
	}
	
	f << "</svg>\n</body>\n</html>\n";
	f.close();
}


// -------------------------------------------------------------------------------------------------------------
PathFinderDebug::PathFinderDebug() {

}


PathFinderDebug::PathFinderDebug(GLuint prog_draw) :
	_prog_draw(prog_draw), _world2clip(glm::mat4(1.0f)), _n_pts_grid(0), _n_pts_obstacle(0), _n_pts_path(0)
{
	glGenBuffers(3, _buffers);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
	glUseProgram(0);
}


PathFinderDebug::~PathFinderDebug() {
	
}


void PathFinderDebug::draw() {
	if (_n_pts_grid> 0) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(_world2clip));
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_color_loc);

		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_POINTS, 0, _n_pts_grid);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	if (_n_pts_obstacle> 0) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(_world2clip));
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_color_loc);

		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_LINES, 0, _n_pts_obstacle* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	if (_n_pts_path> 0) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(_world2clip));
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_color_loc);

		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_LINES, 0, (_n_pts_path- 1)* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}


void PathFinderDebug::anim(const glm::mat4 & world2clip) {
	_world2clip= world2clip;
}


void PathFinderDebug::update(const PathFinder & path_finder, const vector<glm::vec2> & path, const vector<unsigned int> & visited) {
	float alti= 300.0f;

	_n_pts_grid= path_finder._grid->_vertices.size();
	float data_pts[_n_pts_grid* 6];
	unsigned int idx= 0;
	path_finder._grid->_it_v= path_finder._grid->_vertices.begin();
	while (path_finder._grid->_it_v!= path_finder._grid->_vertices.end()) {
		data_pts[6* idx+ 0]= path_finder._grid->_it_v->second._pos.x;
		data_pts[6* idx+ 1]= path_finder._grid->_it_v->second._pos.y;
		data_pts[6* idx+ 2]= alti;
		if (find(visited.begin(), visited.end(), path_finder._grid->_it_v->first)!= visited.end()) {
			data_pts[6* idx+ 3]= 0.9f;
			data_pts[6* idx+ 4]= 0.2f;
			data_pts[6* idx+ 5]= 0.2f;
		}
		else {
			if (path_finder._grid->_it_v->second._weight< 10.0f) {
			//if (path_finder._grid->_it_v->second._active) {
				data_pts[6* idx+ 3]= 0.2f;
				data_pts[6* idx+ 4]= 0.9f;
				data_pts[6* idx+ 5]= 0.9f;
			}
			else {
				data_pts[6* idx+ 3]= 0.2f;
				data_pts[6* idx+ 4]= 0.5f;
				data_pts[6* idx+ 5]= 0.5f;
			}
		}
		path_finder._grid->_it_v++;
		idx++;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, _n_pts_grid* 6* sizeof(float), data_pts, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// -------------------------------------------------
	_n_pts_obstacle= 0;
	for (auto polygon : path_finder._polygons) {
		_n_pts_obstacle+= polygon->_pts.size();
	}
	float data_obstacle[_n_pts_obstacle* 12];
	idx= 0;
	for (auto polygon : path_finder._polygons) {
		for (unsigned int i=0; i<polygon->_pts.size(); ++i) {
			data_obstacle[12* idx+ 0]= polygon->_pts[i].x;
			data_obstacle[12* idx+ 1]= polygon->_pts[i].y;
			data_obstacle[12* idx+ 2]= alti;
			data_obstacle[12* idx+ 3]= 0.2f;
			data_obstacle[12* idx+ 4]= 0.2f;
			data_obstacle[12* idx+ 5]= 0.9f;

			data_obstacle[12* idx+ 6]= polygon->_pts[(i+ 1)% polygon->_pts.size()].x;
			data_obstacle[12* idx+ 7]= polygon->_pts[(i+ 1)% polygon->_pts.size()].y;
			data_obstacle[12* idx+ 8]= alti;
			data_obstacle[12* idx+ 9]= 0.2f;
			data_obstacle[12* idx+ 10]= 0.2f;
			data_obstacle[12* idx+ 11]= 0.9f;

			idx++;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, _n_pts_obstacle* 12* sizeof(float), data_obstacle, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// -------------------------------------------------
	_n_pts_path= path.size();
	float data_path[(_n_pts_path- 1)* 12];
	for (unsigned int i=0; i<_n_pts_path- 1; ++i) {
		data_path[12* i+ 0]= path[i].x;
		data_path[12* i+ 1]= path[i].y;
		data_path[12* i+ 2]= alti;
		data_path[12* i+ 3]= 0.2;
		data_path[12* i+ 4]= 0.5;
		data_path[12* i+ 5]= 0.2;

		data_path[12* i+ 6]= path[i+ 1].x;
		data_path[12* i+ 7]= path[i+ 1].y;
		data_path[12* i+ 8]= alti;
		data_path[12* i+ 9]= 0.2;
		data_path[12* i+ 10]= 0.5;
		data_path[12* i+ 11]= 0.2;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, (_n_pts_path- 1)* 12* sizeof(float), data_path, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
