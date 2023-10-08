#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include "voronoi.h"


// -------------------------------
bool cmp_site_x(glm::vec2 site1, glm::vec2 site2) {
	return site1.x< site2.x;
}


bool cmp_site_y(glm::vec2 site1, glm::vec2 site2) {
	return site1.y< site2.y;
}


// -------------------------------
// A utility function to create a new BST node
struct BeachLineNode* new_node(glm::vec2 site) {
	struct BeachLineNode* temp= new struct BeachLineNode;
	temp->_site= site;
	temp->_left= NULL;
	temp->_right= NULL;
	temp->_circle_event= NULL;
	return temp;
}
 
// A utility function to insert
struct BeachLineNode* insert_node(struct BeachLineNode* node, glm::vec2 site) {
	// If the tree is empty, return a new node
	if (node == NULL)
		return new_node(site);

	// Otherwise, recur down the tree
	if (cmp_site_x(site, node->_site)) {
		node->_left= insert_node(node->_left, site);
	}
	else {
		node->_right= insert_node(node->_right, site);
	}

	return node;
}
 
// Utility function to search a key in a BST
struct BeachLineNode* search_arc_above(struct BeachLineNode* root, glm::vec2 site) {
	if (root->_left== NULL) {
		return root;
	}

	if (cmp_site_x(site, root->_site)) {
		return search_arc_above(root->_right, site);
	}
	else {
		return search_arc_above(root->_left, site);
	}
}


struct BeachLineNode* rebalance(struct BeachLineNode* root) {

}


// -------------------------------
// TODO : mieux comparer 2 glm::vec2 !!!!!!!!!!!!!!!!!!!!!!
bool operator==(const Event& rhs, const Event& lhs) {
	if (rhs._type== CircleEvent) {
		return rhs._circle_lowest_point == lhs._circle_lowest_point;
	}
	else {
		return rhs._site == lhs._site;
	}
}


// -------------------------------
bool CmpSite::operator() (Event e1, Event e2) {
	if (e1._type== SiteEvent) {
		return cmp_site_y(e1._site, e2._site);
	}
	else if (e1._type== CircleEvent) {
		return cmp_site_y(e1._circle_lowest_point, e2._circle_lowest_point);
	}
	return false; // n'arrive jamais
}


// -------------------------------
Voronoi::Voronoi() {

}


Voronoi::Voronoi(std::vector<glm::vec2> sites) : _beachline_root(NULL) {
	for (unsigned int i=0; i<sites.size(); ++i) {
		Event e{SiteEvent, sites[i]};
		_queue.push(e);
	}
	while (!_queue.empty()) {
		Event e= _queue.top();
		//std::cout << glm::to_string(site) << " ";
		_queue.pop();

		if (e._type== SiteEvent) {
			handle_site_event(e);
		}
		else if (e._type== CircleEvent) {
			handle_circle_event(e);
		}
	}
	//std::cout << "\n";

}


Voronoi::~Voronoi() {

}


void Voronoi::handle_site_event(Event e) {
	if (_beachline_root== NULL) {
		insert_node(_beachline_root, e._site);
	}
	else {
		BeachLineNode* b= search_arc_above(_beachline_root, e._site);
		if (b->_circle_event!= NULL) {
			_queue.remove(*b->_circle_event);
		}
	}
}


void Voronoi::handle_circle_event(Event e) {
	
}

