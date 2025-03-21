
#include <iostream>
#include <functional>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bst.h"
#include "utile.h"


void test1() {
	BST<int> * bst= new BST<int>();

	bst->insert(6);
	bst->insert(2);
	bst->insert(1);
	bst->insert(4);
	bst->insert(3);
	bst->insert(5);
	bst->insert(7);

	//bst->rotate_left(bst->search(2));
	//bst->rotate_right(bst->search(6));

	std::cout << "height = " << bst->height() << "\n";
	std::cout << "n_nodes = " << bst->n_nodes() << "\n";
	std::cout << "max_imbalance = " << bst->max_imbalance() << "\n";

	std::cout << *bst;

	//bst->clear();

	//std::cout << *bst;

	//std::cout << "min = " << *bst->minimum(bst->search(1)) << "\n";

	//std::cout << "min = " << *bst->minimum() << "\nmax = " << *bst->maximum() << "\n";

	//std::cout << "succ = " << *bst->successor(bst->search(3)) << "\n";
	//std::cout << "pred = " << bst->predecessor(bst->search(1)) << "\n";

	//std::cout << "succ_leaf = " << *bst->successor_leaf(bst->search(3)) << "\n";
	//std::cout << "pred_leaf = " << bst->predecessor_leaf(bst->search(1)) << "\n";

	//bst->traversal(TraversalType::IN_ORDER);

	//bst->remove(4);

	//Node<int> * node= bst->search(7);
	//std::cout << node->is_left() << "\n";
	/*if (node!= NULL) {
		std::cout << *node << "\n";
	}
	else {
		std::cout << "NULL\n";
	}*/

	bst->export_html("../data/test1.html");

	bst->draw("../data/test1.pbm");

	delete bst;
}


void test2() {
	BST<glm::vec2> * bst= new BST<glm::vec2>(
		[](glm::vec2 a, glm::vec2 b) {
		if (a.x< b.x) return -1;
		else if (a.x> b.x) return 1;
		else return 0;
	});

	bst->insert(glm::vec2(0.0, 0.0));
	bst->insert(glm::vec2(1.01, -2.01));
	bst->insert(glm::vec2(-3.0, 5.0));
	bst->insert(glm::vec2(1.0, 3.0));

	//std::cout << *bst;

	//bst->traversal(TraversalType::IN_ORDER);

	//bst->remove(glm::vec2(0.0, 0.0));

	/*Node<glm::vec2> * node= bst->search(glm::vec2(0.99, 0.0), false);
	if (node!= NULL) {
		std::cout << *node << "\n";
	}
	else {
		std::cout << "NULL\n";
	}*/

	//bst->export_html("../data/test2.html");

	delete bst;
}


void test3() {
	BST<int> * bst= new BST<int>();
	for (int i=0; i<50; ++i) {
		bst->insert(rand_int(0, 1000));
	}

	std::vector<int> v= bst->get_sorted_array();
	for (auto x : v) {
		std::cout << x << " ";
	}
	std::cout << "\n";

	bst->export_html("../data/test3_avant.html");

	bst->balance();

	bst->export_html("../data/test3_apres.html");
}


int main() {
	srand(time(NULL));

	test1();
	//test2();
	//test3();

	return 0;
}
