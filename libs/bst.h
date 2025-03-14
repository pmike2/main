/*
Binary Search Tree
cf https://www.geeksforgeeks.org/binary-search-tree-data-structure
implémenté initialement pour voronoi

*/

#ifndef BST_H
#define BST_H

#include <iostream>
#include <iomanip>
#include <functional>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <math.h>


// ------------------------------------------------------------------------------------------------------
// type de traversée d'un arbre
typedef enum {IN_ORDER, PRE_ORDER, POST_ORDER} TraversalType;


// ------------------------------------------------------------------------------------------------------
// noeud d'un arbre
template <class T>
class Node {
public:
	Node();
	Node(T data);
	~Node();
	bool is_left();
	bool is_right();
	bool is_leaf();
	bool is_root();
	Node * sibling();
	void set_left(Node * n);
	void set_right(Node * n);

	// cf https://stackoverflow.com/questions/4660123/overloading-friend-operator-for-class-template
	template <class U>
	friend std::ostream & operator << (std::ostream & os, const Node<U> & node);

	Node * _left;
	Node * _right;
	Node * _parent; // parent du noeud; pas nécessaire à priori mais facilite certaines méthodes
	T _data;
};


// ------------------------------------------------------------------------------------------------------
// arbre binaire
template <class T>
class BST {
	// méthodes privées
	unsigned int n_nodes(Node<T> * node);
	void max_imbalance(Node<T> * node, int * maximb);
	Node<T> * balance(std::vector<T> & sorted_array, int start, int end);
	void traversal(Node<T> * node, TraversalType tt, std::function<void(Node<T> *)> f);
	//void draw(bool * bits, Node<T> * node, int i, int j, unsigned int width, unsigned int height, unsigned int n);
	void export_html(std::string html_path, Node<T> * node, float x, int y);
	// cf https://stackoverflow.com/questions/4660123/overloading-friend-operator-for-class-template
	template <class U>
	friend std::ostream & operator << (std::ostream & os, BST<U> & bst);
	
public:
	BST();
	BST(std::function<int(T, T)> cmp);
	~BST();
	bool empty();
	unsigned int height(Node<T> * node);
	unsigned int height();
	unsigned int n_nodes();
	int max_imbalance();
	Node<T> * minimum(Node<T> * node);
	Node<T> * minimum();
	Node<T> * maximum(Node<T> * node);
	Node<T> * maximum();
	Node<T> * successor(Node<T> * node);
	Node<T> * predecessor(Node<T> * node);
	Node<T> * successor_leaf(Node<T> * node);
	Node<T> * predecessor_leaf(Node<T> * node);
	Node<T> * gen_node(T data);
	void insert(Node<T> * node);
	void insert(T data);
	void delete_tree(Node<T> * node);
	void clear();
	void remove(Node<T> * node);
	void remove(T data);
	void transplant(Node<T> * old_subtree, Node<T> * new_subtree);
	void rotate_left(Node<T> * node);
	void rotate_right(Node<T> * node);
	Node<T> * search(Node<T> * node, T data, bool exact_match);
	Node<T> * search(T data, bool exact_match=true);
	std::vector<T> get_sorted_array();
	void balance();
	void traversal(TraversalType tt, std::function<void(Node<T> *)> f=[](Node<T> * node){std::cout << *node << "\n";});
	void export_html(std::string html_path);
	//void draw(std::string output);


	// racine
	Node<T> * _root;
	// fonction de comparaison entre 2 noeuds
	std::function<int(T, T)> _cmp;
};


// ------------------------------------------------------------------------------------------------------
template <class T>
Node<T>::Node() :
_left(NULL), _right(NULL), _parent(NULL)
{ }


template <class T>
Node<T>::Node(T data) :
_data(data), _left(NULL), _right(NULL), _parent(NULL)
{ }


template <class T>
Node<T>::~Node() {
	
}


// le noeud est-il l'enfant gauche de son parent
template <class T>
bool Node<T>::is_left() {
	if (_parent== NULL) {
		return false;
	}
	if (_parent->_left== this) {
		return true;
	}
	return false;
}


