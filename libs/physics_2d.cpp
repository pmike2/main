
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "physics_2d.h"
#include "utile.h"



using namespace std;


bool is_pt_inside_body(pt_type pt, RigidBody2D * body) {
    if (glm::distance(pt, body->_position)> body->_polygon->_radius) {
        return false;
    }
    for (unsigned int i=0; i<body->_polygon->_pts.size(); ++i) {
        pt_type pt1= body->_orientation_mat* body->_polygon->_pts[i]+ body->_position;
        pt_type pt2= body->_orientation_mat* body->_polygon->_pts[(i+ 1)% body->_polygon->_pts.size()]+ body->_position;
        if (!is_left(pt1, pt2- pt1, pt)) {
            return false;
        }
    }
    return true;
}


// si existe intersection la + proche du pt de départ du segment avec le poly
bool segment_intersects_body(pt_type pt_begin, pt_type pt_end, RigidBody2D * body, pt_type * result) {
    number min_dist= 1e10;
    bool is_inter= false;
    pt_type inter(0.0f);
    for (unsigned int i=0; i<body->_polygon->_pts.size(); ++i) {
        pt_type pt1= body->_orientation_mat* body->_polygon->_pts[i]+ body->_position;
        pt_type pt2= body->_orientation_mat* body->_polygon->_pts[(i+ 1)% body->_polygon->_pts.size()]+ body->_position;
        
        if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, &inter)) {
            number dist2= glm::distance2(pt_begin, inter);
            if (dist2< min_dist) {
                is_inter= true;
                min_dist= dist2;
                result->x= inter.x;
                result->y= inter.y;
            }
        }
    }
    return is_inter;
}


number distance_body_pt(RigidBody2D * body, pt_type pt, pt_type * proj) {
    number dist_min= 1e10;
    number dist;
    pt_type proj2;

    if (is_pt_inside_body(pt, body)) {
        proj->x= pt.x;
        proj->y= pt.y;
        return 0.0f;
    }

    for (unsigned int i=0; i<body->_polygon->_pts.size(); ++i) {
        pt_type pt1= body->_orientation_mat* body->_polygon->_pts[i]+ body->_position;
        pt_type pt2= body->_orientation_mat* body->_polygon->_pts[(i+ 1)% body->_polygon->_pts.size()]+ body->_position;

        bool x= distance_segment_pt(pt1, pt2, pt, &dist, &proj2);
        if (dist< dist_min) {
            dist_min= dist;
            proj->x= proj2.x;
            proj->y= proj2.y;
        }
    }
    return dist_min;
}


// cf Separating Axis Theorem (SAT)
void axis_least_penetration(RigidBody2D * body_a, RigidBody2D * body_b, unsigned short * idx_pt_max, number * penetration_max) {
    *penetration_max= -1e10;
    *idx_pt_max= 0;
    for (unsigned short idx_pt=0; idx_pt<body_a->_polygon->_pts.size(); ++idx_pt) {
        // normale dans le repère body_a
        pt_type normal_a= body_a->_polygon->_normals[idx_pt];
        // normale dans le repère body_b
        pt_type normal_b= glm::transpose(body_b->_orientation_mat)* body_a->_orientation_mat* normal_a;
        // pt de body_b le + éloigné dans la direction opposée à la normale dans le repère body_b
        pt_type farthest_pt= body_b->_polygon->farthest_pt_along_dir(-normal_b);
        // sommet dans le repère body_a
        pt_type pt_a= body_a->_polygon->_pts[idx_pt];
        // sommet dans le repère body_b
        pt_type pt_b= glm::transpose(body_b->_orientation_mat)* (body_a->_orientation_mat* pt_a+ body_a->_position- body_b->_position);
        // plus grande pénétration dans le repère body_b
        number penetration= glm::dot(normal_b, farthest_pt- pt_b);
        if (*penetration_max< penetration) {
            *penetration_max= penetration;
            *idx_pt_max= idx_pt;
        }
    }
}


// permet d'avoir + de stabilité lors du choix de la normale le long de laquelle appliquer un impulse
// dans des configs où les 2 faces des objets s'interpénétrant sont parallèles
bool biased_cmp(number a, number b) {
    return a>= b* BIAS_CMP_RELATIVE+ a* BIAS_CMP_ABSOLUTE;
}


// --------------------------------------------------------------------------------------------------------------
Material::Material() {

}


Material::Material(number density, number static_friction, number dynamic_friction, number restitution) :
	_density(density), _static_friction(static_friction), _dynamic_friction(dynamic_friction), _restitution(restitution)
{

}


Material::~Material() {

}


void Material::print() {
    cout << "density= " << _density << " ; static_friction=" << _static_friction << " ; dynamic_friction=" << _dynamic_friction << " ; restitution=" << _restitution << "\n";
}

// --------------------------------------------------------------------------------------------------------------
RigidBody2DState::RigidBody2DState() {

}


RigidBody2DState::RigidBody2DState(pt_type position, number orientation) : _position(position), _orientation(orientation) {

}


RigidBody2DState::~RigidBody2DState() {

}


// --------------------------------------------------------------------------------------------------------------
unsigned short RigidBody2D::CurrentID= 0;


RigidBody2D::RigidBody2D() {
    _polygon= new Polygon2D();
    _polygon->randomize(10);

}


