
#ifndef TURING_GAME
#define TURING_GAME

typedef unsigned int (*TEST_TYPE)(unsigned int *);

const unsigned int VSIZE= 3;
const unsigned int NVALUES= 5;
const unsigned int NTESTS= 3;
const unsigned int NPOSSIBLESV= pow(NVALUES, VSIZE);

unsigned int v2val(unsigned int *v);
void val2v(unsigned int val, unsigned int *v);

unsigned int f1(unsigned int *v);
unsigned int f2(unsigned int *v);
unsigned int f3(unsigned int *v);

//const unsigned int (*(tests[NTESTS]))(unsigned int *) = {f1, f2, f3};
const TEST_TYPE TESTS[3]= {&f1, &f2, &f3};

void compute_values(unsigned int *values);
void tests_list(unsigned int n_tests, unsigned int *tests_idx, unsigned int *v);
void gen_game();

#endif
