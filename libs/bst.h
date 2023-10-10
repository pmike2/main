/*
Binary Search Tree
cf https://www.geeksforgeeks.org/binary-search-tree-data-structure
implémenté initialement pour voronoi

*/


#ifndef BST_H
#define BST_H

#include <iostream>
#include <functional>
#include <fstream>
#include <sstream>
#include <string>


// ----------------------------------
typedef enum {IN_ORDER, PRE_ORDER, POST_ORDER} TraversalType;


// ----------------------------------
template <class T>
class Node {
public:
	Node();
	Node(T data);
	~Node();

	T _data;
	Node * _left;
	Node * _right;
};


template <class T>
Node<T>::Node() {
	_left= NULL;
	_right= NULL;
}


template <class T>
Node<T>::Node(T data) {
	_data= data;
	_left= NULL;
	_right= NULL;
}


template <class T>
Node<T>::~Node() {
}


// ----------------------------------
template <class T>
class BST {
	Node<T> * insert(Node<T> * node, T data);
	Node<T> * remove(Node<T> * node, T data);
	Node<T> * search(Node<T> * node, T data);
	void traversal(Node<T> * node, TraversalType tt);
	void traversal(Node<T> * node, TraversalType tt, std::function<void(T)> f);
	void export_html(std::string html_path, Node<T> * node, float x, int y);
	void export_html(std::string html_path, Node<T> * node, float x, int y, std::function<std::string(T)> f);

	
	Node<T> * _root;
	std::function<int(T, T)> _cmp;


public:
	BST();
	BST(std::function<int(T, T)> cmp);
	~BST();
	void clear();
	bool is_empty();
	void insert(T data);
	void remove(T data);
	Node<T> * search(T data);
	void balance();
	void traversal(TraversalType tt);
	void traversal(TraversalType tt, std::function<void(T)> f);
	void export_html(std::string html_path);
	void export_html(std::string html_path, std::function<std::string(T)> f);


	//Node<T> * _root;
};


template <class T>
BST<T>::BST() {
	_root= NULL;
	_cmp= [](T a, T b) { 
		if (a < b) return -1;
		else if (a > b) return 1;
		else return 0;
	};
}


template <class T>
BST<T>::BST(std::function<int(T, T)> cmp) {
	_cmp= cmp;
}


template <class T>
BST<T>::~BST() {
}


template <class T>
void BST<T>::clear() {
	remove(_root);
}


template <class T>
bool BST<T>::is_empty() {
	if (_root== NULL) {
		return true;
	}
	return false;
}


template <class T>
Node<T> * BST<T>::insert(Node<T> * node, T data) {
	if (node== NULL) {
		return new Node<T>(data);
	}

	if (_cmp(data, node->_data)> 0) {
		node->_right= insert(node->_right, data);
	}
	else if (_cmp(data, node->_data)< 0) {
		node->_left= insert(node->_left, data);
	}
	return node;
}


template <class T>
void BST<T>::insert(T data) {
	_root= insert(_root, data);
}


template <class T>
Node<T> * BST<T>::remove(Node<T> * node, T data) {
	if (node== NULL) {
		return NULL;
	}
	
	if (_cmp(node->_data, data)> 0) {
		node->_left= remove(node->_left, data);
		return node;
	}
	else if (_cmp(node->_data, data)< 0) {
		node->_right= remove(node->_right, data);
		return node;
	}

	if (node->_left== NULL) {
		Node<T> * tmp= node->_right;
		delete node;
		return tmp;
	}
	else if (node->_right== NULL) {
		Node<T> * tmp= node->_left;
		delete node;
		return tmp;
	}
	else {
		Node<T> * succ_parent= node;
		Node<T> * succ= node->_right;
		while (succ->_left!= NULL) {
			succ_parent= succ;
			succ= succ->_left;
		}
		if (succ_parent!= node) {
			succ_parent->_left= succ->_right;
		}
		else {
			succ_parent->_right= succ->_right;
		}
		node->_data= succ->_data;
		delete succ;
		return node;
	}
}