RigidBody2D::RigidBody2D(Polygon2D * polygon, Material * material, pt_type position, number orientation) :
    _id(CurrentID++),
    _polygon(polygon), _material(material), _position(position), _velocity(pt_type(0.0f)), _angular_velocity(0.0f), _force(pt_type(0.0f)), _torque(0.0f),
	_mass(0.0f), _mass_inv(0.0f), _inertia_moment(0.0f), _inertia_moment_inv(0.0f), _is_static(false)
{
    set_orientation(orientation);

	// calcul masse
    _mass= _polygon->_area* _material->_density;

    // calcul moment d'inertie
    _inertia_moment= 0.0f;
    for (unsigned int i=0; i<_polygon->_pts.size(); ++i) {
        pt_type pt1= _polygon->_pts[i];
        pt_type pt2= _polygon->_pts[(i+ 1)% _polygon->_pts.size()];
        _inertia_moment+= 0.25f* THIRD* _material->_density* cross2d(pt1, pt2)* (glm::dot(pt1, pt1)+ glm::dot(pt1, pt2)+ glm::dot(pt2, pt2));
    }

	// solides inamovibles
    if (_material->_density< EPSILON) {
        _mass= 0.0f;
        _mass_inv= 0.0f;
        _inertia_moment= 0.0f;
        _inertia_moment_inv= 0.0f;
        _is_static= true;
    }
    else {
        _mass_inv= 1.0f/ _mass;
        _inertia_moment_inv= 1.0f/ _inertia_moment;
    }

    _previous_state= new RigidBody2DState(_position, _orientation);
}


RigidBody2D::~RigidBody2D() {
    delete _previous_state;
}


void RigidBody2D::set_orientation(number orientation) {
    _orientation= orientation;
    rotation_float2mat(_orientation, _orientation_mat);
}


void RigidBody2D::integrate_forces(number dt) {
    // inamovible
    if (_is_static) {
        return;
    }

    _velocity+= (_force* _mass_inv+ GRAVITY)* dt* 0.5;
    _angular_velocity+= _torque* _inertia_moment_inv* dt* 0.5;
}


void RigidBody2D::integrate_velocity(number dt) {
    // inamovible
    if (_is_static) {
        return;
    }

    _position+= _velocity* dt;
    set_orientation(_orientation+ _angular_velocity* dt);
    // dans le code du tuto il refait ça ici, pourquoi ? 
    integrate_forces(dt);
}


void RigidBody2D::apply_impulse(pt_type impulse, pt_type contact) {
    // inamovible
    if (_is_static) {
        return;
    }

    _velocity+= _mass_inv* impulse;
    _angular_velocity+= _inertia_moment_inv* cross2d(contact, impulse);
}


void RigidBody2D::clear_forces() {
    _force= pt_type(0.0f);
    _torque= 0.0f;
}


void RigidBody2D::update_previous_state() {
    _previous_state->_position= _position;
    _previous_state->_orientation= _orientation;
}


void RigidBody2D::print() {
	_polygon->print();
	_material->print();
    cout << "mass= " << _mass << " ; mass_inv= " << _mass_inv << "\n";
    cout << "inertia_moment= " << _inertia_moment << " ; inertia_moment_inv= " << _inertia_moment_inv << "\n";
    cout << "position=" << glm::to_string(_position) << " ; previous position=" << glm::to_string(_previous_state->_position) << "\n";
    cout << "orientation=" << _orientation << " ; previous orientation=" << _previous_state->_orientation << "\n";
}


void RigidBody2D::save(string ch_file) {
    ofstream ofs(ch_file);
    ofs << "# pts\n";
    for (auto it_pt : _polygon->_pts) {
        ofs << it_pt.x << " " << it_pt.y << "\n";
    }
    ofs << "\n# position\n" << _position.x << " " << _position.y << "\n";
    ofs << "\n# orientation\n" << _orientation << "\n";
}


// --------------------------------------------------------------------------------------------------------------
Contact2D::Contact2D() {

}


Contact2D::Contact2D(pt_type position, RigidBody2D * body_reference, RigidBody2D * body_incident, pt_type normal) :
    _position(position), _body_reference(body_reference), _body_incident(body_incident), _is_valid(true), _is_tangent_valid(true), _normal(normal), _j(0.0f), _jt(0.0f),
	_normal_impulse(pt_type(0.0f)), _tangent_impulse(pt_type(0.0f)), _normal_impulse_cumul(pt_type(0.0f)), _is_resting(true)
{
    _r_ref  = _position- _body_reference->_position;
    _r_incid= _position- _body_incident->_position;
    

    // écart angulaire à _normal
    number cross_norm_ref  = cross2d(_r_ref  , _normal);
    number cross_norm_incid= cross2d(_r_incid, _normal);

    // dénominateur formule calcul j
    _mass_inv_sum= _body_reference->_mass_inv+ _body_incident->_mass_inv+ cross_norm_ref* cross_norm_ref* _body_reference->_inertia_moment_inv+ cross_norm_incid* cross_norm_incid* _body_incident->_inertia_moment_inv;
}


Contact2D::~Contact2D() {
    
}


pt_type Contact2D::contact_relative_velocity() {
    pt_type contact_velocity_ref  = _body_reference->_velocity+ _body_reference->_angular_velocity* pt_type(-_r_ref.y, _r_ref.x);
    pt_type contact_velocity_incid= _body_incident->_velocity + _body_incident->_angular_velocity * pt_type(-_r_incid.y, _r_incid.x);

    return contact_velocity_incid- contact_velocity_ref;
}


