#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <type_traits>
#include <functional>


struct GridLocation {
	int _x, _y;
};
auto hash = [](const GridLocation& gl) { return gl._x ^ (gl._y << 4); };
auto equal = [](const GridLocation& gl1, const GridLocation& gl2) { return gl1._x == gl2._x && gl1._y == gl2._y; };
using GridLocationHash= std::unordered_set<GridLocation, decltype(hash), decltype(equal)>;


struct SquareGrid {
	static std::array<GridLocation, 4> DIRS;
	int _width, _height;
	GridLocationHash * _walls;


	SquareGrid(int width, int height) : _width(width), _height(height) {
		_walls= new GridLocationHash(10, hash, equal);
	}


	bool in_bounds(GridLocation id) const {
		return 0<= id._x && id._x< _width && 0<= id._y && id._y< _height;
	}


	bool passable(GridLocation id) const {
		return _walls->find(id)== _walls->end();
	}


	std::vector<GridLocation> neighbors(GridLocation id) const {
		std::vector<GridLocation> results;

		for (GridLocation dir : DIRS) {
			GridLocation next{id._x+ dir._x, id._y+ dir._y};
			if (in_bounds(next) && passable(next)) {
				results.push_back(next);
			}
		}

		if ((id._x+ id._y) % 2 == 0) {
			// see "Ugly paths" section for an explanation:
			std::reverse(results.begin(), results.end());
		}

		return results;
	}
};


std::array<GridLocation, 4> SquareGrid::DIRS = {
	// East, West, North, South
	GridLocation{1, 0}, GridLocation{-1, 0},
	GridLocation{0, -1}, GridLocation{0, 1}
};


struct GridWithWeights: SquareGrid {
	GridLocationHash * _forests;

	GridWithWeights(int w, int h): SquareGrid(w, h) {
		_forests= new GridLocationHash(10, hash, equal);
	}


	double cost(GridLocation from_node, GridLocation to_node) const {
		return _forests->find(to_node)!= _forests->end() ? 5 : 1;
	}
};


int main(int argc, char *argv[]) {
	SquareGrid * sq= new SquareGrid(4, 5);
	GridLocation gl= {2, 2};
	std::vector<GridLocation> v= sq->neighbors(gl);
	for (int i=0; i<v.size(); ++i) {
		std::cout << v[i]._x << " ; " << v[i]._y << "\n";
	}
	delete sq;

	return 0;
}
