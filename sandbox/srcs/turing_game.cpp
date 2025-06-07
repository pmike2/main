#include <iostream>

#include "utile.h"

#include "turing_game.h"


uint v0eq0(std::vector<uint> v) {
	if (v[0]== 0) {
		return 0;
	}
	return 1;
}


uint v0eq2(std::vector<uint> v) {
	if (v[0]< 2) {
		return 0;
	}
	else if (v[0]== 2) {
		return 1;
	}
	return 2;
}


uint v1eq2(std::vector<uint> v) {
	if (v[1]< 2) {
		return 0;
	}
	else if (v[1]== 2) {
		return 1;
	}
	return 2;
}


uint v1eq3(std::vector<uint> v) {
	if (v[1]< 3) {
		return 0;
	}
	else if (v[1]== 3) {
		return 1;
	}
	return 2;
}


uint v0mod2(std::vector<uint> v) {
	if (v[0] % 2 == 0) {
		return 0;
	}
	return 1;
}


uint v1mod2(std::vector<uint> v) {
	if (v[1] % 2 == 0) {
		return 0;
	}
	return 1;
}


uint v2mod2(std::vector<uint> v) {
	if (v[2] % 2 == 0) {
		return 0;
	}
	return 1;
}



uint nbr1(std::vector<uint> v) {
	uint n1= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 1) {
			n1++;
		}
	}
	return n1;
}


uint nbr3(std::vector<uint> v) {
	uint n3= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 3) {
			n3++;
		}
	}
	return n3;
}


uint nbr4(std::vector<uint> v) {
	uint n4= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 4) {
			n4++;
		}
	}
	return n4;
}


uint v0cmpv1(std::vector<uint> v) {
	if (v[0]< v[1]) {
		return 0;
	}
	else if (v[0]== v[1]) {
		return 1;
	}
	return 2;
}


uint v0cmpv2(std::vector<uint> v) {
	if (v[0]< v[2]) {
		return 0;
	}
	else if (v[0]== v[2]) {
		return 1;
	}
	return 2;
}


uint v1cmpv2(std::vector<uint> v) {
	if (v[1]< v[2]) {
		return 0;
	}
	else if (v[1]== v[2]) {
		return 1;
	}
	return 2;
}


uint cmp_mod2(std::vector<uint> v) {
	uint n_pairs= 0;
	uint n_impairs= 0;
	for (auto x : v) {
		if (x % 2== 0) {
			n_pairs++;
		}
		else {
			n_impairs++;
		}
	}
	if (n_pairs< n_impairs) {
		return 0;
	}
	return 1;
}


uint n_pairs(std::vector<uint> v) {
	uint n_pairs= 0;
	for (auto x : v) {
		if (x % 2== 0) {
			n_pairs++;
		}
	}
	return n_pairs;
}


uint sum_mod2(std::vector<uint> v) {
	uint s= 0;
	for (auto x : v) {
		s+= x;
	}
	return s % 2;
}


uint sum_v0v1_cmp6(std::vector<uint> v) {
	if (v[0]+ v[1]< 6) {
		return 0;
	}
	else if (v[0]+ v[1]== 6) {
		return 1;
	}
	return 2;
}


uint repeat(std::vector<uint> v) {
	uint n_identicals= 0;
	for (uint i=0; i<VSIZE- 1; ++i) {
		for (uint j=i+1; j<VSIZE; ++j) {
			if (v[i] == v[j]) {
				n_identicals++;
			}
		}
	}
	return n_identicals;
}


uint repeat2(std::vector<uint> v) {
	uint n_identicals= 0;
	for (uint i=0; i<VSIZE- 1; ++i) {
		for (uint j=i+1; j<VSIZE; ++j) {
			if (v[i] == v[j]) {
				n_identicals++;
			}
		}
	}
	if (n_identicals== 0) {
		return 0;
	}
	return 1;
}


uint sum_cmp6(std::vector<uint> v) {
	uint s= 0;
	for (auto x : v) {
		s+= x;
	}
	if (s< 6) {
		return 0;
	}
	else if (s== 6) {
		return 1;
	}
	return 2;
}