void Contact2D::update_normal(number dt) {
    // vitesse relative du contact en B par rapport à A
    pt_type rel_vel= contact_relative_velocity();
    // projetée sur la normale
    number rel_vel_normal= glm::dot(rel_vel, _normal);

    // si objets s'éloignent
    if (rel_vel_normal> 0.0f) {
        _is_valid= false;
        _normal_impulse= pt_type(0.0f);
        return;
    }
    
    // Determine if we should perform a resting collision or not
    // The idea is if the only thing moving this object is gravity,
    // then the collision should be performed without any restitution
    number mixed_restitution= min(_body_reference->_material->_restitution, _body_incident->_material->_restitution);

    //cout << glm::length2(rel_vel) << "\n";
    if (glm::length2(rel_vel)< glm::length2(dt* GRAVITY)+ EPSILON) {
        mixed_restitution= 0.0f;
        //_is_resting= true;
    }
    else {
        _is_resting= false;
    }

    // j est l'amplitude de l'impulse
    _j= -(1.0f+ mixed_restitution)* rel_vel_normal;
    _j/= _mass_inv_sum;
    _normal_impulse= _j* _normal;

    _normal_impulse_cumul+= _normal_impulse;
}


void Contact2D::update_tangent(number dt) {
    // vitesse relative du contact en B par rapport à A
    pt_type rel_vel= contact_relative_velocity();

    // projetée sur la tangente et normalisée ; si _normal et rel_vel colinéaires on sort
    if (abs(cross2d(rel_vel, _normal))< EPSILON) {
        _is_tangent_valid= false;
        _tangent_impulse= pt_type(0.0f);
        return;
    }
    pt_type rel_vel_tan= glm::normalize(rel_vel- glm::dot(rel_vel, _normal)* _normal);

    // jt est l'amplitude de l'impulsion tangentielle
    _jt= -glm::dot(rel_vel, rel_vel_tan);
    _jt/= _mass_inv_sum;

    // on ignore les petites frictions
    if (abs(_jt)< EPSILON) {
        _is_tangent_valid= false;
        _tangent_impulse= pt_type(0.0f);
        return;
    }

    // 2 possibilités ici ; voir laquelle est la + plausible
    //_mixed_static_friction= sqrt(_body_reference->_material->_static_friction* _body_incident->_material->_static_friction);
    //_mixed_dynamic_friction= sqrt(_body_reference->_material->_dynamic_friction* _body_incident->_material->_dynamic_friction);
    number mixed_static_friction= sqrt(_body_reference->_material->_static_friction* _body_reference->_material->_static_friction+ _body_incident->_material->_static_friction* _body_incident->_material->_static_friction);
    number mixed_dynamic_friction= sqrt(_body_reference->_material->_dynamic_friction* _body_reference->_material->_dynamic_friction+ _body_incident->_material->_dynamic_friction* _body_incident->_material->_dynamic_friction);

    // loi de Coulomb
    if (abs(_jt)< _j* mixed_static_friction) {
        _tangent_impulse= rel_vel_tan* _jt;
    }
    else {
        _tangent_impulse= rel_vel_tan* (-_j)* mixed_dynamic_friction;
    }
}


void Contact2D::print() {

}


// --------------------------------------------------------------------------------------------------------------
Collision2D::Collision2D() {

}


