
#include <iostream>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bst.h"


void test1() {
	BST<int> * bst= new BST<int>();

	bst->insert(4);
	bst->insert(8);
	bst->insert(2);
	bst->insert(3);
	bst->insert(1);
	bst->insert(7);

	bst->traversal(TraversalType::IN_ORDER);

	//bst->remove(7);

	//bst->traversal(TraversalType::IN_ORDER);

	/*Node<int> * node= bst->search(4);
	if (node!= NULL) {
		std::cout << node->_data << "\n";
	}
	else {
		std::cout << "NULL\n";
	}*/

	/*std::cout << bst->_root->_data << "\n";
	std::cout << bst->_root->_left->_data << "\n";
	std::cout << bst->_root->_right->_data << "\n";*/

	bst->export_html("../data/test1.html");

	delete bst;
}


void test2() {
	std::function<int(glm::vec2, glm::vec2)> f= [](glm::vec2 a, glm::vec2 b) {
		if (a.x< b.x) return -1;
		else if (a.x> b.x) return 1;
		else return 0;
	};
	BST<glm::vec2> * bst= new BST<glm::vec2>(f);

	bst->insert(glm::vec2(0.0, 0.0));
	bst->insert(glm::vec2(1.01, -2.01));
	bst->insert(glm::vec2(-3.0, 5.0));
	bst->insert(glm::vec2(1.0, 3.0));

	bst->traversal(TraversalType::IN_ORDER, [](glm::vec2 v){std::cout << glm::to_string(v) << " ";});

	//bst->remove(glm::vec2(0.0, 0.0));

	//bst->traversal(TraversalType::IN_ORDER, [](glm::vec2 v){std::cout << glm::to_string(v) << " ";});

	/*Node<glm::vec2> * node= bst->search(glm::vec2(1.01, -2.01));
	if (node!= NULL) {
		std::cout << glm::to_string(node->_data) << "\n";
	}
	else {
		std::cout << "NULL\n";
	}*/

	bst->export_html("../data/test2.html", [](glm::vec2 v){return glm::to_string(v);});

	delete bst;
}


int main() {
	//test1();
	test2();
	
	return 0;
}
