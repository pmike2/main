#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


// http://www.geom.uiuc.edu/~samuelp/del_project.html



struct Triangle {
	Triangle();
	~Triangle();

	int _idx[3];
};



struct BTreeNode {
	BTreeNode();
	~BTreeNode();


	std::vector<Triangle *> _triangles;
	unsigned int _idx_min;
	unsigned int _idx_max;
	BTreeNode * _left;
	BTreeNode * _right;
};


struct BTree {
	BTree();
	~BTree();
	void recursive_init(BTreeNode * node);
	void recursive_delete(BTreeNode * node);
	void recursive_leaf_triangulate(BTreeNode * node);
	void recursive_merge(BTreeNode * node);
	void recursive_print(BTreeNode * node);
	void triangulate();
	void print();


	BTreeNode * _root;
	std::vector<glm::vec2> _pts;
};


#endif
