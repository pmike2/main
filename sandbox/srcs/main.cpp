

#include <iostream>
#include <string>
#include <vector>

#include "bst.h"

using namespace std;



int main() {
	BSTNode<int> * bst= new BSTNode<int>();
	//BSTNode<int> * root= NULL;
	/*root= bst->insert(root, 2);
	bst->insert(root, 2);
	bst->insert(root, 4);
	bst->insert(root, 1);
	bst->insert(root, 8);
	bst->insert(root, 0);*/

	//bst->loop_in_order(root);

	bst->insert(8);
	bst->insert(2);
	bst->insert(7);
	bst->insert(4);
	delete bst;
	
	return 0;
}