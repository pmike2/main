#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const int COURT= 0;
const int LONG= 1;
int SEGMENTS[]= {LONG, LONG, LONG, LONG, COURT, COURT, COURT, LONG, LONG, COURT, COURT, LONG, COURT, LONG, COURT, COURT, LONG};


unsigned long long ipow(int base, int exp) {
	unsigned long long result= 1ULL;
	unsigned long long lbase= (unsigned long long)base;
	while (exp) {
		if (exp & 1)
			result *= lbase;
		exp >>= 1;
		lbase *= lbase;
	}
	return result;
}


void cube_idx2xyz(int idx, int* res) {
	int x= idx % 3;
	int y= ((idx- x)/ 3) % 3;
	int z= (idx- x- 3* y)/ 9;
	res[0]= x;
	res[1]= y;
	res[2]= z;
}


int cube_xyz2idx(int x, int y, int z) {
	return x+ 3* y+ 9* z;
}

/*
int get_digits(unsigned long long number, int base, int* res) {
	unsigned long long n= number;
	int idx= 0;
	while (n) {
		res[idx]= n % 4;
		n/= base;
		idx++;
	}
	return idx;
}
*/

int get_digits_4(unsigned long long number, int* res) {
	unsigned long long n= number;
	int idx= 0;
	while (n) {
		res[idx]= n & 3;
		n>>= 2;
		idx++;
	}
	return idx;
}


int next_indices(int idx, char* instr, int seg_type, int* res) {
	int coords[6];
	int xyz[3]; int x, y, z;
	int eps;
	int i;
	
	for (i=0; i<6; ++i)
		coords[i]= 0;

	cube_idx2xyz(idx, xyz);
	x= xyz[0];
	y= xyz[1];
	z= xyz[2];
	
	if (instr[1]== '-') {
		eps= -1;
	}
	else if (instr[1]== '+') {
		eps= 1;
	}
	
	if (instr[0]== 'x') {
		coords[0]= x+ eps;
		coords[1]= y;
		coords[2]= z;
		if (seg_type== LONG) {
			coords[3]= x+ 2* eps;
			coords[4]= y;
			coords[5]= z;
		}
	}
	else if (instr[0]== 'y') {
		coords[0]= x;
		coords[1]= y+ eps;
		coords[2]= z;
		if (seg_type== LONG) {
			coords[3]= x;
			coords[4]= y+ 2* eps;
			coords[5]= z;
		}
	}
	else if (instr[0]== 'z') {
		coords[0]= x;
		coords[1]= y;
		coords[2]= z+ eps;
		if (seg_type== LONG) {
			coords[3]= x;
			coords[4]= y;
			coords[5]= z+ 2* eps;
		}
	}
	
	for (i=0; i<6; ++i) {
		if ((coords[i]< 0) || (coords[i]> 2)) {
			return -1;
		}
	}
	
	if (seg_type== COURT) {
		res[0]= cube_xyz2idx(coords[0], coords[1], coords[2]);
		return 1;
	}
	else {
		res[0]= cube_xyz2idx(coords[0], coords[1], coords[2]);
		res[1]= cube_xyz2idx(coords[3], coords[4], coords[5]);
		return 2;
	}
}


void next_instruction(char* instr, int digit, char* res) {
	if (instr[0]== 'x') {
		if (digit== 0) strcpy(res, "y+");
		else if (digit== 1) strcpy(res, "y-");
		else if (digit== 2) strcpy(res, "z+");
		else if (digit== 3) strcpy(res, "z-");
	}
	else if (instr[0]== 'y') {
		if (digit== 0) strcpy(res, "x+");
		else if (digit== 1) strcpy(res, "x-");
		else if (digit== 2) strcpy(res, "z+");
		else if (digit== 3) strcpy(res, "z-");
	}
	else if (instr[0]== 'z') {
		if (digit== 0) strcpy(res, "x+");
		else if (digit== 1) strcpy(res, "x-");
		else if (digit== 2) strcpy(res, "y+");
		else if (digit== 3) strcpy(res, "y-");
	}
}


int test_compt(unsigned long long compt, int* indices0, int n_indices0, char* instr0) {
	int current_idx= indices0[n_indices0- 1];
	char instructions[32][3];
	int cube[27];
	int i, idx_seg, n_indices;
	int digits[32];
	int digit, n_digits;
	int compt_instr= 0;
	char instr[3];
	int indices[2];
	int last_pos[3];

	strcpy(instructions[0], instr0);
	
	for (i=0; i<27; ++i)
		cube[i]= 0;

	for (i=0; i<n_indices0; ++i)
		cube[indices0[i]]= 1;
	
	//n_digits= get_digits(compt, 4, digits);
	n_digits= get_digits_4(compt, digits);
	
	for (idx_seg=0; idx_seg<sizeof(SEGMENTS)/ sizeof(SEGMENTS[0])- 1; ++idx_seg) {
		if (idx_seg>= n_digits) {
			digit= 0;
		}
		else {
			digit= digits[idx_seg];
		}
		
		next_instruction(instructions[compt_instr], digit, instr);
		strcpy(instructions[compt_instr+ 1], instr);
		compt_instr++;
		n_indices= next_indices(current_idx, instr, SEGMENTS[idx_seg+ 1], indices);
		if (n_indices== -1) {
			
			/*printf("sortie\n");
			for (i=0; i<compt_instr+ 1; ++i)
				printf("%s ", instructions[i]);
			printf("\n");*/
			
			return 0;
		}
		for (i=0; i<n_indices; ++i) {
			if (cube[indices[i]]== 1) {

				/*printf("ecrase\n");
				for (i=0; i<compt_instr+ 1; ++i)
					printf("%s ", instructions[i]);
				printf("\n");*/
			
				return 0;
			}
			cube[indices[i]]= 1;
		}
		current_idx= indices[n_indices- 1];
	}
	
	printf("solution !\n");
	for (i=0; i<compt_instr+ 1; ++i)
		printf("%s ", instructions[i]);
	printf("\n");
	cube_idx2xyz(current_idx, last_pos);
	printf("dernier idx = %d ; x = %d ; y = %d ; z = %d\n", current_idx, last_pos[0], last_pos[1], last_pos[2]);
	
	return 1;
}


