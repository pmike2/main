#include "math.h"
#include <iostream>

#include "turing_game.h"


unsigned int v2val(unsigned int *v) {
	unsigned int val= 0;
	for (unsigned int i=VSIZE-1; i>=0; --i) {
		val+= v[i]* pow(NVALUES, (VSIZE- 1- i));
	}
	return val;
}


void val2v(unsigned int val, unsigned int *v) {
	unsigned int idx= 0;
	unsigned int r= 0;
	unsigned int q= val;
	for (unsigned int i=0; i<VSIZE; ++i) {
		v[i]= 0;
	}
	while (true) {
		r= q % NVALUES;
		v[idx++]= r;
		if (r== q) {
			break;
		}
		q= (q- r)/ NVALUES;
	}
}


unsigned int f1(unsigned int *v) {
	if (v[0] % 2 == 0) {
		return 0;
	}
	return 1;
}


unsigned int f2(unsigned int *v) {
	unsigned int n4= 0;
	for (unsigned int i=0; i<VSIZE; ++i) {
		if (v[i]== 4) {
			n4++;
		}
	}
	return n4;
}


unsigned int f3(unsigned int *v) {
	for (unsigned int i=0; i<VSIZE- 1; ++i) {
		for (unsigned int j=i+1; j<VSIZE; ++j) {
			if (v[i] == v[j]) {
				return 1;
			}
		}
	}
	return 0;
}


void compute_values(unsigned int *values) {
	unsigned int v[VSIZE];
	for (unsigned int idx_test=0; idx_test<NTESTS; ++idx_test) {
		for (unsigned int idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			val2v(idx_v, v);
			unsigned int res= TESTS[idx_test](v);
			//std::cout << idx_test << " ; " << idx_v << " ; " << res << "\n";
			values[idx_test][idx_v]= res;
		}
	}
}


void tests_list(unsigned int n_tests, unsigned int *tests_idx, unsigned int *v) {
	unsigned int values[NPOSSIBLESV][NTESTS];
	std::cout << "ok\n";
	compute_values((unsigned int *)values);
	bool v_unique= true;
	for (unsigned int idx_test=0; idx_test<n_tests; ++idx_test) {
		bool ok= true;
		unsigned int idx_v_2test= v2val(v);
		unsigned int val2test= values[tests_idx[idx_test]][idx_v_2test];
		for (unsigned int idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			unsigned int val= values[tests_idx[idx_test]][idx_v];
			if (val== val2test) {
				ok= false;
				break;
			}
		}
		if (!ok) {
			v_unique= false;
			break;
		}
	}

	if (v_unique) {
		std::cout << "unique !\n";
	}
	else {
		std::cout << "pas unique\n";
	}
}


void gen_game() {
	unsigned int tests_idx[3]= {0, 1, 2};
	unsigned int v[3]= {1, 4, 0};
	tests_list(3, tests_idx, v);
}
