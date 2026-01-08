#ifndef PATH_FIND_HIERARCHICAL
#define PATH_FIND_HIERARCHICAL

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "graph.h"


struct HPAEdge {
	float _weight;

	
	HPAEdge();
	HPAEdge(float weight);
	~HPAEdge();
};


struct HPAVertex {
	glm::vec2 _pos;
	std::unordered_map<uint, HPAEdge> _edges;
	bool _active;
	HPAVertex * _child;


	HPAVertex();
	HPAVertex(glm::vec2 pos);
	~HPAVertex();
};


struct HPACluster {
	AABB_2D * _aabb;


	HPACluster();
	HPACluster(glm::vec2 pos, glm::vec2 size);
	~HPACluster();
};


struct HPAGraph {
	std::unordered_map<uint, HPAVertex> _vertices;
	std::unordered_map<uint, HPAVertex>::iterator _it_v;
	std::unordered_map<uint, HPAEdge>::iterator _it_e;
	std::vector<HPACluster *> _clusters;


	HPAGraph();
	~HPAGraph();
	void add_vertex(uint i, float weight=0.0f, glm::vec2 pos= glm::vec2(0.0f, 0.0f));
	void add_edge(uint i, uint j, float weight=0.0f);
	void remove_vertex(uint i);
	void remove_edge(uint i, uint j);
	std::vector<uint> neighbors(uint i);
};


struct HPA {
	std::vector<HPAGraph *> _graphs;


	HPA();
	HPA(glm::vec2 step_size, glm::uvec2 level0_dimensions, std::vector<glm::uvec2> cluster_sizes);
	~HPA();
};

#endif
