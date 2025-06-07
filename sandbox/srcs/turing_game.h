
#ifndef TURING_GAME
#define TURING_GAME

#include <vector>
#include "math.h"


using uint = uint;

typedef uint (*TEST_TYPE)(std::vector<uint>);

const uint VSIZE= 3;
const uint NVALUES= 5;
const uint NTESTS= 22;
const uint NPOSSIBLESV= pow(NVALUES, VSIZE);


uint v0eq0(std::vector<uint> v);
uint v0eq2(std::vector<uint> v);
uint v1eq2(std::vector<uint> v);
uint v1eq3(std::vector<uint> v);
uint v0mod2(std::vector<uint>);
uint v1mod2(std::vector<uint> v);
uint v2mod2(std::vector<uint> v);
uint nbr1(std::vector<uint> v);
uint nbr3(std::vector<uint> v);
uint nbr4(std::vector<uint>);
uint v0cmpv1(std::vector<uint> v);
uint v0cmpv2(std::vector<uint> v);
uint v1cmpv2(std::vector<uint> v);
uint cmp_mod2(std::vector<uint> v);
uint n_pairs(std::vector<uint> v);
uint sum_mod2(std::vector<uint> v);
uint sum_v0v1_cmp6(std::vector<uint> v);
uint repeat(std::vector<uint> v);
uint repeat2(std::vector<uint> v);
uint sum_cmp6(std::vector<uint> v);
uint seq_croissant(std::vector<uint> v);
uint seq_croissant_decroissant(std::vector<uint> v);

const TEST_TYPE TESTS[NTESTS]= {&v0eq0, &v0eq2, &v1eq2, &v1eq3, &v0mod2, &v1mod2, &v2mod2, &nbr1, &nbr3, &nbr4, 
&v0cmpv1, &v0cmpv2, &v1cmpv2, &cmp_mod2, &n_pairs, &sum_mod2, &sum_v0v1_cmp6, &repeat, &repeat2, &sum_cmp6, &seq_croissant, &seq_croissant};

std::string v2str(std::vector<uint> v);
uint v2val(std::vector<uint> v);
std::vector<uint> val2v(uint val);

std::vector<std::vector<uint> > compute_values();
void tests_filters(std::vector<uint> tests_idx);
void gen_game();

#endif
