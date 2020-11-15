#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>

#include <dirent.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "CoreFoundation/CoreFoundation.h"

#include "utile.h"


using namespace std;


// renvoie un double aléatoire entre x0 et x1
double rand_double(double x0, double x1) {
	if (x1> x0)
		return x0+ (x1- x0)* (double)(rand()% 10000)/ 10000;
	else
		return x1+ (x0- x1)* (double)(rand()% 10000)/ 10000;
}


// renvoie un float aléatoire entre x0 et x1
float rand_float(float x0, float x1) {
	if (x1> x0)
		return x0+ (x1- x0)* (float)(rand()% 10000)/ 10000;
	else
		return x1+ (x0- x1)* (float)(rand()% 10000)/ 10000;
}


// renvoie un int aléatoire entre x0 et x1 compris
int rand_int(int x0, int x1) {
	if (x1> x0)
		return x0+ rand()% (x1- x0+ 1);
	else
		return x1+ rand()% (x0- x1+ 1);
}


bool rand_bool() {
	if (rand()% 2)
		return true;
	return false;
}


// interpolation linéaire ; w doit etre entre 0 et 1
float perlin_lerp(float a0, float a1, float w) {
	return (1.0f- w)* a0+ w* a1;
}


// Computes the dot product of the distance and gradient vectors.
float perlin_dot_gradient(int ix, int iy, float x, float y, float* gradient, unsigned int gradient_w, unsigned int gradient_h) {

	// Compute the distance vector
	float dx= x- (float)ix;
	float dy= y- (float)iy;

	// Compute the dot-product
	return (dx* gradient[2* (ix+ iy* gradient_w)]+ dy* gradient[2* (ix+ iy* gradient_w)+ 1]);
}


// Compute Perlin noise at coordinates x, y
float perlin(float x, float y, float* gradient, unsigned int gradient_w, unsigned int gradient_h) {
	
	// Determine grid cell coordinates
	int x0= int(x);
	int x1= x0 + 1;
	int y0= int(y);
	int y1= y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx= x- (float)x0;
	float sy= y- (float)y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;
	n0= perlin_dot_gradient(x0, y0, x, y, gradient, gradient_w, gradient_h);
	n1= perlin_dot_gradient(x1, y0, x, y, gradient, gradient_w, gradient_h);
	ix0= perlin_lerp(n0, n1, sx);
	n0= perlin_dot_gradient(x0, y1, x, y, gradient, gradient_w, gradient_h);
	n1= perlin_dot_gradient(x1, y1, x, y, gradient, gradient_w, gradient_h);
	ix1= perlin_lerp(n0, n1, sx);
	value= perlin_lerp(ix0, ix1, sy);
	
	return value;
}


void calculate_normal(float *coord1, float *coord2, float *coord3, float *norm) {
	/* calculate Vector1 and Vector2 */
	float va[3], vb[3], vr[3], val;
	va[0]= coord1[0]- coord2[0];
	va[1]= coord1[1]- coord2[1];
	va[2]= coord1[2]- coord2[2];
	
	vb[0]= coord1[0]- coord3[0];
	vb[1]= coord1[1]- coord3[1];
	vb[2]= coord1[2]- coord3[2];
	
	/* cross product */
	vr[0]= va[1]* vb[2]- vb[1]* va[2];
	vr[1]= vb[0]* va[2]- va[0]* vb[2];
	vr[2]= va[0]* vb[1]- vb[0]* va[1];
	
	/* normalization factor */
	val= sqrt(vr[0]* vr[0]+ vr[1]* vr[1]+ vr[2]* vr[2]);
	
	norm[0]= vr[0]/ val;
	norm[1]= vr[1]/ val;
	norm[2]= vr[2]/ val;
}


void absolute_path(const char * rel_path, char * abs_path) {
/*#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
		cout << "Erreur lors de la recherche du chemin courant" << endl;
	CFRelease(resourcesURL);
	
	string s1(path);
	string s2(rel_path);
	string concat= s1+ "/"+ s2;
	strcpy(abs_path, concat.c_str());
#else*/
	strcpy(abs_path, rel_path);
//#endif
}


unsigned int diff_time_ms(struct timeval * after, struct timeval * before) {
	struct timeval diff;
	timersub(after, before, &diff);
	unsigned long i= diff.tv_sec* 1e6+ diff.tv_usec;
	return i/ 1000;
}


unsigned int diff_time_ms_from_now(struct timeval * begin) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return diff_time_ms(&now, begin);
}


glm::vec3 sum_over_e(glm::vec3* e, glm::vec3* e_prime, int& i) {
	int k = 0;
	glm::vec3 result;
	while (k < i) {
		float e_prime_k_squared = glm::dot(e_prime[k], e_prime[k]);
		result += (glm::dot(e[i], e_prime[k]) / e_prime_k_squared) * e_prime[k];
		k++;
	}
	return result;
}


void gram_schmidt(float * mat) {
	glm::vec3 e[] = {
		glm::vec3(mat[0], mat[1], mat[2]),
		glm::vec3(mat[4], mat[5], mat[6]),
		glm::vec3(mat[8], mat[9], mat[10])
	};
	glm::vec3 e_prime[3];
	e_prime[0]= e[0];
	int i= 0;

	do {
		e_prime[i]= e[i]- sum_over_e(e, e_prime, i);
		i++;
	} while (i< 3);
	
	for (i=0; i<3; i++)
		e_prime[i]= glm::normalize(e_prime[i]);
	
	float * pt;
	pt= glm::value_ptr(e_prime[0]);
	memcpy(mat, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[1]);
	memcpy(mat+ 4, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[2]);
	memcpy(mat+ 8, pt, sizeof(float) * 3);
}


vector<string> list_files(string ch_dir, string ext) {
	vector<string> res;
	DIR *d;
	struct dirent *dir;
	d = opendir(ch_dir.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			string f= string(dir->d_name);
			if ((ext== "") || (f.substr(f.find_last_of(".")+ 1)== ext))
				//res.push_back(f.substr(0, f.find_last_of(".")));
				res.push_back(f);
		}
		closedir(d);
	}
	
	return res;
}


// trim from start (in place)
void ltrim(string & s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
}

// trim from end (in place)
void rtrim(string & s) {
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
}

// trim from both ends (in place)
void trim(string & s) {
	ltrim(s);
	rtrim(s);
}

string basename(string s) {
	string with_ext= s.substr(s.find_last_of("/")+ 1);
	return with_ext.substr(0, with_ext.find("."));
}
