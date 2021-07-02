#include <algorithm>

#include "triangulation.h"

#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

#include "geom_2d.h"


using namespace std;


bool pts_sort(glm::vec2 & pt1, glm::vec2 & pt2) {
	return pt1.x< pt2.x;
}


// --------------------------------------------------------------------
Triangle::Triangle() {
	_idx[0]= -1;
	_idx[1]= -1;
	_idx[2]= -1;
}


Triangle::~Triangle() {

}


// --------------------------------------------------------------------
BTreeNode::BTreeNode() : _idx_min(0), _idx_max(0), _left(NULL), _right(NULL) {

}


BTreeNode::~BTreeNode() {
	for (auto t : _triangles) {
		delete t;
	}
	_triangles.clear();
}


// --------------------------------------------------------------------
BTree::BTree() :_root(NULL) {
	
}


BTree::~BTree() {
	recursive_delete(_root);
}


void BTree::recursive_init(BTreeNode * node) {
	node->_left= new BTreeNode();
	node->_left->_idx_min= node->_idx_min;
	node->_left->_idx_max= (node->_idx_min+ node->_idx_max)/ 2;
	if (node->_left->_idx_max- node->_left->_idx_min> 3) {
		recursive_init(node->_left);
	}

	node->_right= new BTreeNode();
	node->_right->_idx_min= node->_left->_idx_max+ 1;
	node->_right->_idx_max= node->_idx_max;
	if (node->_right->_idx_max- node->_right->_idx_min> 3) {
		recursive_init(node->_right);
	}
}


void BTree::recursive_delete(BTreeNode * node) {
	if (node->_left) {
		recursive_delete(node->_left);
		recursive_delete(node->_right);
	}
	delete node;
}


void BTree::recursive_leaf_triangulate(BTreeNode * node) {
	if (node->_left) {
		recursive_leaf_triangulate(node->_left);
		recursive_leaf_triangulate(node->_right);
	}
	else {
		if (node->_idx_max- node->_idx_min== 2) {
			Triangle * t= new Triangle();
			t->_idx[0]= node->_idx_min;
			t->_idx[1]= node->_idx_min+ 1;
			t->_idx[2]= node->_idx_min+ 2;
			node->_triangles.push_back(t);
		}
		else if (node->_idx_max- node->_idx_min== 1) {
			Triangle * t= new Triangle();
			t->_idx[0]= node->_idx_min;
			t->_idx[1]= node->_idx_min+ 1;
			node->_triangles.push_back(t);
		}
	}
}


void BTree::recursive_merge(BTreeNode * node) {
	if (node->_left) {
		recursive_merge(node->_left);
		recursive_merge(node->_right);


	}
}


void BTree::recursive_print(BTreeNode * node) {
	cout << "-------\n";
	cout << node->_idx_min << " ; " << node->_idx_max << "\n";
	for (auto t : node->_triangles) {
		cout << t->_idx[0] << " ; " << t->_idx[1] << " ; " << t->_idx[2] << "\n";
	}
	if (node->_left) {
		recursive_print(node->_left);
		recursive_print(node->_right);
	}
}


void BTree::triangulate() {
	unsigned int n_pts= _pts.size();

	sort(_pts.begin(), _pts.end(), pts_sort);

	_root= new BTreeNode();
	_root->_idx_min= 0;
	_root->_idx_max= n_pts- 1;
	recursive_init(_root);
	recursive_leaf_triangulate(_root);
	recursive_merge(_root);
	
}


void BTree::print() {
	for (auto pt : _pts) {
		cout << glm::to_string(pt) << "\n";
	}
	recursive_print(_root);
}