// le noeud est-il l'enfant droit de son parent
template <class T>
bool Node<T>::is_right() {
	if (_parent== NULL) {
		return false;
	}
	if (_parent->_right== this) {
		return true;
	}
	return false;
}


// est-ce une feuille
template <class T>
bool Node<T>::is_leaf() {
	if ((_left== NULL) && (_right== NULL)) {
		return true;
	}
	return false;
}


// est-ce root
template <class T>
bool Node<T>::is_root() {
	if (_parent== NULL) {
		return true;
	}
	return false;
}


// renvoie la feuille soeur si this est une feuille sinon NULL
template <class T>
Node<T> * Node<T>::sibling() {
	if (!is_leaf()) {
		return NULL;
	}
	if (is_left()) {
		return _parent->_right;
	}
	return _parent->_left;
}


template <class T>
void Node<T>::set_left(Node<T> * n) {
	_left= n;
	if (n!= NULL) {
		n->_parent= this;
	}
}


template <class T>
void Node<T>::set_right(Node<T> * n) {
	_right= n;
	if (n!= NULL) {
		n->_parent= this;
	}
}


// surcharge <<
template <class T>
std::ostream & operator << (std::ostream & os, const Node<T> & node) {
	//std::string NULL_STRING= "--------NULL--------";
	std::string NULL_STRING= "NULL";
	std::string SEPARATOR= " | ";
	unsigned int str_width= 30;

	//os << std::left;
	//os << std::setw(str_width);
	os << node._data;
	os <<  SEPARATOR;
	os << "left = ";
	//os << std::setw(str_width);
	if (node._left!= NULL) {
		os << node._left->_data;
	}
	else {
		os << NULL_STRING;
	}
	os <<  SEPARATOR;
	os << "right = ";
	//os << std::setw(str_width);
	if (node._right!= NULL) {
		os << node._right->_data;
	}
	else {
		os << NULL_STRING;
	}
	os <<  SEPARATOR;
	os << "parent = ";
	//os << std::setw(str_width);
	if (node._parent!= NULL) {
		os << node._parent->_data;
	}
	else {
		os << NULL_STRING;
	}
	return os;
}


// ------------------------------------------------------------------------------------------------------
template <class T>
BST<T>::BST() : 
	_root(NULL),
	_cmp([](T a, T b) { 
		if (a < b) return -1;
		else if (a > b) return 1;
		else return 0;
	})
{ }


template <class T>
BST<T>::BST(std::function<int(T, T)> cmp) :
	_root(NULL),
	_cmp(cmp)
{ }


template <class T>
BST<T>::~BST() {
}


// le BST est-il vide
template <class T>
bool BST<T>::empty() {
	if (_root== NULL) {
		return true;
	}
	return false;
}


// nombre de noeuds sous node
template <class T>
unsigned int BST<T>::n_nodes(Node<T> * node) {
	if (node== NULL) {
		return 0;
	}

	unsigned int n_nodes_left= n_nodes(node->_left);
	unsigned int n_nodes_right= n_nodes(node->_right);
	
	return n_nodes_left+ n_nodes_right+ 1;
}


// nombre de noeuds de l'arbre
template <class T>
unsigned int BST<T>::n_nodes() {
	return n_nodes(_root);
}


template <class T>
void BST<T>::max_imbalance(Node<T> * node, int * maximb) {
	if (node== NULL) {
		return;
	}

	unsigned int n_nodes_left= n_nodes(node->_left);
	unsigned int n_nodes_right= n_nodes(node->_right);
	int imbalance= (int)(n_nodes_right)- (int)(n_nodes_left);
	if (abs(imbalance)> abs(*maximb)) {
		*maximb= imbalance;
	}
	max_imbalance(node->_left, maximb);
	max_imbalance(node->_right, maximb);
}


template <class T>
int BST<T>::max_imbalance() {
	int maximb= 0;
	max_imbalance(_root, &maximb);
	return maximb;
}