uint seq_croissant(std::vector<uint> v) {
	if ((v[1]== v[0]+ 1) && (v[2]== v[1]+ 1)) {
		return 0;
	}
	else if ((v[1]== v[0]+ 1) || (v[2]== v[1]+ 1)) {
		return 1;
	}
	return 2;
}


uint seq_croissant_decroissant(std::vector<uint> v) {
	if (((v[1]== v[0]+ 1) && (v[2]== v[1]+ 1)) || ((v[1]== v[0]- 1) && (v[2]== v[1]- 1))) {
		return 0;
	}
	else if ((v[1]== v[0]+ 1) || (v[2]== v[1]+ 1) || (v[1]== v[0]- 1) || (v[2]== v[1]- 1)) {
		return 1;
	}
	return 2;
}


// --------------------------------------------------------------------------------------------------
std::string v2str(std::vector<uint> v) {
	std::string s= "(";
	for (auto x : v) {
		s+= std::to_string(x)+ " ; ";
	}
	s+= ")";
	return s;
}


uint v2val(std::vector<uint> v) {
	uint val= 0;
	
	for (int i=VSIZE-1; i>=0; --i) {
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
		v.push_back(0);
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


std::vector<std::vector<uint> > compute_values() {
	std::vector<std::vector<uint> > values;
	for (uint idx_test=0; idx_test<NTESTS; ++idx_test) {
		std::vector<uint> w;
		for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			std::vector<uint> v= val2v(idx_v);
			uint res= TESTS[idx_test](v);
			//std::cout << idx_test << " ; " << idx_v << " ; " << res << "\n";
			//values[idx_test][idx_v]= res;
			w.push_back(res);
		}
		values.push_back(w);
	}
	return values;
}


void tests_filters(std::vector<uint> tests_idx) {
	std::vector<std::vector<uint> > values= compute_values();
	
	std::vector<uint> possibles_v;
	for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
		possibles_v.push_back(idx_v);
	}
	
	std::vector<uint> uniques_v;

	uint min_identicals_size= 1000;
	while(true) {
		uint idx_v= possibles_v.back();
		possibles_v.pop_back();
		std::vector<uint> identicals;

		for (auto idx_v2 : possibles_v) {
			bool diff= false;
			for (uint idx_test=0; idx_test<tests_idx.size(); ++idx_test) {
				uint val1= values[tests_idx[idx_test]][idx_v];
				uint val2= values[tests_idx[idx_test]][idx_v2];
				if (idx_v== 124) {
					//std::cout << idx_v << " ; " << idx_v2 << " ; " << idx_test << " : " << val1 << " ; " << val2 << "\n";
				}
				if (val1!= val2) {
					diff= true;
					break;
				}
			}
			if (!diff) {
				identicals.push_back(idx_v2);
			}
		}

		if (identicals.size()== 0) {
			uniques_v.push_back(idx_v);
			std::vector<uint> vec= val2v(idx_v);
			std::cout << "unique : " << idx_v << " ; " << v2str(vec) << "\n";
		}
		
		if (identicals.size()< min_identicals_size) {
			min_identicals_size= identicals.size();
		}

		possibles_v.erase(std::remove_if(possibles_v.begin(), possibles_v.end(), [identicals](uint i){
			return (std::find(identicals.begin(), identicals.end(), i) != identicals.end());
		}), possibles_v.end());

		if (possibles_v.empty()) {
			break;
		}
	}

	std::cout << "min_identicals_size=" << min_identicals_size << "\n";
}


void gen_game() {
	uint n_tests= 3;
	std::vector<uint> tests_idx;
	for (uint i=0; i<n_tests; ++i) {
		while (true) {
			uint j= rand_int(0, NTESTS- 1);
			if (std::find(tests_idx.begin(), tests_idx.end(), j) == tests_idx.end()) {
				tests_idx.push_back(j);
				break;
			}
		}
	}
	std::cout << "filters = " << v2str(tests_idx) << "\n";
	tests_filters(tests_idx);
}
