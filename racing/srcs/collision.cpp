
#include "typedefs.h"
#include "bbox_2d.h"
#include "geom_2d.h"
#include "utile.h"
#include "car.h"
#include "collision.h"


bool collision(StaticObject * obj1, StaticObject * obj2, pt_type & position) {
	if (obj1->_model->_movement== MVMT_FIXED && obj2->_model->_movement== MVMT_FIXED) {
		return false;
	}

	if (!obj1->_model->_material->_solid || !obj2->_model->_material->_solid) {
		return false;
	}

	// 1er test - cher sur les AABB
	if (!aabb_intersects_aabb(obj1->_bbox->_aabb, obj2->_bbox->_aabb)) {
		return false;
	}

	// axis est le vecteur le long duquel les 2 objets s'intersectent le plus
	// overlap est la taille de l'intersection le long de cet axe
	// idx_pt est l'indice du pt de l'objet qui pénètre l'autre
	// is_pt_in_poly1 indique si c'est poly1 qui est pénétrant ou pénétré
	// is_inter vaudra true s'il y a intersection
	pt_type axis(0.0, 0.0);
	number overlap= 0.0;
	unsigned int idx_pt= 0;
	bool is_pt_in_poly1= false;
	bool is_inter= poly_intersects_poly(obj1->_footprint, obj2->_footprint, &axis, &overlap, &idx_pt, &is_pt_in_poly1);

	// pas d'intersection : on sort
	if (!is_inter) {
		return false;
	}

	// pour une déco on ne va pas plus loin mais on renvoie true pour que la deco->_car_contact soit à true
	if (obj1->_model->_type== DECORATION || obj2->_model->_type== DECORATION) {
		return true;
	}

	// on veut éviter le cas où c'est un angle du décor qui pénètre la voiture car sinon réactions bizarres
	// on attend du coup la situation inverse où un angle de la voiture pénètre le décor, ce qui fera plus naturel
	if (is_pt_in_poly1 && obj1->_model->_type== OBSTACLE_TILE) {
		return false;
	}

	// on se place comme dans le cas https://en.wikipedia.org/wiki/Collision_response
	// où la normale est celle de body1 et le point dans body2
	if (is_pt_in_poly1) {
		StaticObject * obj_tmp= obj1;
		obj1= obj2;
		obj2= obj_tmp;
	}

	// position de la collision, récupérée par Track ensuite
	position= obj2->_footprint->_pts[idx_pt];

	// on écarte un peu plus que de 0.5 de chaque coté ou de 1.0 dans le cas fixed
	// est-ce utile ?
	if (obj1->_model->_movement== MVMT_FIXED || obj1->_model->_movement== MVMT_ROTATE || obj1->_model->_movement== MVMT_TRANSLATE_CONSTRAINED) {
		obj2->_com+= overlap* 1.05* axis;
	}
	else if (obj2->_model->_movement== MVMT_FIXED || obj2->_model->_movement== MVMT_ROTATE || obj2->_model->_movement== MVMT_TRANSLATE_CONSTRAINED) {
		obj1->_com-= overlap* 1.05* axis;
	}
	else {
		obj1->_com-= overlap* 0.55* axis;
		obj2->_com+= overlap* 0.55* axis;
	}

	// voir doc/collision.png récupéré de https://en.wikipedia.org/wiki/Collision_response
	pt_type r1, r2;
	r1= obj2->_footprint->_pts[idx_pt]- obj1->_com;
	r2= obj2->_footprint->_pts[idx_pt]- obj2->_com;
	
	pt_type r1_norm= normalized(r1);
	pt_type r1_norm_perp(-1.0* r1_norm.y, r1_norm.x);
	pt_type contact_pt_velocity1= obj1->_velocity+ obj1->_angular_velocity* r1_norm_perp;

	pt_type r2_norm= normalized(r2);
	pt_type r2_norm_perp(-1.0* r2_norm.y, r2_norm.x);
	pt_type contact_pt_velocity2= obj2->_velocity+ obj2->_angular_velocity* r2_norm_perp;

	pt_type vr= contact_pt_velocity2- contact_pt_velocity1;

	// sera la norme de la nouvelle vitesse des objets
	number impulse;

	// https://en.wikipedia.org/wiki/Coefficient_of_restitution
	// restitution doit etre entre 0 et 1 ; proche de 0 -> pas de rebond ; proche de 1 -> beaucoup de rebond
	// en pratique j'ai mis des restitution > 1 pour plus de fun
	// on prend la moyenne
	number restitution= 0.5* (obj1->_model->_material->_restitution+ obj2->_model->_material->_restitution);
	
	// dans le cas où 1 des 2 objets est fixe on considère que sa masse et son inertie sont infinies
	if (obj1->_model->_movement== MVMT_FIXED) {
		pt_type v= (cross2d(r2, axis)/ obj2->_inertia)* r2;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj2->_mass+ dot(v, axis));
	}
	else if (obj2->_model->_movement== MVMT_FIXED) {
		pt_type v= (cross2d(r1, axis)/ obj1->_inertia)* r1;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_mass+ dot(v, axis));
	}
	else {
		pt_type v= (cross2d(r1, axis)/ obj1->_inertia)* r1+ (cross2d(r2, axis)/ obj2->_inertia)* r2;
		impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ obj1->_mass+ 1.0/ obj2->_mass+ dot(v, axis));
	}

	// on sature impulse afin que ca ne parte pas trop en cahouèt
	if (abs(impulse)> MAX_IMPULSE) {
		std::cout << "impulse=" << impulse << "\n";
		impulse= MAX_IMPULSE;
	}

	// on modifie directement la vitesse et la vitesse angulaire
	if (obj1->_model->_movement== MVMT_ALL || obj1->_model->_movement== MVMT_TRANSLATE) {
		obj1->_velocity-= (impulse/ obj1->_mass)* axis;
	}
	else if (obj1->_model->_movement== MVMT_TRANSLATE_CONSTRAINED) {
		obj1->_velocity-= (impulse/ obj1->_mass)* proj(axis, obj1->_translation_constraint);
	}

	if (obj1->_model->_movement== MVMT_ALL || obj1->_model->_movement== MVMT_ROTATE) {
		// facteur multiplicatif pour _angular_velocity pour que ce soit plus dynamique...
		obj1->_angular_velocity-= COLLISION_ANGULAR_FACTOR* (impulse/ obj1->_inertia)* cross2d(r1, axis);
	}

	if (obj2->_model->_movement== MVMT_ALL || obj2->_model->_movement== MVMT_TRANSLATE) {
		obj2->_velocity+= (impulse/ obj2->_mass)* axis;
	}
	else if (obj2->_model->_movement== MVMT_TRANSLATE_CONSTRAINED) {
		obj2->_velocity+= (impulse/ obj2->_mass)* proj(axis, obj2->_translation_constraint);
	}

	if (obj2->_model->_movement== MVMT_ALL || obj2->_model->_movement== MVMT_ROTATE) {
		obj2->_angular_velocity+= COLLISION_ANGULAR_FACTOR* (impulse/ obj2->_inertia)* cross2d(r2, axis);
	}

	// peut-être pas nécessaire
	obj1->_acceleration= pt_type(0.0);
	obj1->_angular_acceleration= 0.0;
	obj2->_acceleration= pt_type(0.0);
	obj2->_angular_acceleration= 0.0;

	// thrust max brimé par matériau en collision
	for (auto obj_pair : std::vector<std::pair<StaticObject *, StaticObject *> >{{obj1, obj2}, {obj2, obj1}}) {
		if (obj_pair.first->_model->_type== CAR) {
			Car * car= (Car *)(obj_pair.first);
			if (car->_thrust> obj_pair.second->_model->_material->_collision_thrust) {
				car->_thrust= obj_pair.second->_model->_material->_collision_thrust;
			}
		}
	}

	// bumps
	if (obj1->_model->_movement!= MVMT_FIXED && obj1->_model->_material->_solid && obj1->_model->_material->_bumpable) {
		int obj_bump_idx_1= -1;
		int obj_bump_idx_2= -1;
		std::pair<BBOX_SIDE, BBOX_CORNER>p= bbox_side_corner(obj1->_bbox, obj2->_footprint->_pts[idx_pt]);
		if (p.first== BOTTOM_SIDE) {
			obj_bump_idx_1= 0;
			obj_bump_idx_2= 1;
		}
		else if (p.first== RIGHT_SIDE) {
			obj_bump_idx_1= 2;
			obj_bump_idx_2= 3;
		}
		else if (p.first== TOP_SIDE) {
			obj_bump_idx_1= 4;
			obj_bump_idx_2= 5;
		}
		else if (p.first== LEFT_SIDE) {
			obj_bump_idx_1= 6;
			obj_bump_idx_2= 7;
		}
		for (int i=0; i<N_BUMPS; ++i) {
			if (i== obj_bump_idx_1 || i== obj_bump_idx_2) {
				obj1->_bumps[i]+= obj2->_model->_material->_damage* impulse;
			}
			else {
				obj1->_bumps[i]+= obj2->_model->_material->_damage* impulse* rand_number(0.2, 0.5);
			}
			if (obj1->_bumps[i]> BUMP_MAX) {
				obj1->_bumps[i]= BUMP_MAX;
			}
		}
	}

	if (obj2->_model->_movement!= MVMT_FIXED && obj2->_model->_material->_solid && obj2->_model->_material->_bumpable) {
		int obj_bump_idx_1= -1;
		int obj_bump_idx_2= -1;
		std::pair<BBOX_SIDE, BBOX_CORNER>p= bbox_side_corner(obj2->_bbox, obj2->_footprint->_pts[idx_pt]);
		if (p.second== BOTTOMLEFT_CORNER) {
			obj_bump_idx_1= 0;
			obj_bump_idx_2= 7;
		}
		else if (p.second== BOTTOMRIGHT_CORNER) {
			obj_bump_idx_1= 1;
			obj_bump_idx_2= 2;
		}
		else if (p.second== TOPRIGHT_CORNER) {
			obj_bump_idx_1= 3;
			obj_bump_idx_2= 4;
		}
		else if (p.second== TOPLEFT_CORNER) {
			obj_bump_idx_1= 5;
			obj_bump_idx_2= 6;
		}
		for (int i=0; i<N_BUMPS; ++i) {
			if (i== obj_bump_idx_1 || i== obj_bump_idx_2) {
				obj2->_bumps[i]+= obj1->_model->_material->_damage* impulse;
			}
			else {
				obj2->_bumps[i]+= obj1->_model->_material->_damage* impulse* rand_number(0.2, 0.5);
			}
			if (obj2->_bumps[i]> BUMP_MAX) {
				obj2->_bumps[i]= BUMP_MAX;
			}
		}
	}

	return true;
}
