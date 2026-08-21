#ifndef QUADTREE_H
#define QUADTREE_H

#include "typedefs.h"
#include "bbox_2d.h"
#include "view_system.h"


template <class T>
struct QuadTreeNode {
	QuadTreeNode();
	QuadTreeNode(T data);
	~QuadTreeNode();

	pt_2d _pt;
	T _data;
};


template <class T>
struct QuadTree {
	QuadTree();
	QuadTree(ViewSystem * view_system);
	~QuadTree();
	void insert(QuadTreeNode<T> * node);


	ViewSystem * _view_system;
	AABB_2D * _aabb;
	QuadTree * _so;
	QuadTree * _se;
	QuadTree * _no;
	QuadTree * _ne;
	QuadTreeNode * _node;
};


// ---------------------------------------------------------


#endif
