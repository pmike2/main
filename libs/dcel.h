#ifndef DCEL_H
#define DCEL_H

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


struct DCEL_HalfEdge;
struct DCEL_Face;


struct DCEL_Vertex {
	glm::vec2 _coords;
	DCEL_HalfEdge * _incident_edge;
};


struct DCEL_HalfEdge {
	DCEL_Vertex * _origin;
	DCEL_HalfEdge * _twin;
	DCEL_HalfEdge * _next;
	DCEL_HalfEdge * _prev;
	DCEL_Face * _incident_face;
};


struct DCEL_Face {
	DCEL_HalfEdge * _outer_edge; // NULL pour la face infinie
	std::vector<DCEL_HalfEdge> * _inner_edges; // trous
};


class DCEL {
public:
	DCEL();
	~DCEL();
	
	
	
	std::vector<DCEL_Vertex> _vertices;
	std::vector<DCEL_HalfEdge> _half_edges;
	std::vector<DCEL_Face> _faces;
};

#endif
