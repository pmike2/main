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
#include <algorithm>
#include <math.h>


// ----------------------------------
typedef enum {IN_ORDER, PRE_ORDER, POST_ORDER} TraversalType;


// ----------------------------------
template <class T>
class Node {
public:
	Node();
	Node(T data);
	//Node(T data, Node left, Node right, Node parent);
	~Node();

	// cf https://stackoverflow.com/questions/4660123/overloading-friend-operator-for-class-template
	template <class U>
	friend std::ostream & operator << (std::ostream & os, const Node<U> & node);

	T _data;
	Node * _left;
	Node * _right;
	Node * _parent; // parent du noeud; pas nécessaire à priori mais facilite certaines méthodes
};


template <class T>
Node<T>::Node() : _left(NULL), _right(NULL), _parent(NULL) {
}


template <class T>
Node<T>::Node(T data) : _data(data), _left(NULL), _right(NULL), _parent(NULL) {
}


/*template <class T>
Node<T>::Node(T data, Node left, Node right, Node parent) : 
	_data(data), _left(left), _right(right), _parent(parent) {
}
*/

template <class T>
Node<T>::~Node() {
}


template <class T>
std::ostream & operator << (std::ostream & os, const Node<T> & node) {
	os << "node = (" << &node << " , " << node._data << ") ; ";
	if (node._left!= NULL) {
		os << "left = (" << node._left << " , " << node._left->_data << ") ; ";
	}
	else {
		os << "left = NULL ; ";
	}
	if (node._right!= NULL) {
		os << "right = (" << node._right << " , " << node._right->_data << ") ; ";
	}
	else {
		os << "right = NULL ; ";
	}
	return os;
}


// ----------------------------------
template <class T>
class BST {
	//Node<T> * insert(Node<T> * node, T data);
	//Node<T> * remove(Node<T> * node, T data);
	//Node<T> * search(Node<T> * node, T data, bool exact_match);
	void transplant(Node<T> * old_subtree, Node<T> * new_subtree);
	Node<T> * balance(std::vector<T> & sorted_array, int start, int end);
	void traversal(Node<T> * node, TraversalType tt, std::function<void(T)> f);
	void export_html(std::string html_path, Node<T> * node, float x, int y, std::function<std::string(T)> f_str);

	// cf https://stackoverflow.com/questions/4660123/overloading-friend-operator-for-class-template
	template <class U>
	friend std::ostream & operator << (std::ostream & os, const BST<U> & bst);

	
	Node<T> * _root;
	std::function<int(T, T)> _cmp;


public:
	BST();
	BST(std::function<int(T, T)> cmp);
	~BST();
	void clear();
	bool empty();
	Node<T> * minimum(Node<T> * node);
	Node<T> * maximum(Node<T> * node);
	Node<T> * successor(Node<T> * node);
	Node<T> * predecessor(Node<T> * node);
	void insert(T data);
	void remove(T data);
	Node<T> * search(Node<T> * node, T data, bool exact_match);
	Node<T> * search(T data, bool exact_match=true);
	std::vector<T> get_sorted_array();
	void balance();
	void traversal(TraversalType tt, std::function<void(T)> f=[](T a){std::cout << a << " ";});
	//std::pair<Node<T> *, Node<T> *> neighbours_leaf(T data);
	void export_html(std::string html_path, std::function<std::string(T)> f_str=[](T a){return std::to_string(a);});
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
	_root= NULL;
	_cmp= cmp;
}


template <class T>
BST<T>::~BST() {
}


template <class T>
void BST<T>::clear() {
	remove(_root);
}


// le BST est-il vide
template <class T>
bool BST<T>::empty() {
	if (_root== NULL) {
		return true;
	}
	return false;
}


// renvoie le noeud min dans le sous-arbre de racine node
template <class T>
Node<T> * BST<T>::minimum(Node<T> * node) {
	while (node->_left!= NULL) {
		node= node->_left;
	}
	return node;
}


// renvoie le noeud min dans le sous-arbre de racine node
template <class T>
Node<T> * BST<T>::maximum(Node<T> * node) {
	while (node->_right!= NULL) {
		node= node->_right;
	}
	return node;
}


