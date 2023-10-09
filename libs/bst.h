#ifndef BST_H
#define BST_H

#include <iostream>


// ----------------------------------
/*template <class T>
class Node {
public:
	Node();
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
Node<T>::~Node() {
	_left= NULL;
	_right= NULL;
}*/


// ----------------------------------
template <class T>
class BSTNode {
public:
	BSTNode();
	BSTNode(T data);
	~BSTNode();
	//BSTNode * insert(BSTNode * root, T data);
	void insert(T data);
	void loop_in_order(BSTNode * root);


	T _data;
	BSTNode * _left;
	BSTNode * _right;
};


template <class T>
BSTNode<T>::BSTNode() {
	//_data= NULL;
	_left= NULL;
	_right= NULL;
}


template <class T>
BSTNode<T>::BSTNode(T data) {
	_data= data;
	_left= NULL;
	_right= NULL;
}


template <class T>
BSTNode<T>::~BSTNode() {

}


/*template <class T>
BSTNode<T> * BSTNode<T>::insert(BSTNode<T> * root, T data) {
	if (root== NULL) {
		return new BSTNode<T>(data);
	}

	if (data> root->_data) {
		root->_right= insert(root->_right, data);
	}
	else if (data< root->_data) {
		root->_left= insert(root->_left, data);
	}
	return root;
}*/


template <class T>
void BSTNode<T>::insert(T data) {
	BSTNode<T>* node = new BSTNode<T>(data);
	if (_data== NULL) {
		_data = data;
		return;
	}
	
	BSTNode<T>* prev = NULL;
	BSTNode<T>* temp = this;
	while (temp) {
		if (temp->_data > data) {
			prev = temp;
			temp = temp->_left;
		}
		else if (temp->_data < data) {
			prev = temp;
			temp = temp->_right;
		}
	}
	
	if (prev->_data > data) {
		prev->_left = node;
	}
	else {
		prev->_right = node;
	}
}


template <class T>
void BSTNode<T>::loop_in_order(BSTNode<T> * root) {
	if (root== NULL) {
		std::cout << "\n";
		return;
	}
	loop_in_order(root->_left);
	std::cout << root->_data << " ";
	loop_in_order(root->_right);
}

#endif
