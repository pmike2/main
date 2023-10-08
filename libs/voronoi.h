#ifndef VORONOI_H
#define VORONOI_H

#include <string>
#include <vector>
#include <queue>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"


typedef enum {CircleEvent, SiteEvent} EventType;


bool cmp_site_x(glm::vec2 site1, glm::vec2 site2);
bool cmp_site_y(glm::vec2 site1, glm::vec2 site2);

struct Event;

struct BeachLineNode {
	glm::vec2 _site;
	BeachLineNode * _left;
	BeachLineNode * _right;
	Event * _circle_event;
};

// A utility function to create a new BST node
struct BeachLineNode* new_node(glm::vec2 site);
 
// A utility function to insert
struct BeachLineNode* insert_node(struct BeachLineNode* node, glm::vec2 site);
 
// Utility function to search a key in a BST
struct BeachLineNode* search_arc_above(struct BeachLineNode* root, glm::vec2 site);

struct BeachLineNode* rebalance(struct BeachLineNode* root);


struct Event {
	EventType _type;
	glm::vec2 _site;
	glm::vec2 _circle_lowest_point;
	BeachLineNode * _leaf;
};


class CmpSite {
public:
	bool operator() (Event e1, Event e2);
};


template<typename T, class Container=std::vector<T>, class Compare=std::less<typename Container::value_type> >
class custom_priority_queue : public std::priority_queue<T, std::vector<T>, Compare> {
  public:

      bool remove(const T& value) {
          auto it = std::find(this->c.begin(), this->c.end(), value);
       
          if (it == this->c.end()) {
              return false;
          }
          if (it == this->c.begin()) {
              // deque the top element
              this->pop();
          }    
          else {
              // remove element and re-heap
              this->c.erase(it);
              std::make_heap(this->c.begin(), this->c.end(), this->comp);
         }
         return true;
     }
};


class Voronoi {
public:
	Voronoi();
	Voronoi(std::vector<glm::vec2> sites);
	~Voronoi();
	void handle_site_event(Event e);
	void handle_circle_event(Event e);


	DCEL _diagram;
	BeachLineNode * _beachline_root;
	//std::priority_queue<Event, std::vector<Event>, CmpSite> _queue;
	custom_priority_queue<Event, std::vector<Event>, CmpSite> _queue;
};

#endif
