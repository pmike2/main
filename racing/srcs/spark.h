#ifndef SPARK_H
#define SPARK_H

#include <chrono>

#include "geom_2d.h"
#include "typedefs.h"


const unsigned int N_MAX_SPARKS= 2000;
const unsigned int N_MIN_SPARKS_PER_COLLISION= 1;
const unsigned int N_MAX_SPARKS_PER_COLLISION= 2;
const number SPARKLE_OPACITY_INIT= 1.0;
const number SPARKLE_OPACITY_DECREMENT= 0.06;
const number SPARKLE_OPACITY_THRESHOLD= 0.05;
const number MIN_SPARK_HALF_SIZE= 0.005;
const number MAX_SPARK_HALF_SIZE= 0.01;
const number MIN_SPARK_CENTER_INC= -0.05;
const number MAX_SPARK_CENTER_INC= 0.05;
const number MIN_SPARK_ALPHA_INC= 0.0;
const number MAX_SPARK_ALPHA_INC= 0.1;
const number MIN_SPARK_HALF_SIZE_INC= 0.001;
const number MAX_SPARK_HALF_SIZE_INC= 0.005;


class Spark {
public:
	Spark();
	~Spark();
	void reinit(pt_type center, number alpha, pt_type half_size, pt_type center_inc, number alpha_inc, pt_type half_size_inc);
	void anim();


	BBox_2D * _bbox; // BBox englobante
	number _opacity; // opacité
	bool _is_alive; // active ?
	pt_type _center_inc;
	pt_type _half_size_inc;
	number _alpha_inc;
};


class SparkSystem {
public:
	SparkSystem();
	~SparkSystem();
	void reinit();
	void anim(std::chrono::system_clock::time_point t, std::vector<pt_type> positions);


	Spark ** _sparks;
};

#endif
