#ifndef PATH_FIND_HIERARCHICAL
#define PATH_FIND_HIERARCHICAL

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "bbox_2d.h"


struct HPAEdge {
	float _weight;

	
	HPAEdge();
	HPAEdge(float weight);
	HPAEdge();
};


struct HPAVertex {
	glm::vec2 _pos;
	std::unordered_map<unsigned int, HPAEdge> _edges;
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
	std::unordered_map<unsigned int, HPAVertex> _vertices;
	std::unordered_map<unsigned int, GraphVertex>::iterator _it_v;
	std::unordered_map<unsigned int, GraphEdge>::iterator _it_e;
	std::vector<HPACluster *> _clusters;


	HPAGraph();
	~HPAGraph();
	void add_vertex(unsigned int i, float weight=0.0f, glm::vec2 pos=glm::vec(0.0f));
	void add_edge(unsigned int i, unsigned int j, float weight=0.0f);
	void remove_vertex(unsigned int i);
	void remove_edge(unsigned int i, unsigned int j);
	std::vector<unsigned int> neighbors(unsigned int i);
};


struct HPA {
	std::vector<HPAGraph *> _graphs;


	HPA();
	HPA(glm::vec2 step_size, glm::uvec2 level0_dimensions, std::vector<glm::uvec2> cluster_sizes);
	~HPA();
};

#endif