Collision2D::Collision2D(RigidBody2D * body_a, RigidBody2D * body_b, bool verbose) :
    _verbose(verbose), _warm_starting(false), _is_valid(false), _is_resting(false)
{
    unsigned short idx_pt_max_a, idx_pt_max_b;
    number penetration_max_a, penetration_max_b;
    
    _contacts.clear();

    if (glm::distance(body_a->_position, body_b->_position)> body_a->_polygon->_radius+ body_b->_polygon->_radius) {
        return;
    }

    // axe de moindre pénétration ; si pénétration >= 0 on a trouvé un axe de séparation entre les 2 polys, pas d'intersection
    axis_least_penetration(body_a, body_b, &idx_pt_max_a, &penetration_max_a);
    if (penetration_max_a>= 0.0f) {
        return;
    }

    axis_least_penetration(body_b, body_a, &idx_pt_max_b, &penetration_max_b);
    if (penetration_max_b>= 0.0f) {
        return;
    }

	if (_verbose) {
    	cout << "\nCollision2D::Collision2D() :\n";
	}

    unsigned short ref_face_idx;
    // identification body de réf, body d'incidence
    if (biased_cmp(penetration_max_a, penetration_max_b)) {
        _body_reference= body_a;
        _body_incident = body_b;
        ref_face_idx= idx_pt_max_a;
    }
    else {
        _body_reference= body_b;
        _body_incident = body_a;
        ref_face_idx= idx_pt_max_b;
    }

    // recup idx face incidence
    unsigned short incid_face_idx= incident_face_idx(ref_face_idx);

    _hash= ((unsigned long long)(_body_reference->_id) << 48) + ((unsigned long long)(ref_face_idx) << 32)+ ((unsigned long long)(_body_incident->_id) << 16) + (unsigned long long)(incid_face_idx);

    // sommets impliqués
    pt_type ref_1= _body_reference->_polygon->_pts[ref_face_idx];
    pt_type ref_2= _body_reference->_polygon->_pts[(ref_face_idx+ 1)% _body_reference->_polygon->_pts.size()];
    pt_type incid_1= _body_incident->_polygon->_pts[incid_face_idx];
    pt_type incid_2= _body_incident->_polygon->_pts[(incid_face_idx+ 1)% _body_incident->_polygon->_pts.size()];

    // dans le système world
    ref_1= _body_reference->_orientation_mat* ref_1+ _body_reference->_position;
    ref_2= _body_reference->_orientation_mat* ref_2+ _body_reference->_position;
    incid_1= _body_incident->_orientation_mat* incid_1+ _body_incident->_position;
    incid_2= _body_incident->_orientation_mat* incid_2+ _body_incident->_position;

    // _normal dans le système world
    _normal= _body_reference->_orientation_mat* _body_reference->_polygon->_normals[ref_face_idx];
    //cout << glm::to_string(_body_reference->_orientation_mat) << " ; " << glm::to_string(_body_reference->_polygon->_normals[ref_face_idx]) << "\n";

    // clip
    pt_type inter_1(0.0f);
    pt_type inter_2(0.0f);
    bool is_intersect_ref_1= ray_intersects_segment(ref_1, _normal, incid_1, incid_2, &inter_1);
    bool is_intersect_ref_2= ray_intersects_segment(ref_2, _normal, incid_1, incid_2, &inter_2);
    //bool is_intersect_ref_1= segment_intersects_line(incid_1, incid_2, ref_1, _normal, inter_1);
    //bool is_intersect_ref_2= segment_intersects_line(incid_1, incid_2, ref_2, _normal, inter_2);
    bool is_incid_1_unclipped= true;
    bool is_incid_2_unclipped= true;
    if ((!is_left(ref_1, _normal, incid_1)) || (is_left(ref_2, _normal, incid_1))) {
        is_incid_1_unclipped= false;
    }
    if ((!is_left(ref_1, _normal, incid_2)) || (is_left(ref_2, _normal, incid_2))) {
        is_incid_2_unclipped= false;
    }

    vector<pt_type> contacts;
    if (is_intersect_ref_1) {
        contacts.push_back(inter_1);
        if (is_intersect_ref_2) {
            contacts.push_back(inter_2);
        }
        else {
            if (is_incid_1_unclipped) {
                contacts.push_back(incid_1);
            }
            else if (is_incid_2_unclipped) {
                contacts.push_back(incid_2);
            }
            else {
                cout << "hoho1\n";
            }
        }
    }
    else if (is_intersect_ref_2) {
        contacts.push_back(inter_2);
        if (is_intersect_ref_1) {
            contacts.push_back(inter_1);
        }
        else {
            if (is_incid_1_unclipped) {
                contacts.push_back(incid_1);
            }
            else if (is_incid_2_unclipped) {
                contacts.push_back(incid_2);
            }
            else {
                cout << "hoho2\n";
            }
        }
    }
    else {
        contacts.push_back(incid_1);
        contacts.push_back(incid_2);
    }

    // on ne garde que les points situés à l'intérieur du polygone de ref
    _penetration= 0.0f;
    number dist;
    pt_type proj;
    for (auto it_contact : contacts) {
        if ((is_left(ref_1, ref_2- ref_1, it_contact)) && (distance_segment_pt(ref_1, ref_2, it_contact, &dist, &proj))) {
            _contacts.push_back(new Contact2D(it_contact, _body_reference, _body_incident, _normal));
            _penetration+= dist;
        }
    }
    if (_contacts.size()) {
        _penetration/= (number)(_contacts.size());
    }
    else {
        // ca arrive a cause de biased_cmp()
		//cout << "hoho3\n";
	}

    /*_penetration= 0.0f;
    number dist;
    pt_type proj;
    for (auto it_contact : _contacts) {
        if (distance_segment_pt(ref_1, ref_2, it_contact, &dist, &proj)) {
            _penetration+= dist;
        }
        else {
            // ne devrait pas arriver car on clip
            cout << "hoho4\n";
            //cout << glm::to_string(ref_1) << " ; " << glm::to_string(ref_2) << " ; " << glm::to_string(it_contact) << " ; " << dist << " ; " << glm::to_string(proj) << "\n";
            body_a->save("body_a.txt");
            body_b->save("body_b.txt");

            ofstream ofs("coll.txt");
            for (auto it_pt : _body_reference->_polygon->_pts) {
                pt_type pt= _body_reference->_orientation_mat* it_pt+ _body_reference->_position;
                ofs << pt.x << " " << pt.y << " r\n";
            }
            for (auto it_pt : _body_incident->_polygon->_pts) {
                pt_type pt= _body_incident->_orientation_mat* it_pt+ _body_incident->_position;
                ofs << pt.x << " " << pt.y << " i\n";
            }
            ofs << ref_1.x << " " << ref_1.y << " ref_1\n";
            ofs << ref_2.x << " " << ref_2.y << " ref_2\n";
            ofs << incid_1.x << " " << incid_1.y << " incid_1\n";
            ofs << incid_2.x << " " << incid_2.y << " incid_2\n";
            ofs << inter_1.x << " " << inter_1.y << " inter_1\n";
            ofs << inter_2.x << " " << inter_2.y << " inter_2\n";
            for (unsigned int i=0; i<contacts.size(); ++i) {
                ofs << contacts[i].x << " " << contacts[i].y << " contacts[" << i << "]\n";
            }
            for (unsigned int i=0; i<_contacts.size(); ++i) {
                ofs << _contacts[i].x << " " << _contacts[i].y << " _contacts[" << i << "]\n";
            }
        }
    }
    _penetration/= _contacts.size();*/

	if (_verbose) {
		cout << "\t_penetration=" << _penetration << " ; _normal=" << glm::to_string(_normal) << "\n";
		for (auto it_contact : _contacts) {
			it_contact->print();
		}
	}
}


Collision2D::~Collision2D() {
    for (auto it_contact : _contacts) {
        delete it_contact;
    }
    _contacts.clear();
}


// recherche de la face du polygon incident
unsigned int Collision2D::incident_face_idx(unsigned int reference_face_idx) {
    pt_type reference_normal= _body_reference->_polygon->_normals[reference_face_idx];
    // passage repere body_incident
    reference_normal= glm::transpose(_body_incident->_orientation_mat)* _body_reference->_orientation_mat* reference_normal;
    // recherche de la normale la plus opposée
    unsigned int result= 0;
    number min_dot= 1e10;
    for (unsigned int idx_pt=0; idx_pt<_body_incident->_polygon->_pts.size(); ++idx_pt) {
        number dot= glm::dot(reference_normal, _body_incident->_polygon->_normals[idx_pt]);
        if (dot< min_dot) {
            min_dot= dot;
            result= idx_pt;
        }
    }
    return result;
}


