#include <iostream>
#include <sstream>

#include "utile.h"

#include "turing_game.h"


// fonctions de filtrage du jeu ----------------------------------------------------
uint v0eq0(vint v) {
	if (v[0]== 0) {
		return 0;
	}
	return 1;
}


uint v0eq2(vint v) {
	if (v[0]< 2) {
		return 0;
	}
	else if (v[0]== 2) {
		return 1;
	}
	return 2;
}


uint v1eq2(vint v) {
	if (v[1]< 2) {
		return 0;
	}
	else if (v[1]== 2) {
		return 1;
	}
	return 2;
}


uint v1eq3(vint v) {
	if (v[1]< 3) {
		return 0;
	}
	else if (v[1]== 3) {
		return 1;
	}
	return 2;
}


uint v0mod2(vint v) {
	if (v[0] % 2 == 0) {
		return 0;
	}
	return 1;
}


uint v1mod2(vint v) {
	if (v[1] % 2 == 0) {
		return 0;
	}
	return 1;
}


uint v2mod2(vint v) {
	if (v[2] % 2 == 0) {
		return 0;
	}
	return 1;
}



uint nbr1(vint v) {
	uint n1= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 1) {
			n1++;
		}
	}
	return n1;
}


uint nbr3(vint v) {
	uint n3= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 3) {
			n3++;
		}
	}
	return n3;
}


uint nbr4(vint v) {
	uint n4= 0;
	for (uint i=0; i<VSIZE; ++i) {
		if (v[i]== 4) {
			n4++;
		}
	}
	return n4;
}


uint v0cmpv1(vint v) {
	if (v[0]< v[1]) {
		return 0;
	}
	else if (v[0]== v[1]) {
		return 1;
	}
	return 2;
}


uint v0cmpv2(vint v) {
	if (v[0]< v[2]) {
		return 0;
	}
	else if (v[0]== v[2]) {
		return 1;
	}
	return 2;
}


uint v1cmpv2(vint v) {
	if (v[1]< v[2]) {
		return 0;
	}
	else if (v[1]== v[2]) {
		return 1;
	}
	return 2;
}


