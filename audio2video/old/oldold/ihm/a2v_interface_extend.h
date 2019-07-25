
#ifndef A2V_INTERFACE_EXTEND_H
#define A2V_INTERFACE_EXTEND_H

#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Value_Slider.H>


const unsigned int TIME_GRAPH_POS_SIZE= 3;
const unsigned int TIME_GRAPH_MARGIN= 4;


class Table4X4 : public Fl_Scroll {
public:
	Fl_Float_Input *w[16];

	Table4X4(int X, int Y, int W, int H, const char*L=0);

};


class Vec3 : public Fl_Scroll {
public:
	Fl_Value_Slider *w[3];

	Vec3(int X, int Y, int W, int H, const char*L=0);
};


class Vec4 : public Fl_Scroll {
public:
	Fl_Float_Input *w[4];

	Vec4(int X, int Y, int W, int H, const char*L=0);
};


class PtIhm {
public:
	PtIhm();
	PtIhm(int x_, int y_);
	int x, y;
};

class TimeGraph : public Fl_Tile {
public:
	TimeGraph(int X, int Y, int W, int H, const char*L=0);
	void draw(void);
	int handle(int event);

	std::vector<PtIhm> pos;
	int selected_idx;
};


#endif
