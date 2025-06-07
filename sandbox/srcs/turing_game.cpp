#include "math.h"
#include <iostream>

#include "turing_game.h"


uint v2val(std::vector<uint> v) {
	uint val= 0;
	for (uint i=VSIZE-1; i>=0; --i) {
		val+= v[i]* pow(NVALUES, (VSIZE- 1- i));
	}
	return val;
}


std::vector<uint> val2v(uint val) {
	std::vector<uint> v;
	uint idx= 0;
	uint r= 0;
	uint q= val;
	for (uint i=0; i<VSIZE; ++i) {
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
	return v;
}


uint f1(std::vector<uint> v) {
	if (v[0] % 2 == 0) {
		return 0;
	}
	return 1;
}


uint f2(std::vector<uint> v) {
	uint n4= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 4) {
			n4++;
		}
	}
	return n4;
}


uint f3(std::vector<uint> v) {
	for (uint i=0; i<VSIZE- 1; ++i) {
		for (uint j=i+1; j<VSIZE; ++j) {
			if (v[i] == v[j]) {
				return 1;
			}
		}
	}
	return 0;
}


std::vector<std::vector<uint> > compute_values() {
	std::vector<std::vector<uint> > values;
	for (uint idx_test=0; idx_test<NTESTS; ++idx_test) {
		std::vector<uint> v;
		for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			std::vector<uint> v= val2v(idx_v);
			uint res= TESTS[idx_test](v);
			//std::cout << idx_test << " ; " << idx_v << " ; " << res << "\n";
			//values[idx_test][idx_v]= res;
			v.push_back(res);
		}
		values.push_back(v);
	}
	return values;
}


void tests_list(std::vector<uint> tests_idx, std::vector<uint> v) {
	std::vector<std::vector<uint> > values= compute_values();
	bool v_unique= true;
	for (uint idx_test=0; idx_test<tests_idx.size(); ++idx_test) {
		bool ok= true;
		uint idx_v_2test= v2val(v);
		uint val2test= values[tests_idx[idx_test]][idx_v_2test];
		for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			uint val= values[tests_idx[idx_test]][idx_v];
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
	std::vector<uint> tests_idx= {0, 1, 2};
	std::vector<uint> v= {1, 4, 0};
	tests_list(tests_idx, v);
}
