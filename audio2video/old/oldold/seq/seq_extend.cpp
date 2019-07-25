
#include "seq_extend.h"

using namespace std;



SeqScroll::SeqScroll(int X, int Y, int W, int H, const char*L) : Fl_Scroll(X,Y,W,H,L) {  
	int cellw = 20;
	int cellh = 20;
	int xx = X, yy = Y;

	for ( int r=0; r<ROWS; r++ ) {
		for ( int c=0; c<COLS; c++ ) {
			Fl_Light_Button *but = new Fl_Light_Button(xx,yy,cellw,cellh);
			but->box(FL_BORDER_BOX);
			but->value(0);
			w[r* COLS+ c]= but;
			xx += cellw;
		}
		xx = X;
		yy += cellh;
	}
	end();
};