void Collision2D::apply_impulse(number dt) {
    // les 2 sont inamovibles
    if ((_body_reference->_is_static) && (_body_incident->_is_static)) {
        _body_reference->_velocity= pt_type(0.0f);
        _body_incident->_velocity = pt_type(0.0f);
		_is_valid= false;
    }

	// a partir d'ici l'ordre des opérations est TRES important !
	if (!_warm_starting) {
		for (auto it_contact : _contacts) {
			it_contact->update_normal(dt);
		}
	}
	_warm_starting= false;

	_is_valid= false;
	for (auto it_contact : _contacts) {
		if (it_contact->_is_valid) {
			_is_valid= true;
			break;
		}
	}
    if (!_is_valid) {
        return;
    }
    
    _is_resting= true;
    for (auto it_contact : _contacts) {
        if (!it_contact->_is_resting) {
            _is_resting= false;
            break;
        }
    }

	if (_verbose) {
    	cout << "\nCollision2D::apply_impulse() :\n";
	}

	for (auto it_contact : _contacts) {
        // impulse selon la normale ------------------------------------------------------
        if (it_contact->_is_valid) {
            pt_type normal_impulse= it_contact->_normal_impulse/ (number)(_contacts.size());
            _body_reference->apply_impulse(-normal_impulse, it_contact->_r_ref);
            _body_incident->apply_impulse(normal_impulse  , it_contact->_r_incid);
			if (_verbose) {
				cout << "normal_impulse=" << glm::to_string(normal_impulse) << "\n";
			}
        }
	}

    for (auto it_contact : _contacts) {
        it_contact->update_tangent(dt);
	}

	for (auto it_contact : _contacts) {
        if (it_contact->_is_valid) {
            // impulse tangentielle -> friction --------------------------------------------
            if (it_contact->_is_tangent_valid) {
                pt_type tangent_impulse= it_contact->_tangent_impulse/ (number)(_contacts.size());
                _body_reference->apply_impulse(-tangent_impulse, it_contact->_r_ref);
                _body_incident->apply_impulse(tangent_impulse, it_contact->_r_incid);
                if (_verbose) {
                    cout << "tangent_impulse=" << glm::to_string(tangent_impulse) << "\n";
                }
            }
        }
    }
}


// empeche les corps de s'enfoncer les uns dans les autres ; on écarte légèrement les 2 corps le long de _normal
void Collision2D::positional_correction() {
	if (_verbose) {
    	cout << "\nCollision2D::positional_correction() :\n";
	}
    
    pt_type correction= (max(_penetration- PENETRATION_ALLOWANCE, 0.0)/ (_body_reference->_mass_inv+ _body_incident->_mass_inv))* _normal* PENETRATION_PERCENT2CORRECT;
    _body_reference->_position-= correction* _body_reference->_mass_inv;
    _body_incident->_position += correction* _body_incident->_mass_inv;

	if (_verbose) {
    	cout << "\tcorrection=" << glm::to_string(correction) << "\n";
	}
}


void Collision2D::print() {

}


// --------------------------------------------------------------------------------------------------------------
LastImpulse::LastImpulse() : _is_2delete(false) {

}


LastImpulse::~LastImpulse() {
    _normal_impulses.clear();
    _tangent_impulses.clear();
}


// --------------------------------------------------------------------------------------------------------------
Physics2D::Physics2D() {

}


Physics2D::Physics2D(number dt) : _dt(dt), _paused(false), _warm_starting_enabled(false) {
	_materials.clear();

    // density, static_friction, dynamic_friction, restitution
	Material * mat_static= new Material(0.0f, 0.5f, 0.3f, 0.4f); // densité == 0 -> mass == 0 -> mass_inv == 0 -> inamovible
	Material * mat_rock  = new Material(0.6f, 0.5f, 0.3f, 0.1f);
	Material * mat_wood  = new Material(0.3f, 0.5f, 0.3f, 0.2f);
	Material * mat_metal = new Material(1.2f, 0.5f, 0.3f, 0.05f);
	Material * mat_bouncy= new Material(0.3f, 0.5f, 0.3f, 0.95f);
	Material * mat_pillow= new Material(0.1f, 0.5f, 0.3f, 0.2f);

	_materials.push_back(mat_static);
	_materials.push_back(mat_rock);
	_materials.push_back(mat_wood);
	_materials.push_back(mat_metal);
	_materials.push_back(mat_bouncy);
	_materials.push_back(mat_pillow);
}


Physics2D::~Physics2D() {
    for (auto it_collision : _collisions) {
        delete it_collision;
    }
    _collisions.clear();
    for (auto it_body : _bodies) {
        delete it_body;
    }
    _bodies.clear();

	for (auto it_material : _materials) {
		delete it_material;
	}
	_materials.clear();

	for (auto it_polygon : _polygons) {
		delete it_polygon;
	}
	_polygons.clear();
}


void Physics2D::add_polygon(Polygon2D * polygon) {
	_polygons.push_back(polygon);
}


void Physics2D::add_body(unsigned int idx_polygon, unsigned int idx_material, pt_type position, number orientation) {
	if (idx_polygon>= _polygons.size()) {
		cout << "Physics2D::add_body : bad idx_polygon\n";
		return;
	}
	if (idx_material>= _materials.size()) {
		cout << "Physics2D::add_body : bad idx_material\n";
		return;
	}
    RigidBody2D * body= new RigidBody2D(_polygons[idx_polygon], _materials[idx_material], position, orientation);
    _bodies.push_back(body);
}


