#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>


struct GraphEdge {
	//unsigned int _start_node;
	//unsigned int _end_node;
	float _weight;
};


struct GraphVertex {
	glm::vec3 _pos;
	float _weight;
	std::unordered_map<unsigned int, GraphEdge> _edges;
	//bool _active;
};


struct Graph {
	Graph();
	~Graph();
	void add_vertex(unsigned int i, glm::vec3 pos=glm::vec3(0.0f), float weight=1.0f);
	void add_edge(unsigned int i, unsigned int j, float weight=1.0f, bool weight_is_dist=false);
	void remove_vertex(unsigned int i);
	void remove_edge(unsigned int i, unsigned int j);
	std::vector<unsigned int> neighbors(unsigned int i);
	void clear();
	//void rand();
	void reinit_weights();
	friend std::ostream & operator << (std::ostream & os, Graph & g);


	std::unordered_map<unsigned int, GraphVertex> _vertices;
	std::unordered_map<unsigned int, GraphVertex>::iterator _it_v;
	std::unordered_map<unsigned int, GraphEdge>::iterator _it_e;
};


struct Mesh {
	std::vector<std::pair<unsigned int, unsigned int> > _edges;
	bool _debug= true;
};


#endif