// hauteur du sous-arbre de root node
template <class T>
unsigned int BST<T>::height(Node<T> * node) {
	if (node== NULL) {
		return 0;
	}

	unsigned int height_left= height(node->_left);
	unsigned int height_right= height(node->_right);
	
	return std::max(height_left, height_right)+ 1;
}


// hauteur de l'arbre
template <class T>
unsigned int BST<T>::height() {
	return height(_root);
}


// renvoie le noeud min dans le sous-arbre de racine node
template <class T>
Node<T> * BST<T>::minimum(Node<T> * node) {
	while (node->_left!= NULL) {
		node= node->_left;
	}
	return node;
}


// minimum de tout l'arbre
template <class T>
Node<T> * BST<T>::minimum() {
	return minimum(_root);
}


// renvoie le noeud max dans le sous-arbre de racine node
template <class T>
Node<T> * BST<T>::maximum(Node<T> * node) {
	while (node->_right!= NULL) {
		node= node->_right;
	}
	return node;
}


// maximum de tout l'arbre
template <class T>
Node<T> * BST<T>::maximum() {
	return maximum(_root);
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


// renvoie la feuille la plus petite, supérieure à node
template <class T>
Node<T> * BST<T>::successor_leaf(Node<T> * node) {
	Node<T> * succ= successor(node);
	// node est le noeud max
	if (succ== NULL) {
		return NULL;
	}
	
	if (succ->_right== NULL) {
		// succ est la feuille que l'on cherchait
		if (succ->is_leaf()) {
			return succ;
		}
		else {
			// node est déjà la feuille max
			return NULL;
		}
	}
	succ= succ->_right;
	while ((succ->_left!= NULL) || (succ->_right!= NULL)) {
		if (succ->_left!= NULL) {
			succ= succ->_left;
		}
		else if (succ->_right!= NULL) {
			succ= succ->_right;
		}
	}
	return succ;
}


// renvoie la feuille la plus petite, supérieure à node
template <class T>
Node<T> * BST<T>::predecessor_leaf(Node<T> * node) {
	Node<T> * pred= predecessor(node);
	// node est le noeud min
	if (pred== NULL) {
		return NULL;
	}
	
	if (pred->_left== NULL) {
		// pred est la feuille que l'on cherchait
		if (pred->is_leaf()) {
			return pred;
		}
		else {
			// node est déjà la feuille min
			return NULL;
		}
	}
	pred= pred->_left;
	while ((pred->_left!= NULL) || (pred->_right!= NULL)) {
		if (pred->_right!= NULL) {
			pred= pred->_right;
		}
		else if (pred->_left!= NULL) {
			pred= pred->_left;
		}
	}
	return pred;
}


// génération d'un noeud pas encore rattaché à l'arbre
template <class T>
Node<T> * BST<T>::gen_node(T data) {
	return new Node<T>(data);
}


// insertion d'un noeud
template <class T>
void BST<T>::insert(Node<T> * node) {
	// noeud comparé à node
	Node<T> * x= _root;
	// sera le parent de node
	Node<T> * y= NULL;

	// on cherche y, qui sera le parent de node
	while (x!= NULL) {
		y= x;
		if (_cmp(node->_data, x->_data)< 0) {
			x= x->_left;
		}
		else {
			x= x->_right;
		}
	}
	node->_parent= y;

	// BST était vide
	if (y== NULL) {
		_root= node;
	}
	else if (_cmp(node->_data, y->_data)< 0) {
		y->_left= node;
	}
	else {
		y->_right= node;
	}
}


// insertion de donnée
template <class T>
void BST<T>::insert(T data) {
	// noeud à insérer
	Node<T> * new_node= gen_node(data);
	insert(new_node);
}


// suppression d'un sous-arbre
template <class T>
void BST<T>::delete_tree(Node<T> * node) {
	if (node!= NULL) {
		if (node->_left!= NULL) {
			delete_tree(node->_left);
		}
		if (node->_right!= NULL) {
			delete_tree(node->_right);
		}
		
		delete node;
		node= NULL;
	}
}


// vidage de tout l'arbre
template <class T>
void BST<T>::clear() {
	delete_tree(_root);
	_root= NULL;
}


// suppression d'un noeud
template <class T>
void BST<T>::remove(Node<T> * node) {
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
			y->_right->_parent= y;
		}
		transplant(node, y);
		y->_left= node->_left;
		y->_left->_parent= y;
	}
}

