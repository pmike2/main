#include "utile.h"
#include "spark.h"


// Spark ---------------------------------------
Spark::Spark() : _is_alive(false), _opacity(0.0) {
	_bbox= new BBox_2D();
}


Spark::~Spark() {
	delete _bbox;
}


void Spark::reinit(pt_type center, number alpha, pt_type half_size, pt_type center_inc, number alpha_inc, pt_type half_size_inc) {
	_bbox->_center= center;
	_bbox->_alpha= alpha;
	_bbox->_half_size= half_size;
	_bbox->update();

	_center_inc= center_inc;
	_alpha_inc= alpha_inc;
	_half_size_inc= half_size_inc;

	_opacity= SPARKLE_OPACITY_INIT;
	_is_alive= true;
}


void Spark::anim() {
	if (!_is_alive) {
		return;
	}

	_opacity-= SPARKLE_OPACITY_DECREMENT;
	if (_opacity< SPARKLE_OPACITY_THRESHOLD) {
		_opacity= 0.0;
		_is_alive= false;
	}

	_bbox->_center+= _center_inc;
	_bbox->_half_size+= _half_size_inc;
	_bbox->_alpha+= _alpha_inc;
	_bbox->update();
}


// SparkSystem ------------------------------------
SparkSystem::SparkSystem() {
	_sparks= new Spark*[N_MAX_SPARKS];
	for (unsigned int i=0; i<N_MAX_SPARKS; ++i) {
		_sparks[i]= new Spark();
	}
}


SparkSystem::~SparkSystem() {
	for (unsigned int i=0; i<N_MAX_SPARKS; ++i) {
		delete _sparks[i];
	}
	delete[] _sparks;
}



void SparkSystem::reinit() {
	for (unsigned int i=0; i<N_MAX_SPARKS; ++i) {
		_sparks[i]->_is_alive= false;
	}
}


void SparkSystem::anim(time_point t, std::vector<pt_type> positions) {
	for (unsigned int i=0; i<N_MAX_SPARKS; ++i) {
		_sparks[i]->anim();
	}

	for (auto position : positions) {
		unsigned int n_sparks= rand_int(N_MIN_SPARKS_PER_COLLISION, N_MAX_SPARKS_PER_COLLISION);
		for (unsigned int i=0; i<n_sparks; ++i) {
			Spark * spark= NULL;
			for (unsigned int i=0; i<N_MAX_SPARKS; ++i) {
				if (!_sparks[i]->_is_alive) {
					spark= _sparks[i];
					break;
				}
			}
			if (spark!= NULL) {
				number alpha= rand_number(0.0, 2.0* M_PI);
				pt_type half_size= pt_type(rand_number(MIN_SPARK_HALF_SIZE, MAX_SPARK_HALF_SIZE), SPARK_HALF_WIDTH);
				pt_type center_inc= pt_type(rand_number(MIN_SPARK_CENTER_INC, MAX_SPARK_CENTER_INC), rand_number(MIN_SPARK_CENTER_INC, MAX_SPARK_CENTER_INC));
				number alpha_inc= rand_number(MIN_SPARK_ALPHA_INC, MAX_SPARK_ALPHA_INC);
				pt_type half_size_inc= pt_type(rand_number(MIN_SPARK_HALF_SIZE_INC, MAX_SPARK_HALF_SIZE_INC), 0.0);
				spark->reinit(position, alpha, half_size, center_inc, alpha_inc, half_size_inc);
			}
			else {
				std::cout << "SparkSystem saturé\n";
			}
		}
	}
}
