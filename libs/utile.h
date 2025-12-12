
#ifndef UTILE_H
#define UTILE_H


#include <string>
#include <sys/time.h>
#include <vector>

#include <glm/glm.hpp>

#include "typedefs.h"


// test égalité stricte
bool number_equals_strict(number x, number y);

// test égalité avec marge
bool number_equals_epsilon(number x, number y, number threshold=1e-9);

// renvoie un double aléatoire entre x0 et x1
double rand_double(double x0, double x1);

// renvoie un number aléatoire entre x0 et x1
float rand_float(float x0, float x1);

number rand_number(number x0, number x1);

pt_type_3d rand_pt_3d(number xmin, number xmax, number ymin, number ymax, number zmin, number zmax);

// renvoie un int aléatoire entre x0 et x1 compris
int rand_int(int x0, int x1);

// renvoie un booléen aléatoire
bool rand_bool();

number rand_gaussian(number mean, number deviation);
pt_type rand_gaussian(pt_type mean, pt_type deviation);
;

// perlin noise
number * perlin_gradient(uint gradient_w, uint gradient_h);
number perlin_lerp(number a0, number a1, number w);
number perlin_dot_gradient(int ix, int iy, number x, number y, number * gradient, uint gradient_w, uint gradient_h);
number perlin(number x, number y, number * gradient, uint gradient_w, uint gradient_h);

// a terme remplacer ça par un appel à glm
void calculate_normal(number * coord1, number * coord2, number * coord3, number * norm);

uint diff_time_ms(struct timeval * after, struct timeval * before);
uint diff_time_ms_from_now(struct timeval * begin);
std::string current_date_time();

std::vector<std::string> list_files(std::string ch_dir, std::string ext="");
bool file_exists(const std::string filepath);
std::string basename(std::string s);
std::string dirname(std::string s);
std::pair<std::string, std::string> splitext(std::string s);

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

std::vector<std::string> split(const std::string& s, const std::string& delimiter);

std::string glm_to_string(pt_type v, int n_decimals=-1);
std::string glm_to_string(pt_type_3d v, int n_decimals=-1);

std::string get_cmd_output(std::string cmd);


template <typename T> void dfs(std::vector<T> & main_set, int i, std::vector<std::vector<T>> & res, std::vector<T> & subset) {
	if (i == main_set.size()) {
		res.push_back(subset);
		return;
	}

	subset.push_back(main_set[i]);
	dfs(main_set, i+ 1, res, subset);
	subset.pop_back();
	dfs(main_set, i+ 1, res, subset);
}


template <typename T> std::vector<std::vector<T> > subsets(std::vector<T> & main_set) {
	std::vector<T> subset;
	std::vector<std::vector<T>> res;
	dfs(main_set, 0, res, subset);
	return res;
}


#endif