// cf https://gafferongames.com/post/integration_basics/
// explicit Euler est instable ; il faut faire du semi-implicite Euler
// ce qui revient à faire évoluer la vitesse AVANT la position
// alternative à tester : Runge-Kutta
void Physics2D::step(bool verbose) {
	if ((_paused) || (!_bodies.size())) {
		return;
	}
    
    // remplissage _collisions
    for (auto it_collision : _collisions) {
        delete it_collision;
    }
    _collisions.clear();
    
	for (unsigned int i=0; i<_bodies.size()- 1; ++i) {
        for (unsigned int j=i+ 1; j<_bodies.size(); ++j) {
            RigidBody2D * body_a= _bodies[i];
            RigidBody2D * body_b= _bodies[j];
            
            // 2 corps inamovibles qui peuvent s'intersecter
            if ((body_a->_is_static) && (body_b->_is_static)) {
                continue;
            }
            
            Collision2D * collision= new Collision2D(body_a, body_b, verbose);
            if (collision->_contacts.size()) {
                _collisions.push_back(collision);
            }
			else {
				delete collision;
			}
        }
    }

    // maj des états précédents (utile a l'affichage (fixed time step cf https://gafferongames.com/post/fix_your_timestep/))
    for (auto it_body : _bodies) {
        it_body->update_previous_state();
    }

    // integration forces
    for (auto it_body : _bodies) {
        it_body->integrate_forces(_dt);
    }

    if (_warm_starting_enabled) {
        for (auto it_collision : _collisions) {
            if (_collisions_hash_table.find(it_collision->_hash)!= _collisions_hash_table.end()) {
                if (verbose) {
                    cout << "init coll with hash\n";
                }
                it_collision->_warm_starting= true;
                for (unsigned int i=0; i<_collisions_hash_table[it_collision->_hash]->_normal_impulses.size(); ++i) {
                    if (i< it_collision->_contacts.size()) {
                        it_collision->_contacts[i]->_normal_impulse= 0.98* pt_type(_collisions_hash_table[it_collision->_hash]->_normal_impulses[i]);
                    }
                }
            }
        }
    }

    // résolution contacts
    /*for (unsigned int iter=0; iter<N_ITER_COLLISION; ++iter) {
        for (auto it_collision : _collisions) {
            it_collision->apply_impulse(_dt);
		}
	}*/
    
    unsigned int iter= 0;
    number max_diff= 0.0f;
    while (true) {
		if (!_collisions.size()) {
			break;
		}

		bool is_iter_valid= false;
        max_diff= -1e10;
        for (auto it_collision : _collisions) {
            it_collision->apply_impulse(_dt);
			if (it_collision->_is_valid) {
				is_iter_valid= true;
			}
			for (auto it_contact : it_collision->_contacts) {
                number normal_norm2= 0.0f;
				number tangent_norm2= 0.0f;
                if (it_contact->_is_valid) {
                    normal_norm2= glm::length2(it_contact->_normal_impulse);
                    if (it_contact->_is_tangent_valid) {
                        tangent_norm2= glm::length2(it_contact->_tangent_impulse);
                    }
                }
				max_diff= max(max(normal_norm2, tangent_norm2), max_diff);
			}
        }

        ++iter;
        if ((!is_iter_valid) || (iter>= N_ITER_MAX) || (max_diff< IMPULSE_TRESH)) {
            if (verbose) {
                cout << "Sortie apply impulse : " << is_iter_valid << " ; " << iter << " ; " << max_diff << "\n";
            }
            break;
        }
    }

    for (auto it_hash : _collisions_hash_table) {
        it_hash.second->_is_2delete= true;
    }

    for (auto it_collision : _collisions) {
        //cout << it_collision->_is_valid << it_collision->_is_resting << "\n";
        if ((!it_collision->_is_valid) || (!it_collision->_is_resting)) {
            continue;
        }

        if (_collisions_hash_table.find(it_collision->_hash)== _collisions_hash_table.end()) {
            _collisions_hash_table[it_collision->_hash]= new LastImpulse();
            if (verbose) {
                cout << "new hash\n";
            }
        }
        else {
            _collisions_hash_table[it_collision->_hash]->_is_2delete= false;
        }
        _collisions_hash_table[it_collision->_hash]->_normal_impulses.clear();
        for (auto it_contact : it_collision->_contacts) {
            _collisions_hash_table[it_collision->_hash]->_normal_impulses.push_back(pt_type(it_contact->_normal_impulse_cumul));
        }
    }

    for (auto it_hash=_collisions_hash_table.cbegin(); it_hash!=_collisions_hash_table.cend();) {
        if (it_hash->second->_is_2delete) {
            if (verbose) {
                cout << "deleting hash\n";
            }
            _collisions_hash_table.erase(it_hash++);
        }
        else {
            ++it_hash;
        }
    }

    // integration velocity
    for (auto it_body : _bodies) {
        it_body->integrate_velocity(_dt);
    }
    
    // correct positions
    for (auto it_collision : _collisions) {
        it_collision->positional_correction();
    }

    // remise à 0 des forces
    for (auto it_body : _bodies) {
        it_body->clear_forces();
    }

	// suppression objets tombés trop bas
	for (int i=_bodies.size()- 1; i>=0; --i) {
		if (_bodies[i]->_position.y< BODY_MIN_Y) {
			_bodies.erase(_bodies.begin()+ i);
		}
	}
}


void Physics2D::new_external_force(pt_type pt_begin, pt_type pt_end) {
    pt_type inter(0.0f);
    for (auto it_body : _bodies) {
        if (segment_intersects_body(pt_begin, pt_end, it_body, &inter)) {
            pt_type force= (pt_end- pt_begin)* NEW_EXTERNAL_FORCE_FACTOR;
            it_body->_force= force;
            it_body->_torque= cross2d(inter- it_body->_position, force);
        }
    }
}