uint cmp_mod2(vint v) {
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


uint n_pairs(vint v) {
	uint n_pairs= 0;
	for (auto x : v) {
		if (x % 2== 0) {
			n_pairs++;
		}
	}
	return n_pairs;
}


uint sum_mod2(vint v) {
	uint s= 0;
	for (auto x : v) {
		s+= x;
	}
	return s % 2;
}


uint sum_v0v1_cmp6(vint v) {
	if (v[0]+ v[1]< 6) {
		return 0;
	}
	else if (v[0]+ v[1]== 6) {
		return 1;
	}
	return 2;
}


uint repeat(vint v) {
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


uint repeat2(vint v) {
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


uint sum_cmp6(vint v) {
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


uint seq_croissant(vint v) {
	if ((v[1]== v[0]+ 1) && (v[2]== v[1]+ 1)) {
		return 0;
	}
	else if ((v[1]== v[0]+ 1) || (v[2]== v[1]+ 1)) {
		return 1;
	}
	return 2;
}


uint seq_croissant_decroissant(vint v) {
	if (((v[1]== v[0]+ 1) && (v[2]== v[1]+ 1)) || ((v[1]== v[0]- 1) && (v[2]== v[1]- 1))) {
		return 0;
	}
	else if ((v[1]== v[0]+ 1) || (v[2]== v[1]+ 1) || (v[1]== v[0]- 1) || (v[2]== v[1]- 1)) {
		return 1;
	}
	return 2;
}


// --------------------------------------------------------------------------------------------------
// print vecteur
std::string v2str(vint v) {
	std::string s= "(";
	for (uint idx=0; idx<v.size(); ++idx) {
		s+= std::to_string(v[idx]);
		if (idx< v.size()- 1) {
			s+= " ; ";
		}
	}
	s+= ")";
	return s;
}


// passage vecteur -> int
uint v2val(vint v) {
	uint val= 0;
	
	for (int i=VSIZE-1; i>=0; --i) {
		val+= v[i]* pow(NVALUES, (VSIZE- 1- i));
	}
	return val;
}


// passage int -> vecteur
vint val2v(uint val) {
	vint v;
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


// calcul du résultat de chaque test pour chaque triplet possible
std::vector<vint > compute_values() {
	std::vector<vint > values;
	for (uint idx_test=0; idx_test<TESTS.size(); ++idx_test) {
		vint w;
		for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
			vint v= val2v(idx_v);
			uint res= TESTS[idx_test](v);
			w.push_back(res);
		}
		values.push_back(w);
	}
	return values;
}


// test d'un jeu de filtre pour voir s'il existe une solution unique
bool tests_filters_unique_solution(enigma * e) {
	std::vector<vint> values= compute_values();
	
	// on init un vecteur avec toutes les valeurs possibles puis on va filtrer
	vint possibles_v;
	for (uint idx_v=0; idx_v<NPOSSIBLESV; ++idx_v) {
		possibles_v.push_back(idx_v);
	}
	
	// contiendra les triplets solutions uniques
	vint uniques_v;

	// si sol unique existe, vaudra 0; sinon vaudra la taille du plus petit vecteur de solutions ayant le 
	// meme résultat pour tous les filtres
	uint min_identicals_size= 1000;
	
	while(true) {
		// recup du 1er triplet possible, que l'on supprime de possibles_v
		uint idx_v= possibles_v.back();
		possibles_v.pop_back();
		
		// contiendra toutes les solutions ayant les memes valeurs pour chaque filtre
		vint identicals;

		// recherche des solutions ayant les memes valeurs pour chaque filtre
		for (auto idx_v2 : possibles_v) {
			bool diff= false;
			for (uint idx_test=0; idx_test<e->_tests_idx.size(); ++idx_test) {
				uint val1= values[e->_tests_idx[idx_test]][idx_v];
				uint val2= values[e->_tests_idx[idx_test]][idx_v2];
				if (val1!= val2) {
					diff= true;
					break;
				}
			}
			if (!diff) {
				identicals.push_back(idx_v2);
			}
		}

		// si pas de triplet avec memes valeurs pour tous les filtres, alors on a une solution unique
		if (identicals.size()== 0) {
			uniques_v.push_back(idx_v);
		}
		
		if (identicals.size()< min_identicals_size) {
			min_identicals_size= identicals.size();
		}

		// suppression des identicals puisqu'ils ne peuvent pas être solution unique
		possibles_v.erase(std::remove_if(possibles_v.begin(), possibles_v.end(), [identicals](uint i){
			return (std::find(identicals.begin(), identicals.end(), i) != identicals.end());
		}), possibles_v.end());

		if (possibles_v.empty()) {
			break;
		}
	}

	// il n'y a pas de solution unique pour ce jeu de filtres
	if (uniques_v.empty()) {
		return false;
	}

	// il y a au moins une solution unique; on prend la 1ère
	e->_solution.clear();
	e->_solution= val2v(uniques_v[0]);
	return true;
}


// si on retire 1 ou plusieurs filtres est-ce qu'il y a encore une solution unique
bool tests_filters_all_filters_needed(enigma * e) {
	enigma * e2= new enigma;

	// tous les sous-ensembles d'indices possibles
	std::vector<std::vector<uint> > tests_idx_subsets= subsets<uint>(e->_tests_idx);
	for (auto subset : tests_idx_subsets) {
		// on ignore l'ensemble vide et l'ensemble total
		if (subset.size()== 0 || subset.size()== e->_tests_idx.size()) {
			continue;
		}

		e2->_tests_idx.clear();
		for (auto test_idx : subset) {
			e2->_tests_idx.push_back(test_idx);
		}
		if (tests_filters_unique_solution(e2)) {
			return false;
		}
	}

	delete e2;

	return true;
}


// recherche d'un jeu de filtres avec solution unique et tel que tous les filtres sont nécessaires
enigma * find_enigma(uint n_tests) {
	enigma * e= new enigma;

	// on itère tant qu'un jeu de filtre avec solution unique n'a pas été trouvé
	while (true) {
		e->_tests_idx.clear();
		for (uint i=0; i<n_tests; ++i) {
			while (true) {
				uint j= rand_int(0, TESTS.size()- 1);
				if (std::find(e->_tests_idx.begin(), e->_tests_idx.end(), j) == e->_tests_idx.end()) {
					e->_tests_idx.push_back(j);
					break;
				}
			}
		}
		
		bool filters_unique_solution= tests_filters_unique_solution(e);

		if (!filters_unique_solution) {
			continue;
		}

		bool filters_all_filters_needed= tests_filters_all_filters_needed(e);

		if (filters_unique_solution && filters_all_filters_needed) {
			break;
		}
	}

	return e;
}


// jeu
void gen_game() {
	const bool DEBUG= false;

	uint n_try= 0;
	bool stop= false;

	// recherche d'une énigme
	enigma * e= find_enigma(N_FILTERS);

	// affichage des docs des tests
	std::cout << "\n-----------------\n";
	for (uint idx_test=0; idx_test<e->_tests_idx.size(); ++idx_test) {
		std::cout << ALPHABET[idx_test] << " : " << TESTS_DOCS[e->_tests_idx[idx_test]] << "\n";
	}
	std::cout << "-----------------\n";
	
	if (DEBUG) {
		std::cout << "solution = " << v2str(e->_solution) << "\n";
	}

	while (true) {
		n_try++;
		std::cout << "\nEssai " << n_try << "\n\n";

		// lecture proposition -------------------------------------------------
		std::cout << "proposition ? (STOP pour quitter) ";
		std::string proposition_str;
		std::getline(std::cin, proposition_str);

		if (proposition_str == "STOP") {
			break;
		}

		std::stringstream proposition_ss(proposition_str);
		std::string proposition_s;
		vint proposition;
		while (proposition_ss >> proposition_s) {
			proposition.push_back(std::stoi(proposition_s));
		}

		// lecture tests à faire pour proposition -------------------------------
		for (uint i=0; i<N_FILTERS_PER_PROPOSITION; ++i) {
			std::cout << "filtre à tester ? (STOP pour quitter) ";
			std::string test_letter;
			std::cin >> test_letter;

			if (test_letter == "STOP") {
				stop= true;
				break;
			}

			uint filter_idx= (uint)(test_letter.c_str()[0]- 'A');

			// comparaison test entre proposition et solution
			uint proposition_result= TESTS[e->_tests_idx[filter_idx]](proposition);
			uint solution_result= TESTS[e->_tests_idx[filter_idx]](e->_solution);
			
			if (DEBUG) {
				std::cout << "filter = " << e->_tests_idx[filter_idx] << " ; proposition = " << v2str(proposition) << " ; prop result = " << proposition_result << " ; solution result = "  << solution_result << "\n";
			}

			if (proposition_result== solution_result) {
				std::cout << "OK\n";
			}
			else {
				std::cout << "NON\n";
			}
		}

		if (stop) {
			break;
		}

		// lecture solution joueur -----------------------------------------------
		std::cin.ignore(10000, '\n');
		std::cout << "solution ? (STOP pour quitter ; NON pour continuer à tester) ";
		std::string solution_str;
		std::getline(std::cin, solution_str);

		if (solution_str == "NON") {
			continue;
		}

		if (solution_str == "STOP") {
			break;
		}

		std::stringstream solution_ss(solution_str);
		std::string solution_s;
		vint solution;
		while (solution_ss >> solution_s) {
			solution.push_back(std::stoi(solution_s));
		}
		if (v2val(solution)== v2val(e->_solution)) {
			std::cout << "BRAVO ! nbr essais = " << n_try << "\n";
			break;
		}
		else {
			std::cout << "NON\n";
		}
	}

	std::cout << "\nFIN ; solution = " << v2str(e->_solution) << "\n";

	delete e;
}
