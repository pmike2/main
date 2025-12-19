#ifndef GEOM_H
#define GEOM_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"


const bool VERBOSE = false;


bool triangle_intersects_triangle(pt_3d v[3], pt_3d w[3]);


//-------------------------------------
struct Face;

// pt pour ConvexHull
struct Pt {
	Pt();
	Pt(pt_3d coords);
	~Pt();
	friend std::ostream & operator << (std::ostream & os, const Pt & pt);


	pt_3d _coords; // coordonnées
	std::vector<Face *> _conflict; // liste de faces en conflit avec le point
};


// face triangulaire pour ConvexHull
struct Face {
	Face();
	Face(glm::uvec3 idx);
	~Face();
	void change_orientation(); // changer orientation de la face ; on veut qu'elles soient toutes en CCW vues de l'extérieur
	bool operator==(const Face& rhs) { return (rhs._idx[0] == _idx[0] && rhs._idx[1] == _idx[1] && rhs._idx[2] == _idx[2]); }
	friend std::ostream & operator << (std::ostream & os, const Face & face);


	glm::uvec3 _idx; // triplet d'indices de point au sein de ConvexHull._pts
	pt_3d _normal; // vecteur normal à la face
	std::vector<Pt *> _conflict; // liste de points en conflit avec la face
	bool _delete; // faut-il supprimer la face
};


// délimitation des edges vus par un point
struct Horizon {
	Face * _face; // face à supprimer à terme
	Face * _opposite_face; // face opposée à _face par rapport au edge
	uint _idx_edge; // indice edge au sein de la _face ; _idx_edge = 0 pour v0-v1, = 1 pour v1-v2, = 2 pour v2-v0
};


// enveloppe convexe nuage de points 3D ; voir bouquin computational geom
class ConvexHull {
public:
	ConvexHull();
	~ConvexHull();
	void clear(); // tout supprimer
	bool is_conflict(Pt * pt, Face * face); // le pt est-til en conflit avec la face
	void add_conflict(Pt * pt, Face * face); // ajout conflit entre pt et face
	Pt * add_pt(pt_3d coords); // ajout pt
	Pt * add_pt(number x, number y, number z); // ajout pt
	Face * add_face(glm::uvec3 idx); // ajout face
	Face * opposite_face(Face * face, uint idx_edge); // face opposée par rapport à edge
	void randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax); // pts random
	void randomize(uint n_pts, pt_3d vmin, pt_3d vmax);
	void compute(); // calcul
	friend std::ostream & operator << (std::ostream & os, const ConvexHull & hull);


	std::vector<Pt *> _pts;
	std::vector<Face * > _faces;
};

#endif