template <class T>
void BST<T>::remove(T data) {
	_root= remove(_root, data);
}


template <class T>
Node<T> * BST<T>::search(Node<T> * node, T data) {
	if (node== NULL) {
		return NULL;
	}
	if (_cmp(node->_data, data)== 0) {
		return node;
	}
	if (_cmp(node->_data, data)< 0) {
		return search(node->_right, data);
	}
	return search(node->_left, data);
}


template <class T>
Node<T> * BST<T>::search(T data) {
	return search(_root, data);
}


template <class T>
void BST<T>::balance() {

}


template <class T>
void BST<T>::traversal(TraversalType tt) {
	traversal(_root, tt);
	std::cout << "\n";
}


template <class T>
void BST<T>::traversal(Node<T> * node, TraversalType tt) {
	if (node== NULL) {
		return;
	}

	if (tt== IN_ORDER) {
		traversal(node->_left, tt);
		std::cout << node->_data << " ";
		traversal(node->_right, tt);
	}
	else if (tt== PRE_ORDER) {
		std::cout << node->_data << " ";
		traversal(node->_left, tt);
		traversal(node->_right, tt);
	}
	else if (tt== POST_ORDER) {
		traversal(node->_left, tt);
		traversal(node->_right, tt);
		std::cout << node->_data << " ";
	}
}


template <class T>
void BST<T>::traversal(TraversalType tt, std::function<void(T)> f) {
	traversal(_root, tt, f);
	std::cout << "\n";
}


template <class T>
void BST<T>::traversal(Node<T> * node, TraversalType tt, std::function<void(T)> f) {
	if (node== NULL) {
		return;
	}

		if (tt== IN_ORDER) {
		traversal(node->_left, tt, f);
		f(node->_data);
		traversal(node->_right, tt, f);
	}
	else if (tt== PRE_ORDER) {
		f(node->_data);
		traversal(node->_left, tt, f);
		traversal(node->_right, tt, f);
	}
	else if (tt== POST_ORDER) {
		traversal(node->_left, tt, f);
		traversal(node->_right, tt, f);
		f(node->_data);
	}
}


template <class T>
void BST<T>::export_html(std::string html_path) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".point_text_class {fill: red; font-size: 0.1px;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" viewbox=\"" << -1.2 << " " << -0.2 << " " << 2.4 << " " << 2.4 << "\">\n";
	f.close();

	export_html(html_path, _root, 0.0f, 1);
	
	f.open(html_path, std::ios_base::app);
	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


template <class T>
void BST<T>::export_html(std::string html_path, Node<T> * node, float x, int y) {
	if (node== NULL) {
		return;
	}

	float x_factor= 0.5f;
	float y_factor= 0.3f;
	float pt_x= x* x_factor;
	float pt_y= float(y)* y_factor;
	float pt_left_x= (x- 1.0f/float(y))* x_factor;
	float pt_right_x= (x+ 1.0f/float(y))* x_factor;
	float pt_left_y= float(y+ 1)* y_factor;
	float pt_right_y= float(y+ 1)* y_factor;
	
	std::ofstream f;
	f.open(html_path, std::ios_base::app);
	f << "<text class=\"point_text_class\" x=\"" << pt_x+ 0.02f << "\" y=\"" << pt_y << "\" >" << node->_data << "</text>\n";
	f << "<circle class=\"point_class\" cx=\"" << pt_x << "\" cy=\"" << pt_y << "\" r=\"" << 0.01f << "\" />\n";
	if (node->_left!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_left_x << "\" y2=\"" << pt_left_y << "\" />\n";
	}
	if (node->_right!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_right_x << "\" y2=\"" << pt_right_y << "\" />\n";
	}
	f.close();
	export_html(html_path, node->_left, x- 1.0f/float(y), y+ 1);
	export_html(html_path, node->_right, x+ 1.0f/float(y), y+ 1);
}

#endif
