
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

// renvoie un float aléatoire entre x0 et x1
float rand_float(float x0, float x1);

number rand_number(number x0, number x1);

// renvoie un int aléatoire entre x0 et x1 compris
int rand_int(int x0, int x1);

// renvoie un booléen aléatoire
bool rand_bool();

// perlin noise
float perlin_lerp(float a0, float a1, float w);
float perlin_dot_gradient(int ix, int iy, float x, float y, float* gradient, unsigned int gradient_w, unsigned int gradient_h);
float perlin(float x, float y, float* gradient, unsigned int gradient_w, unsigned int gradient_h);

// a terme remplacer ça par un appel à glm
void calculate_normal(float * coord1, float * coord2, float * coord3, float * norm);

unsigned int diff_time_ms(struct timeval * after, struct timeval * before);
unsigned int diff_time_ms_from_now(struct timeval * begin);
std::string current_date_time();

std::vector<std::string> list_files(std::string ch_dir, std::string ext="");
std::string basename(std::string s);
std::string dirname(std::string s);
std::pair<std::string, std::string> splitext(std::string s);

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

std::vector<std::string> split(const std::string& s, const std::string& delimiter);

std::string glm_to_string(pt_type v);

std::string get_cmd_output(std::string cmd);

#endif
