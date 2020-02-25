
#ifndef SEQ_EXTEND_H
#define SEQ_EXTEND_H

#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Box.H>

#include "constantes_seq.h"


class SeqScroll : public Fl_Scroll {
public:
	Fl_Light_Button *w[ROWS* COLS];
	SeqScroll(int X, int Y, int W, int H, const char*L=0);
};


#endif
