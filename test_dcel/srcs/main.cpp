
#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	dcel->export_html("../data/test1.html");
	delete dcel;
}


int main() {
	test1();
	
	return 0;
}