void Physics2D::new_explosion(pt_type center, number radius) {
    const number C= 0.01f;

    pt_type proj(0.0f);
    for (auto it_body : _bodies) {
        number dist= distance_body_pt(it_body, center, &proj);
        if (dist< radius) {
            /*if (dist< C) {
                dist= C;
            }*/
            if (dist< EPSILON) {
                // ???
            }
            pt_type force= (proj- center)* NEW_EXPLOSION_FACTOR/ (dist* dist);
            it_body->_force= force;
            it_body->_torque= cross2d(proj- it_body->_position, force);
            cout << dist << " ; " << glm::to_string(force) << " ; " << it_body->_torque << "\n";
        }
    }
}


void Physics2D::load_body(string ch_file, unsigned int idx_material) {
	ifstream ifs(ch_file);
	string line;
    string current_operation= "pts";
    number x, y;
    Polygon2D * polygon= new Polygon2D();
    pt_type position(0.0f);
    number orientation= 0.0f;
    vector<number> pts;

    while (getline(ifs, line)) {
		istringstream iss(line);
		string s;
		iss >> s;
        if (s== "") {
            continue;
        }
        
        if (s== "#") {
            iss >> current_operation;
        }
        else if (current_operation== "pts") {
            x= stof(s);
            iss >> y;
            pts.push_back(x);
            pts.push_back(y);
        }
        else if (current_operation== "position") {
            x= stof(s);
            iss >> y;
            position.x= x;
            position.y= y;
        }
        else if (current_operation== "orientation") {
            x= stof(s);
            orientation= x;
        }
    }
    polygon->set_points(&pts[0], pts.size()/ 2);

    add_polygon(polygon);
    add_body(_polygons.size()- 1, idx_material, position, orientation);
}


// --------------------------------------------------------------------------------------------------------------
DebugPhysics2D::DebugPhysics2D() {

}


DebugPhysics2D::DebugPhysics2D(Physics2D * physics_2d, GLuint prog_draw_2d, ScreenGL * screengl) : 
	_physics_2d(physics_2d), _prog_draw(prog_draw_2d), _screengl(screengl), _n_pts(0), _n_bodies(0), _n_collisions(0), _n_contacts(0),
	_visible_pt(true), _visible_normal(false), _visible_center(false), _visible_vel_force(false), _visible_collision(false), _visible_contact(false)
{
	glGenBuffers(6, _buffers);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float)* 16);
	glm::mat4 glm_ortho= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);

    update(0.0f);
}


DebugPhysics2D::~DebugPhysics2D() {

}


void DebugPhysics2D::draw() {
	// pts ------------------------------------------------------------------------------------------
	if (_visible_pt) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_LINES, 0, _n_pts* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
	
	// normals ------------------------------------------------------------------------------------------
	if (_visible_normal) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_LINES, 0, _n_pts* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// centers ------------------------------------------------------------------------------------------
	if (_visible_center) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_POINTS, 0, _n_bodies* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// velocity & force -----------------------------------------------------------------------------------
	if (_visible_vel_force) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[3]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_LINES, 0, _n_bodies* 4);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// collisions -----------------------------------------------------------------------------------
	if (_visible_collision) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[4]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_LINES, 0, _n_contacts* 2);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// contacts -----------------------------------------------------------------------------------
	if (_visible_contact) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[5]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_POINTS, 0, _n_contacts);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}


