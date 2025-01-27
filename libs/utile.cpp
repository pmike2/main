#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sys/stat.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "CoreFoundation/CoreFoundation.h"

#include "utile.h"


using namespace std;


// test égalité stricte
bool number_equals_strict(number x, number y) {
	return x== y;
}


// test égalité avec marge
bool number_equals_epsilon(number x, number y, number threshold) {
	return abs(x- y)< threshold;
}


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


number rand_number(number x0, number x1) {
	if (x1> x0)
		return x0+ (x1- x0)* (number)(rand()% 10000)/ 10000;
	else
		return x1+ (x0- x1)* (number)(rand()% 10000)/ 10000;
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


string current_date_time() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[256];
	tstruct= *localtime(&now);
	strftime(buf, sizeof(buf), "%Y_%m_%d_%H_%M_%S", &tstruct);

	return buf;
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

				if (f.back()!= '.') {
					//res.push_back(f);
					res.push_back(ch_dir+ "/"+ f);
				}
		}
		closedir(d);
	}
	
	std::sort(res.begin(), res.end());
	
	return res;
}


bool file_exists(const string filepath) {
	struct stat buffer;
	return (stat(filepath.c_str(), &buffer)== 0);
}


string basename(string s) {
	string with_ext= s.substr(s.find_last_of("/")+ 1);
	return with_ext.substr(0, with_ext.find("."));
}


string dirname(string s) {
	return s.substr(0, s.find_last_of("/"));
}


std::pair<std::string, std::string> splitext(std::string s) {
	string without_ext= s.substr(0, s.find_last_of("."));
	string ext= s.substr(s.find_last_of(".") + 1);
	return std::make_pair(without_ext, ext);
}


// trim from start (in place)
void ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !isspace(ch);
	}));
}


// trim from end (in place)
void rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !isspace(ch);
	}).base(), s.end());
}


// trim from both ends (in place)
void trim(string &s) {
	rtrim(s);
	ltrim(s);
}


vector<string> split(const string & s, const string & delimiter) {
	size_t pos_start= 0, pos_end, delim_len= delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end= s.find(delimiter, pos_start))!= string::npos) {
		token= s.substr(pos_start, pos_end- pos_start);
		pos_start= pos_end+ delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}


// remplace glm::to_string en supprimant les 0 dans les décimales
string glm_to_string(pt_type v) {
	string str_x= to_string (v.x);
	str_x.erase(str_x.find_last_not_of('0')+ 1, string::npos);
	str_x.erase(str_x.find_last_not_of('.')+ 1, string::npos);

	string str_y= to_string (v.y);
	str_y.erase(str_y.find_last_not_of('0')+ 1, string::npos);
	str_y.erase(str_y.find_last_not_of('.')+ 1, string::npos);

	return "("+ str_x+ ", "+ str_y+ ")";
}


std::string get_cmd_output(std::string cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}