// renvoie le noeud directement supérieur à node
template <class T>
Node<T> * BST<T>::successor(Node<T> * node) {
	// si node a un enfant _right, on renvoie le node minimum de node._right
	if (node->_right!= NULL) {
		return minimum(node->_right);
	}
	// sinon on cherche le parent le plus bas dont le left child est un parent de node
	Node<T> * y= node->_parent;
	while ((y!= NULL) && (node== y->_right)) {
		node= y;
		y= y->_parent;
	}
	return y;
}


// renvoie le noeud directement inférieur à node
template <class T>
Node<T> * BST<T>::predecessor(Node<T> * node) {
	// si node a un enfant _left, on renvoie le node maximum de node._left
	if (node->_left!= NULL) {
		return maximum(node->_left);
	}
	// sinon on cherche le parent le plus bas dont le right child est un parent de node
	Node<T> * y= node->_parent;
	while ((y!= NULL) && (node== y->_left)) {
		node= y;
		y= y->_parent;
	}
	return y;
}


/*template <class T>
Node<T> * BST<T>::insert(Node<T> * node, T data) {
	if (node== NULL) {
		return new Node<T>(data);
	}

	if (_cmp(data, node->_data)> 0) {
		node->_right= insert(node->_right, data);
	}
	else if (_cmp(data, node->_data)<= 0) {
		node->_left= insert(node->_left, data);
	}
	return node;
}*/


template <class T>
void BST<T>::insert(T data) {
	//_root= insert(_root, data);

	// noeud à insérer
	Node<T> * z= new Node<T>(data);
	// noeud comparé à z
	Node<T> * x= _root;
	// sera le parent de z
	Node<T> * y= NULL;

	// on cherche y, qui sera le parent de z
	while (x!= NULL) {
		y= x;
		if (_cmp(z->_data, x->_data)< 0) {
			x= x->_left;
		}
		else {
			x= x->_right;
		}
	}
	z->_parent= y;

	// BST était vide
	if (y== NULL) {
		_root= z;
	}
	else if (_cmp(z->_data, y->_data)< 0) {
		y->_left= z;
	}
	else {
		y->_right= z;
	}
}


// remplacement de sous-arbres : new_subtree va remplacer old_subtree
// new_subtree peut valoir NULL si l'on veut supprimer old_subtree
template <class T>
void BST<T>::transplant(Node<T> * old_subtree, Node<T> * new_subtree) {
	if (old_subtree== NULL) {
		_root= new_subtree;
	}
	else if (old_subtree== old_subtree->_parent->_left) {
		old_subtree->_parent->_left= new_subtree;
	}
	else {
		old_subtree->_parent->_right= new_subtree;
	}
	
	if (new_subtree!= NULL) {
		new_subtree->_parent= old_subtree->_parent;
	}
}


/*template <class T>
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

	// node->_data == data
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
}*/


// suppression de data ; suppose qu'il ne peut pas y avoir la même valeur 2 fois dans l'arbre !
template <class T>
void BST<T>::remove(T data) {
	//_root= remove(_root, data);
	Node<T> * node= search(data, true);
	// data n'est pas dans l'arbre, on sort
	if (node== NULL) {
		return;
	}

	// si node n'a pas d'enfant left on remplace le subtree node par le subtree node._right
	if (node->_left== NULL) {
		transplant(node, node->_right);
	}
	// si node n'a pas d'enfant right on remplace le subtree node par le subtree node._left
	else if (node->_right== NULL) {
		transplant(node, node->_left);
	}
	// sinon voir schéma livre algo p323
	else {
		Node<T> * y= minimum(node->_right);
		if (y!= node->_right) {
			transplant(y, y->_right);
			y->_right= node->_right;
			y->_right->_parent= y->_right;
		}
		transplant(node, y);
		y->_left= node->_left;
		y->_left->_parent= y->_left;
	}
}


/*template <class T>
Node<T> * BST<T>::search(Node<T> * node, T data, bool exact_match) {
	if (node== NULL) {
		return NULL;
	}
	if ((!exact_match) && (node->_right== NULL) && (node->_left== NULL)) {
		return node;
	}
	if (_cmp(node->_data, data)== 0) {
		return node;
	}
	if (_cmp(node->_data, data)< 0) {
		return search(node->_right, data, exact_match);
	}
	return search(node->_left, data, exact_match);
}*/


