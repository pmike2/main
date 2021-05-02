#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "geom_2d.h"


struct Edge {
	//unsigned int _start_node;
	//unsigned int _end_node;
	float _weight;
};


struct Vertex {
	float _weight;
	glm::vec2 _pos;
	bool _visited;
	std::unordered_map<unsigned int, Edge> _edges;
};


struct Graph {
	std::unordered_map<unsigned int, Vertex> _vertices;
	std::unordered_map<unsigned int, Vertex>::iterator _it_v;
	std::unordered_map<unsigned int, Edge>::iterator _it_e;


	Graph();
	~Graph();
	void add_vertex(unsigned int i, float weight=0.0f, float x=0.0f, float y=0.0f);
	void add_edge(unsigned int i, unsigned int j, float weight=0.0f, bool weight_is_dist=false);
	void remove_vertex(unsigned int i);
	void remove_edge(unsigned int i, unsigned int j);
	std::vector<unsigned int> neighbors(unsigned int i);
	float cost(unsigned int i, unsigned int j);
	float heuristic(unsigned int i, unsigned int j);
	void clear();
	void rand();
	friend std::ostream & operator << (std::ostream & os, Graph & g);
};


struct Grid : public Graph {
	unsigned int _width;
	unsigned int _height;


	Grid();
	Grid(unsigned int width, unsigned int height);
	~Grid();
	friend std::ostream & operator << (std::ostream & os, Grid & g);
};


bool frontier_cmp(std::pair<unsigned int, float> x, std::pair<unsigned int, float> y);

struct Level {
	Grid * _grid;
	std::vector<Polygon2D *> _polygons;


	Level();
	Level(unsigned int width, unsigned int height);
	~Level();
	bool line_of_sight(unsigned int i, unsigned int j);
	std::vector<unsigned int> path_find(unsigned int start, unsigned int goal);
	void draw_svg(std::vector<unsigned int> path, std::string result);
};


#endif
