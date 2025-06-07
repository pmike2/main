
#ifndef TURING_GAME
#define TURING_GAME

#include <vector>

using uint = uint;

typedef uint (*TEST_TYPE)(std::vector<uint>);

const uint VSIZE= 3;
const uint NVALUES= 5;
const uint NTESTS= 3;
const uint NPOSSIBLESV= pow(NVALUES, VSIZE);

uint v2val(std::vector<uint> v);
std::vector<uint> val2v(uint val);

uint f1(std::vector<uint>);
uint f2(std::vector<uint>);
uint f3(std::vector<uint>);

const TEST_TYPE TESTS[3]= {&f1, &f2, &f3};

std::vector<std::vector<uint> > compute_values();
void tests_list(std::vector<uint> tests_idx, std::vector<uint> v);
void gen_game();

#endif
