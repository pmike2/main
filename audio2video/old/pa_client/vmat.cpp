
#include "vmat.h"

using namespace std;


VMat::VMat(){
	unsigned int i;
	
	alpha= 1.;
	is_active= true;
	
	for (i=0; i<16; i++)
		mat[i]= 0.;
	mat[0]= 1.;
	mat[5]= 1.;
	mat[10]= 1.;
	mat[15]= 1.;
}


void VMat::anim() {
	if (!is_active)
		return;
	
	//gram_schmidt(mat);
}


void VMat::print() {
	unsigned int i;

	cout << "VMat -----------------------------" << endl;
	cout << "current_values.mat= (attention transposee)" << endl;
	for (i=0; i<16; i++) {
		cout << mat[i];
		if (i!= 15)
			cout << " , ";
		if ((i+ 1)% 4== 0)
			cout << endl;
	}
	cout << "is_active=" << is_active << endl;
	cout << "-----------------------------" << endl;
}