void main_loop() {
	int xy0[3][2]= {{0, 0}, {1, 0}, {1, 1}};
	int i, j, n_indices, n_indices0, idx0;
	char instr0[3];
	int indices0[3];
	int indices[2];
	unsigned long long compt;
	unsigned long long compt_max= ipow(4, sizeof(SEGMENTS)/ sizeof(SEGMENTS[0])- 1);
	
	for (i=0; i<3; ++i) {
		strcpy(instr0, "z+");
		idx0= cube_xyz2idx(xy0[i][0], xy0[i][1], 0);
		indices0[0]= idx0;
		n_indices= next_indices(idx0, instr0, SEGMENTS[0], indices);
		for (j=0; j<n_indices; ++j) {
			indices0[j+ 1]= indices[j];
		}
		n_indices0= n_indices+ 1;
		
		for (compt=0; compt<compt_max; ++compt) {
			if (compt % 10000000== 0) {
				printf("%llu\n", compt);
			}
			if (test_compt(compt, indices0, n_indices0, instr0)) {
				printf("OK %d %d %llu\n", xy0[i][0], xy0[i][1], compt);
			}
		}
	}
}


// ---------------------------------------------------------------------------------------
void test0() {
	unsigned long long res;
	int base= 4;
	//int exp= 16;
	int exp= sizeof(SEGMENTS)/ sizeof(SEGMENTS[0])- 1;
	res= ipow(base, exp);
	printf("%d %llu\n", exp, res);
}


void test1() {
	int idx= 9;
	int res[3];
	cube_idx2xyz(idx, res);
	printf("%d %d %d\n", res[0], res[1], res[2]);
}


void test2() {
	int x= 1;
	int y= 2;
	int z= 2;
	int res= cube_xyz2idx(x, y, z);
	printf("%d\n", res);
}


/*void test3() {
	unsigned long n= 4294967295;
	int b= 4;
	int res[20];
	int k= get_digits(n, b, res);
	int i;
	for (i=0; i<k; ++i)
		printf("%d ; ", res[i]);
	printf("\n");
}*/


void test3bis() {
	//unsigned long n= 4294967295;
	unsigned long n= 7;
	int res[20];
	int k= get_digits_4(n, res);
	int i;
	for (i=0; i<k; ++i)
		printf("%d ; ", res[i]);
	printf("\n");
}


void test4() {
	int idx= 4;
	char* instr= "z+";
	int seg_type= LONG;
	int res[2];
	int n= next_indices(idx, instr, seg_type, res);
	int i;
	printf("n=%d ; ", n);
	for (i=0; i<n; ++i)
		printf("%d ; ", res[i]);
	printf("\n");
}


void test5() {
	char* instr="x+";
	int digit= 2;
	char res[3];
	next_instruction(instr, digit, res);
	printf("%s\n", res);
}


void test6() {
	unsigned long long compt= 124;
	int indices0[]= {0, 1, 2};
	int n_indices0= 3;
	char* instr0= "x+";
	int res= test_compt(compt, indices0, n_indices0, instr0);
	printf("%d\n", res);
}


void test7() {
	int xy0[3][2]= {{0, 0}, {1, 0}, {1, 1}};
	int i, j, n_indices, n_indices0, idx0;
	char instr0[3];
	int indices0[3];
	int indices[2];
	unsigned long long compt= 647158700;
	//unsigned long long compt= 2760964494;
	int res;

	strcpy(instr0, "z+");
	i= 0;
	idx0= cube_xyz2idx(xy0[i][0], xy0[i][1], 0);
	indices0[0]= idx0;
	n_indices= next_indices(idx0, instr0, SEGMENTS[0], indices);
	for (j=0; j<n_indices; ++j) {
		indices0[j+ 1]= indices[j];
	}
	n_indices0= n_indices+ 1;
	
	res= test_compt(compt, indices0, n_indices0, instr0);
	printf("%d\n", res);
}


// ---------------------------------------------------------------------------------------
int main() {
	printf("hello\n");
	
	//test0();
	//test1();
	//test2();
	//test3();
	//test3bis();
	//test4();
	//test5();
	//test6();
	//test7();
	
	main_loop();
	
	return 0;
}
