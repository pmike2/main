
#ifndef UTILE_H
#define UTILE_H

#include "CoreFoundation/CoreFoundation.h"

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <dirent.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constantes.h"


//void write2log(char * str);

// renvoie un double aléatoire entre x0 et x1
double rand_double(double x0, double x1);

// renvoie un int aléatoire entre x0 et x1 compris
int rand_int(int x0, int x1);

char * load_source(const char *filename);
GLuint load_shader(GLenum type, const char *filename);
void check_gl_program(GLuint prog);

void calculate_normal(float * coord1, float * coord2, float * coord3, float * norm);

void absolute_path(const char * rel_path, char * abs_path);

unsigned int diff_time_ms(struct timeval * after, struct timeval * before);
unsigned int diff_time_ms_from_now(struct timeval * begin);

std::vector<std::string> list_files(std::string ch_dir, std::string ext="");

glm::vec3 sum_over_e(glm::vec3* e, glm::vec3* e_prime, int& i);
void gram_schmidt(float * mat);

#endif