void DebugPhysics2D::update(number alpha) {
    _n_pts= 0;
	_n_bodies= 0;
	_n_collisions= 0;
	_n_contacts= 0;
    for (auto it_body : _physics_2d->_bodies) {
		++_n_bodies;
        _n_pts+= it_body->_polygon->_pts.size();
    }
	for (auto it_collision : _physics_2d->_collisions) {
		++_n_collisions;
		_n_contacts+= it_collision->_contacts.size();
	}
	
	// pts ------------------------------------------------------------------------
	float data_pts[_n_pts* 10];
    unsigned int compt= 0;
    for (auto it_body : _physics_2d->_bodies) {
        // interpolation cf https://gafferongames.com/post/fix_your_timestep/
        mat interpolated_orientation_mat;
        rotation_float2mat((1.0- alpha)* it_body->_previous_state->_orientation+ alpha* it_body->_orientation, interpolated_orientation_mat);
        pt_type interpolated_position= (1.0f- alpha)* it_body->_previous_state->_position+ alpha* it_body->_position;

        for (unsigned int i=0; i<it_body->_polygon->_pts.size(); ++i) {
            pt_type pt_world_1= interpolated_orientation_mat* it_body->_polygon->_pts[i]+ interpolated_position;
			pt_type pt_world_2= interpolated_orientation_mat* it_body->_polygon->_pts[(i+ 1)% it_body->_polygon->_pts.size()]+ interpolated_position;

            data_pts[compt* 10+ 0]= float(pt_world_1.x);
            data_pts[compt* 10+ 1]= float(pt_world_1.y);
            data_pts[compt* 10+ 2]= 1.0f;
            data_pts[compt* 10+ 3]= 1.0f;
            data_pts[compt* 10+ 4]= 1.0f;

            data_pts[compt* 10+ 5]= float(pt_world_2.x);
            data_pts[compt* 10+ 6]= float(pt_world_2.y);
            data_pts[compt* 10+ 7]= 1.0f;
            data_pts[compt* 10+ 8]= 1.0f;
            data_pts[compt* 10+ 9]= 1.0f;

            ++compt;
        }
    }
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, _n_pts* 10* sizeof(float), data_pts, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normales ---------------------------------------------------------------------
	float data_normals[_n_pts* 10];

    compt= 0;
    for (auto it_body : _physics_2d->_bodies) {
        for (unsigned int i=0; i<it_body->_polygon->_pts.size(); ++i) {
            pt_type pt_world_1= it_body->_orientation_mat* it_body->_polygon->_pts[i]+ it_body->_position;
			pt_type pt_world_2= it_body->_orientation_mat* it_body->_polygon->_pts[(i+ 1)% it_body->_polygon->_pts.size()]+ it_body->_position;
			pt_type normal_1= 0.5* (pt_world_1+ pt_world_2);
			pt_type normal_2= normal_1+ it_body->_orientation_mat* it_body->_polygon->_normals[i];

            data_normals[compt* 10+ 0]= float(normal_1.x);
            data_normals[compt* 10+ 1]= float(normal_1.y);
            data_normals[compt* 10+ 2]= 1.0f;
            data_normals[compt* 10+ 3]= 0.0f;
            data_normals[compt* 10+ 4]= 0.0f;

            data_normals[compt* 10+ 5]= float(normal_2.x);
            data_normals[compt* 10+ 6]= float(normal_2.y);
            data_normals[compt* 10+ 7]= 1.0f;
            data_normals[compt* 10+ 8]= 0.0f;
            data_normals[compt* 10+ 9]= 0.0f;

            ++compt;
        }
    }
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, _n_pts* 10* sizeof(float), data_normals, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// centres et centroids -------------------------------------------------------------
	float data_centers[_n_bodies* 10];
    compt= 0;
    for (auto it_body : _physics_2d->_bodies) {
		pt_type center= it_body->_position;
		pt_type centroid= it_body->_orientation_mat* it_body->_polygon->_centroid+ it_body->_position;

		data_centers[compt* 10+ 0]= float(center.x);
		data_centers[compt* 10+ 1]= float(center.y);
		data_centers[compt* 10+ 2]= 0.0f;
		data_centers[compt* 10+ 3]= 1.0f;
		data_centers[compt* 10+ 4]= 0.0f;

		data_centers[compt* 10+ 5]= float(centroid.x);
		data_centers[compt* 10+ 6]= float(centroid.y);
		data_centers[compt* 10+ 7]= 0.0f;
		data_centers[compt* 10+ 8]= 0.0f;
		data_centers[compt* 10+ 9]= 1.0f;

		++compt;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, _n_bodies* 10* sizeof(float), data_centers, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// velocity et force -------------------------------------------------------------
	float data_vel_force[_n_bodies* 20];
    compt= 0;
    for (auto it_body : _physics_2d->_bodies) {
		pt_type centroid= it_body->_orientation_mat* it_body->_polygon->_centroid+ it_body->_position;
		pt_type velocity= centroid+ it_body->_velocity* 0.1; // facteur mult pour mieux y voir
		pt_type force= centroid+ it_body->_force;

		data_centers[compt* 20+ 0]= float(centroid.x);
		data_centers[compt* 20+ 1]= float(centroid.y);
		data_centers[compt* 20+ 2]= 0.4f;
		data_centers[compt* 20+ 3]= 0.4f;
		data_centers[compt* 20+ 4]= 0.0f;

		data_centers[compt* 20+ 5]= float(velocity.x);
		data_centers[compt* 20+ 6]= float(velocity.y);
		data_centers[compt* 20+ 7]= 0.4f;
		data_centers[compt* 20+ 8]= 0.4f;
		data_centers[compt* 20+ 9]= 0.0f;

		data_centers[compt* 20+ 10]= float(centroid.x);
		data_centers[compt* 20+ 11]= float(centroid.y);
		data_centers[compt* 20+ 12]= 0.0f;
		data_centers[compt* 20+ 13]= 0.4f;
		data_centers[compt* 20+ 14]= 0.4f;

		data_centers[compt* 20+ 15]= float(force.x);
		data_centers[compt* 20+ 16]= float(force.y);
		data_centers[compt* 20+ 17]= 0.0f;
		data_centers[compt* 20+ 18]= 0.4f;
		data_centers[compt* 20+ 19]= 0.4f;

		++compt;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, _n_bodies* 20* sizeof(float), data_centers, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// collisions --------------------------------------------------------------
	float data_collisions[_n_contacts* 10];
	compt= 0;
	for (auto it_collision : _physics_2d->_collisions) {
		for (auto it_contact : it_collision->_contacts) {
			pt_type normal= it_contact->_position+ it_collision->_normal* it_collision->_penetration;

			data_collisions[10* compt+ 0]= float(it_contact->_position.x);
			data_collisions[10* compt+ 1]= float(it_contact->_position.y);
			data_collisions[10* compt+ 2]= 1.0f;
			data_collisions[10* compt+ 3]= 1.0f;
			data_collisions[10* compt+ 4]= 0.0f;

			data_collisions[10* compt+ 5]= float(normal.x);
			data_collisions[10* compt+ 6]= float(normal.y);
			data_collisions[10* compt+ 7]= 0.5f;
			data_collisions[10* compt+ 8]= 0.5f;
			data_collisions[10* compt+ 9]= 0.0f;

			++compt;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, _n_contacts* 10* sizeof(float), data_collisions, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    // pts de contacts -------------------------------------------------------
    float data_contacts[_n_contacts* 5];
    compt= 0;
	for (auto it_collision : _physics_2d->_collisions) {
		for (auto it_contact : it_collision->_contacts) {
			data_contacts[5* compt+ 0]= float(it_contact->_position.x);
			data_contacts[5* compt+ 1]= float(it_contact->_position.y);
			data_contacts[5* compt+ 2]= 0.2f;
			data_contacts[5* compt+ 3]= 0.5f;
			data_contacts[5* compt+ 4]= 0.8f;

			++compt;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[5]);
	glBufferData(GL_ARRAY_BUFFER, _n_contacts* 5* sizeof(float), data_contacts, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
