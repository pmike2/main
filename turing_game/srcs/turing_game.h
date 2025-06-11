/*

A partir du jeu de plateau turing machine

*/

#ifndef TURING_GAME
#define TURING_GAME

#include <vector>
#include <map>
#include "math.h"


using vint = std::vector<uint>;

typedef uint (*test_function_ptr)(vint);

uint v0eq0(vint v);
uint v0eq2(vint v);
uint v1eq2(vint v);
uint v1eq3(vint v);
uint v0mod2(vint);
uint v1mod2(vint v);
uint v2mod2(vint v);
uint nbr1(vint v);
uint nbr3(vint v);
uint nbr4(vint);
uint v0cmpv1(vint v);
uint v0cmpv2(vint v);
uint v1cmpv2(vint v);
uint cmp_mod2(vint v);
uint n_pairs(vint v);
uint sum_mod2(vint v);
uint sum_v0v1_cmp6(vint v);
uint repeat(vint v);
uint repeat2(vint v);
uint sum_cmp6(vint v);
uint seq_croissant(vint v);
uint seq_croissant_decroissant(vint v);

const std::vector<test_function_ptr> TESTS {&v0eq0, &v0eq2, &v1eq2, &v1eq3, &v0mod2, &v1mod2, &v2mod2, &nbr1, &nbr3, &nbr4, 
	&v0cmpv1, &v0cmpv2, &v1cmpv2, &cmp_mod2, &n_pairs, &sum_mod2, &sum_v0v1_cmp6, &repeat, &repeat2, &sum_cmp6,
	&seq_croissant, &seq_croissant};

const std::vector<std::string> TESTS_DOCS {"V0 == 0 ou > 0", "V0 < 2 ou == 2 ou > 2", "V1 < 2 ou == 2 ou > 2",
	"V1 < 3 ou == 3 ou > 3", "V0 % 2 == 0 ou == 1", "V1 % 2 == 0 ou == 1", "V2 % 2 == 0 ou == 1", "n1 == 0 ou 1 ou 2 ou 3", 
	"n3 == 0 ou 1 ou 2 ou 3", "n4 == 0 ou 1 ou 2 ou 3", "v0 < v1 ou v0 == v1 ou v0 > v1", "v0 < v2 ou v0 == v2 ou v0 > v2",
	"V1 < V2 ou V1 == V2 ou V1 > V2", "npairs < nimpairs ou npairs > nimpairs", "npairs == 0 ou npairs == 1 ou npairs == 2",
	"sum paire ou sum impaire", "V0 + V1 < 6 ou V0 + V1 == 6 ou V0 + V1 > 6", "nidentiques == 0 ou == 1 ou == 2 ou == 3",
	"existe (2 ou +) identiques ou non", "V0 + V1 + V2 < 6 ou == 6 ou > 6", "suite croissante de 3 ou de 2 ou non",
	"suite croissante ou décroissante de 3 ou de 2 ou non"};

const uint VSIZE= 3;
const uint NVALUES= 5;
const uint NPOSSIBLESV= pow(NVALUES, VSIZE);
const uint N_FILTERS_PER_PROPOSITION= 3;
const std::string ALPHABET= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

struct enigma {
	vint _tests_idx;
	vint _solution;
};

std::string v2str(vint v);
uint v2val(vint v);
vint val2v(uint val);

std::vector<vint> compute_values();
bool tests_filters_unique_solution(enigma * e);
bool tests_filters_all_filters_needed(enigma * e);
enigma * find_enigma(uint n_tests);
void gen_game();

#endif
