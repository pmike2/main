#ifndef SPARK_H
#define SPARK_H

#include <chrono>

#include "geom_2d.h"
#include "typedefs.h"


// nombre max de sparks pour tout le circuit
const unsigned int N_MAX_SPARKS= 4000;
// min, max du nombre de sparks créés lors d'une collision
const unsigned int N_MIN_SPARKS_PER_COLLISION= 1;
const unsigned int N_MAX_SPARKS_PER_COLLISION= 4;
// opacité initiale - décrément d'opacité à chaque anim - seuil d'opacité -> suppression
const number SPARKLE_OPACITY_INIT= 1.0;
const number SPARKLE_OPACITY_DECREMENT= 0.1;
const number SPARKLE_OPACITY_THRESHOLD= 0.05;
// min, max taille BBox
const number MIN_SPARK_HALF_SIZE= 0.005;
const number MAX_SPARK_HALF_SIZE= 0.01;
// largeur fixe d'un spark
const number SPARK_HALF_WIDTH= 0.01;
// min, max incrément position
const number MIN_SPARK_CENTER_INC= -0.05;
const number MAX_SPARK_CENTER_INC= 0.05;
// min, max incrément angle
const number MIN_SPARK_ALPHA_INC= 0.0;
const number MAX_SPARK_ALPHA_INC= 0.02;
// min, max incrément taille
const number MIN_SPARK_HALF_SIZE_INC= 0.002;
const number MAX_SPARK_HALF_SIZE_INC= 0.006;


// Etincelle
class Spark {
public:
	Spark();
	~Spark();
	void reinit(pt_type center, number alpha, pt_type half_size, pt_type center_inc, number alpha_inc, pt_type half_size_inc);
	void anim();


	BBox_2D * _bbox; // BBox englobante
	number _opacity; // opacité
	bool _is_alive; // active ?
	pt_type _center_inc; // incrémentation position
	pt_type _half_size_inc; // incrémentation taille
	number _alpha_inc; // incrémentation angle rotation
};


// Ensemble d'étincelles
class SparkSystem {
public:
	SparkSystem();
	~SparkSystem();
	void reinit();
	void anim(time_point t, std::vector<pt_type> positions);


	Spark ** _sparks;
};

#endif