// suppression de data ; suppose qu'il ne peut pas y avoir la même valeur 2 fois dans l'arbre !
template <class T>
void BST<T>::remove(T data) {
	// on cherche le noeud correspodant à data
	Node<T> * node= search(data, true);
	// data n'est pas dans l'arbre, on sort
	if (node== NULL) {
		return;
	}
	remove(node);
}


// remplacement de sous-arbres : new_subtree va remplacer old_subtree
// new_subtree peut valoir NULL si l'on veut supprimer old_subtree
template <class T>
void BST<T>::transplant(Node<T> * old_subtree, Node<T> * new_subtree) {
	if (old_subtree->_parent== NULL) {
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


// rotation à gauche, cf https://en.wikipedia.org/wiki/Tree_rotation
template <class T>
void BST<T>::rotate_left(Node<T> * node) {
	if (node== NULL) {
		return;
	}
	
	Node<T> * pivot= node->_right;
	if (pivot== NULL) {
		return;
	}
	
	Node<T> * node_parent= node->_parent;
	bool is_node_left= node->is_left();
	node->set_right(pivot->_left);
	pivot->set_left(node);

	if (node== _root) {
		pivot->_parent= NULL;
		_root= pivot;
	}
	else {
		if (is_node_left) {
			node_parent->set_left(pivot);
		}
		else {
			node_parent->set_right(pivot);
		}
	}
}


// rotation à droite, cf https://en.wikipedia.org/wiki/Tree_rotation
template <class T>
void BST<T>::rotate_right(Node<T> * node) {
	if (node== NULL) {
		return;
	}
	
	Node<T> * pivot= node->_left;
	if (pivot== NULL) {
		return;
	}
	
	Node<T> * node_parent= node->_parent;
	bool is_node_left= node->is_left();
	node->set_left(pivot->_right);
	pivot->set_right(node);

	if (node== _root) {
		pivot->_parent= NULL;
		_root= pivot;
	}
	else {
		if (is_node_left) {
			node_parent->set_left(pivot);
		}
		else {
			node_parent->set_right(pivot);
		}
	}
}


// recherche de data dans le sous-arbre de sommet node
template <class T>
Node<T> * BST<T>::search(Node<T> * node, T data, bool exact_match) {
	while ((node!= NULL) && (_cmp(node->_data, data)!= 0)) {
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


// renvoie le tableau trié des valeurs des noeuds
template <class T>
std::vector<T> BST<T>::get_sorted_array() {
	std::vector<T> result;
	traversal(TraversalType::IN_ORDER, [&result](Node<T> * node){result.push_back(node->_data);});
	return result;
}


template <class T>
Node<T> * BST<T>::balance(std::vector<T> & sorted_array, int start, int end) {
	if (start> end) {
		return NULL;
	}

	int mid= (start+ end)/ 2;
	Node<T> * node= gen_node(sorted_array[mid]);
	node->_left= balance(sorted_array, start, mid- 1);
	node->_right= balance(sorted_array, mid+ 1, end);
	return node;
}


// balance l'arbre, ie minimise sa hauteur
template <class T>
void BST<T>::balance() {
	std::vector<T> sorted_array= get_sorted_array();
	_root= balance(sorted_array, 0, sorted_array.size()- 1);
}


template <class T>
void BST<T>::traversal(Node<T> * node, TraversalType tt, std::function<void(Node<T> *)> f) {
	if (node== NULL) {
		return;
	}

	if (tt== IN_ORDER) {
		traversal(node->_left, tt, f);
		f(node);
		traversal(node->_right, tt, f);
	}
	else if (tt== PRE_ORDER) {
		f(node);
		traversal(node->_left, tt, f);
		traversal(node->_right, tt, f);
	}
	else if (tt== POST_ORDER) {
		traversal(node->_left, tt, f);
		traversal(node->_right, tt, f);
		f(node);
	}
}


// traversée de l'arbre
template <class T>
void BST<T>::traversal(TraversalType tt, std::function<void(Node<T> *)> f) {
	traversal(_root, tt, f);
}


// surcharge <<
template <class T>
std::ostream & operator << (std::ostream & os, BST<T> & bst) {
	bst.traversal(IN_ORDER);
	return os;
}


template <class T>
void BST<T>::export_html(std::string html_path, Node<T> * node, float x, int y) {
	if (node== NULL) {
		return;
	}

	float x_factor= 1.0f;
	float y_factor= 0.4f;
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

	float x_text= 0.0f;
	float y_text= 0.0f;
	if (node->is_left()) {
		x_text= pt_x- 0.1f;
		y_text= pt_y+ 0.05f;
	}
	else {
		x_text= pt_x- 0.1f;
		y_text= pt_y;
	}
	f << "<text class=\"point_text_class\" x=\"" << x_text << "\" y=\"" << y_text << "\" >" << node->_data << "</text>\n";

	f << "<circle class=\"point_class\" cx=\"" << pt_x << "\" cy=\"" << pt_y << "\" r=\"" << 0.01f << "\" />\n";
	if (node->_left!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_left_x << "\" y2=\"" << pt_left_y << "\" />\n";
	}
	if (node->_right!= NULL) {
		f << "<line class=\"line_class\" x1=\"" << pt_x << "\" y1=\"" << pt_y << "\" x2=\"" << pt_right_x << "\" y2=\"" << pt_right_y << "\" />\n";
	}
	f.close();
	export_html(html_path, node->_left, left_x, y+ 1);
	export_html(html_path, node->_right, right_x, y+ 1);
}


// export html pour debug
template <class T>
void BST<T>::export_html(std::string html_path) {
	unsigned int svg_width= 1400;
	unsigned int svg_height= 1200;
	float view_xmin= -2.0f;
	float view_xmax= 2.0f;
	float view_ymin= 0.0f;
	float view_ymax= 2.0f;

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".point_text_class {fill: red; font-size: 0.05px;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" viewbox=\"" << view_xmin << " " << view_ymin << " " << view_xmax- view_xmin << " " << view_ymax- view_ymin << "\">\n";
	f.close();

	export_html(html_path, _root, 0.0f, 1);
	
	f.open(html_path, std::ios_base::app);
	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


// gardé pour mémoire mais très inefficace; le fichier pbm résultat peut devenir énorme

/*template <class T>
void BST<T>::draw(bool * bits, Node<T> * node, int i, int j, unsigned int width, unsigned int height, unsigned int n) {
	if (node== NULL) {
		return;
	}

	if (i< 0 || i>= width || j< 0 || j>= height) {
		std::cerr << "BST::draw probleme : i = " << i << " ; j = " << j << " ; width = " << width << " ; height = " << height << " ; n = " << n << "\n";
		return;
	}

	//std::cout << *node << " ; " << "i = " << i << " ; j = " << j << " ; width = " << width << " ; height = " << height <<  " ; n = " << n << "\n";

	bits[i+ j* width]= 1;
	
	draw(bits, node->_left, i- pow(2, n- 2- j), j+ 1, width, height, n);
	draw(bits, node->_right, i+ pow(2, n- 2- j), j+ 1, width, height, n);
}


template <class T>
void BST<T>::draw(std::string output) {
	unsigned int n= n_nodes();
	unsigned int width= pow(2, n)- 1;
	unsigned int height_= height();
	bool * bits= new bool[width* height_];
	for (int i=0; i<width* height_; ++i) {
		bits[i]= 0;
	}
	
	draw(bits, _root, int(width- 1)/ 2, 0, width, height_, n);
	
	std::ofstream f;
	f.open(output);
	f << "P1\n";
	f << width << " " << height_ << "\n";
	for (int j=0; j<height_; ++j) {
		for (int i=0; i<width; ++i) {
			f << bits[i+ j* width];
			if (i!= width- 1) {
				f << " ";
			}
		}
		f << "\n";
	}
	f.close();
	delete[] bits;
}
*/

#endif