// recherche de data dans le sous-arbre de sommet node
template <class T>
Node<T> * BST<T>::search(Node<T> * node, T data, bool exact_match) {
	while ((node!= NULL) && (node->_data!= data)) {
		if (_cmp(node->_data, data)< 0) {
			if ((node->_right== NULL) && (!exact_match)) {
				return node;
			}
			node= node->_right;
		}
		else {
			if ((node->_left== NULL) && (!exact_match)) {
				return node;
			}
			node= node->_left;
		}
	}
	return node;
}


// recherche de data dans tout l'arbre
template <class T>
Node<T> * BST<T>::search(T data, bool exact_match) {
	return search(_root, data, exact_match);
}


template <class T>
std::vector<T> BST<T>::get_sorted_array() {
	std::vector<T> result;
	traversal(TraversalType::IN_ORDER, [&result](T x){result.push_back(x);});
	return result;
}


template <class T>
void BST<T>::balance() {
	std::vector<T> sorted_array= get_sorted_array();
	_root= balance(sorted_array, 0, sorted_array.size()- 1);
}


template <class T>
Node<T> * BST<T>::balance(std::vector<T> & sorted_array, int start, int end) {
	if (start> end) {
		return NULL;
	}

	int mid= (start+ end)/ 2;
	Node<T> * node= new Node<T>(sorted_array[mid]);
	node->_left= balance(sorted_array, start, mid- 1);
	node->_right= balance(sorted_array, mid+ 1, end);
	return node;
}


template <class T>
void BST<T>::traversal(TraversalType tt, std::function<void(T)> f) {
	traversal(_root, tt, f);
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

/*
template <class T>
std::pair<Node<T> *, Node<T> *> BST<T>::neighbours_leaf(T data) {
	std::vector<T> array= get_sorted_array();
	/*array.erase(std::remove_if(array.begin(), array.end(), 
		[](const T & x){ return ((x._left!= NULL) || (x._right!= NULL));}
	), array.end());*/

	/*Node<T> * prev= NULL;
	Node<T> * next= NULL;
	for (int i=0; i<array.size(); ++i) {
		if (array[i]== data) {
			if (i> 0) {
				prev= search(array[i- 1]);
			}
			if (i< array.size()- 1) {
				next= search(array[i+ 1]);
			}
		}
	}
	return std::make_pair(prev, next);
}*/


template <class T>
std::ostream & operator << (std::ostream & os, const BST<T> & bst) {
	traversal(IN_ORDER, [os](T node)->std::ostream{os << node << "\n";});
	return os;
}


template <class T>
void BST<T>::export_html(std::string html_path, std::function<std::string(T)> f_str) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".point_text_class {fill: red; font-size: 0.05px;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" viewbox=\"" << -1.2 << " " << -0.2 << " " << 2.4 << " " << 2.4 << "\">\n";
	f.close();

	export_html(html_path, _root, 0.0f, 1, f_str);
	
	f.open(html_path, std::ios_base::app);
	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


template <class T>
void BST<T>::export_html(std::string html_path, Node<T> * node, float x, int y, std::function<std::string(T)> f_str) {
	if (node== NULL) {
		return;
	}

	float x_factor= 1.0f;
	float y_factor= 0.1f;
	float pt_x= x* x_factor;
	float pt_y= float(y)* y_factor;
	float left_x= x- 1.0f/ pow(2.0f, float(y));
	float right_x= x+ 1.0f/ pow(2.0f, float(y));
	float pt_left_x= left_x* x_factor;
	float pt_right_x= right_x* x_factor;
	float pt_left_y= float(y+ 1)* y_factor;
	float pt_right_y= float(y+ 1)* y_factor;
	
	std::ofstream f;
	f.open(html_path, std::ios_base::app);
	f << "<text class=\"point_text_class\" x=\"" << pt_x+ 0.02f << "\" y=\"" << pt_y << "\" >" << f_str(node->_data) << "</text>\n";
	f << "<circle class=\"point_class\" cx=\"" << pt_x << "\" cy=\"" << pt_y << "\" r=\"" << 0.01f << "\" />\n";
	if (node->_left!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_left_x << "\" y2=\"" << pt_left_y << "\" />\n";
	}
	if (node->_right!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_right_x << "\" y2=\"" << pt_right_y << "\" />\n";
	}
	f.close();
	export_html(html_path, node->_left, left_x, y+ 1, f_str);
	export_html(html_path, node->_right, right_x, y+ 1, f_str);
}

#endif
