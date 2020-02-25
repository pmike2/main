
#include "a2v_interface_extend.h"

using namespace std;


Table4X4::Table4X4(int X, int Y, int W, int H, const char*L) : Fl_Scroll(X,Y,W,H,L) {
	type(0); // pour supprimer les scrollbars
	int cellw = 80;
	int cellh = 25;
	int xx = X, yy = Y;

	for ( int r=0; r<4; r++ ) {
		for ( int c=0; c<4; c++ ) {
			Fl_Float_Input *in = new Fl_Float_Input(xx,yy,cellw,cellh);
			in->box(FL_BORDER_BOX);
			in->value("0.00");
			w[r* 4+ c] = in;
			
			xx += cellw;
		}
		xx = X;
		yy += cellh;
	}
	end();
}


Vec3::Vec3(int X, int Y, int W, int H, const char*L) : Fl_Scroll(X,Y,W,H,L) {  
	type(0); // pour supprimer les scrollbars
	int cellw = 80;
	int cellh = 25;
	int xx = X, yy = Y;

	for ( int c=0; c<3; c++ ) {
		Fl_Value_Slider *in= new Fl_Value_Slider(xx,yy,cellw,cellh);
		in->type(1);
        in->minimum(0.0);
        in->maximum(1.0);
        in->step(0.05);
		in->value(0.5);
		in->when(FL_WHEN_RELEASE);
		w[c] = in;
		
		yy += cellh;
	}
	end();
}


Vec4::Vec4(int X, int Y, int W, int H, const char*L) : Fl_Scroll(X,Y,W,H,L) {  
	type(0); // pour supprimer les scrollbars
	int cellw = 80;
	int cellh = 25;
	int xx = X, yy = Y;

	for ( int c=0; c<4; c++ ) {
		Fl_Float_Input *in = new Fl_Float_Input(xx,yy,cellw,cellh);
		in->box(FL_BORDER_BOX);
		in->value("0.00");
		w[c] = in;
		
		yy += cellh;
	}
	end();
}


PtIhm::PtIhm() {
	x= 0;
	y= 0;
}

PtIhm::PtIhm(int x_, int y_) {
	x= x_;
	y= y_;
}


TimeGraph::TimeGraph(int X, int Y, int W, int H, const char*L) : Fl_Tile(X, Y, W, H, L) {
	pos.push_back(PtIhm(x()+ TIME_GRAPH_MARGIN, y()+ h()- TIME_GRAPH_MARGIN));
	pos.push_back(PtIhm(x()+ w()- TIME_GRAPH_MARGIN, y()+ TIME_GRAPH_MARGIN));
	selected_idx= -1;
	//end();
}


void TimeGraph::draw(void) {
	unsigned int i;

	fl_push_clip(x(), y(), w(), h());
	fl_color(FL_BLACK);
	fl_rectf(x(), y(), w(), h());
	fl_color(FL_RED);
	for (i=0; i<pos.size(); i++)
		fl_rectf(pos[i].x- TIME_GRAPH_POS_SIZE, pos[i].y- TIME_GRAPH_POS_SIZE, 2* TIME_GRAPH_POS_SIZE+ 1, 2* TIME_GRAPH_POS_SIZE+ 1);
	for (i=0; i<pos.size()- 1; i++)
		fl_line(pos[i].x, pos[i].y, pos[i+ 1].x, pos[i+ 1].y);
	fl_pop_clip();
}


int TimeGraph::handle(int event) {
	// bizarre il faut ajuster les coord souris pour tomber pile
	int mx= Fl::event_x()- 1;
	int my= Fl::event_y()- 3;

	switch(event) {
		case FL_PUSH:
			selected_idx= -1;
			for (unsigned int i=0; i<pos.size(); i++) {
				//cout << i << " : pos=" << pos[i].x << ";" << pos[i].y << " mouse=" << mx << ";" << my << endl;
				if ((abs(mx- pos[i].x)<= TIME_GRAPH_POS_SIZE) && (abs(my- pos[i].y)<= TIME_GRAPH_POS_SIZE)) {
					selected_idx= i;
					break;
				}
			}
			if (Fl::event_state()== FL_BUTTON3) {
				if (selected_idx== -1) {
					int insert_idx;
					for (insert_idx=0; insert_idx<pos.size(); insert_idx++)
						if (pos[insert_idx].x> mx)
							break;
					pos.insert(pos.begin()+ insert_idx, PtIhm(mx, my));
				}
				else if ((selected_idx!= 0) && (selected_idx!= pos.size()- 1)) {
					pos.erase(pos.begin()+ selected_idx);
					selected_idx= -1;
				}
				redraw();
			}
			return 1;
		case FL_DRAG:
			if (selected_idx!= -1) {
				pos[selected_idx].x= mx;
				pos[selected_idx].y= my;
				if ((selected_idx< pos.size()- 1) && (pos[selected_idx].x> pos[selected_idx+ 1].x))
					pos[selected_idx].x= pos[selected_idx+ 1].x;
				if ((selected_idx> 0) && (pos[selected_idx].x< pos[selected_idx- 1].x))
					pos[selected_idx].x= pos[selected_idx- 1].x;
				if (selected_idx== 0)
					pos[selected_idx].x= x()+ TIME_GRAPH_MARGIN;
				if (selected_idx== pos.size()- 1)
					pos[selected_idx].x= x()+ w()- TIME_GRAPH_MARGIN;
				if (pos[selected_idx].y< y())
					pos[selected_idx].y= y()+ TIME_GRAPH_MARGIN;
				if (pos[selected_idx].y> y()+ h())
					pos[selected_idx].y= y()+ h()- TIME_GRAPH_MARGIN;
				
				redraw();
			}
			return 1;
		case FL_RELEASE:
			selected_idx= -1;
			return 1;
		default:
			return Fl_Widget::handle(event);
	}
}

