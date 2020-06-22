#include "physics_2d.h"

using namespace std;


// renvoie la norme de la composante z du prod vectoriel
float cross2d(glm::vec2 v1, glm::vec2 v2) {
    return v1.x* v2.y- v1.y* v2.x;
}


bool cmp_points(glm::vec2 pt1, glm::vec2 pt2) {
    return pt1.x< pt2.x;
}


bool right_turn(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3) {
    glm::vec2 normal= glm::vec2(pt1.y- pt2.y, pt2.x- pt1.x);
    return glm::dot(normal, pt3- pt1)< 0.0f;
}


// cf https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
/*bool segment_intersect(glm::vec2 pt1, glm::vec2 dir1, glm::vec2 pt2, glm::vec2 dir2, glm::vec2 * result) {
    float a= cross2d(dir1, dir2);
    if (a== 0.0f) {
        return false;
    }
    float t1= cross2d(pt2- pt1, dir2)/ a;
    if ((t1< 0.0f) || (t1> 1.0f)) {
        return false;
    }
    float t2= cross2d(pt2- pt1, dir1)/ a;
    if ((t2< 0.0f) || (t2> 1.0f)) {
        return false;
    }
    result->x= pt1.x+ t1* dir1.x;
    result->y= pt1.y+ t1* dir1.y;
    return true;
}*/


bool segment_intersects_line(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 line_origin, glm::vec2 line_direction, glm::vec2 * result) {
    glm::vec2 seg_dir= seg2- seg1;
    float a= cross2d(seg_dir, line_direction);
    if (a== 0.0f) {
        return false;
    }
    float t= cross2d(line_origin- seg1, line_direction)/ a;
    if ((t< 0.0f) || (t> 1.0f)) {
        return false;
    }
    result->x= seg1.x+ t* seg_dir.x;
    result->y= seg1.y+ t* seg_dir.y;
    return true;
}


bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test) {
    return cross2d(glm::vec2(pt_test- pt_ref), dir_ref)< 0.0f;
}


float distance_segment_pt(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 pt) {
    float seg_norm2= glm::distance2(seg1, seg2);
    if (seg_norm2== 0.0f) {
        return glm::distance(seg1, pt);
    }
    float t= max(0.0f, min(1.0f, glm::dot(pt- seg1, seg2- seg1)/ seg_norm2));
    glm::vec2 proj= seg1+ t* (seg2- seg1);
    return glm::distance(pt, proj);
}


void axis_least_penetration(RigidBody2D * body_a, RigidBody2D * body_b, unsigned int * idx_pt_max, float * penetration_max) {
    *penetration_max= -FLT_MAX;
    *idx_pt_max= 0;
    for (unsigned int idx_pt=0; idx_pt<body_a->_polygon->_pts.size(); ++idx_pt) {
        // normale dans le repère body_a
        glm::vec2 normal_a= body_a->_polygon->_normals[idx_pt];
        // normale dans le repère body_b
        glm::vec2 normal_b= glm::transpose(body_b->_orientation_mat)* body_a->_orientation_mat* normal_a;
        // pt de body_b le + éloigné dans la direction opposée à la normale dans le repère body_b
        glm::vec2 farthest_pt= body_b->_polygon->farthest_pt_along_dir(-normal_b);
        // sommet dans le repère body_a
        glm::vec2 pt_a= body_a->_polygon->_pts[idx_pt];
        // sommet dans le repère body_b
        glm::vec2 pt_b= glm::transpose(body_b->_orientation_mat)* (body_a->_orientation_mat* pt_a+ body_a->_position- body_b->_position);
        // plus grande pénétration dans le repère body_b
        float penetration= glm::dot(normal_b, farthest_pt- pt_b);
        if (*penetration_max< penetration) {
            *penetration_max= penetration;
            *idx_pt_max= idx_pt;
        }
    }
}


// permet d'avoir + de stabilité lors du choix de la normale le long de laquelle appliquer un impulse
// dans des configs où les 2 faces des objets s'interpénétrant sont parallèles
bool biased_cmp(float a, float b) {
    const float bias_relative= 0.95f;
    const float bias_absolute= 0.01f;
    return a>= b* bias_relative+ a* bias_absolute;
}


// recherche de la face du polygon incident
unsigned int incident_face_idx(RigidBody2D * body_reference, RigidBody2D * body_incident, unsigned int reference_face_idx) {
    glm::vec2 reference_normal= body_reference->_polygon->_normals[reference_face_idx];
    // passage repere body_incident
    reference_normal= glm::transpose(body_incident->_orientation_mat)* body_reference->_orientation_mat* reference_normal;
    // recherche de la normale la plus opposée
    unsigned int incident_face_idx= 0;
    float min_dot= FLT_MAX;
    for (unsigned int idx_pt=0; idx_pt<body_incident->_polygon->_pts.size(); ++idx_pt) {
        float dot= glm::dot(reference_normal, body_incident->_polygon->_normals[idx_pt]);
        if (dot< min_dot) {
            min_dot= dot;
            incident_face_idx= idx_pt;
        }
    }
    return incident_face_idx;
}


// --------------------------------------------------------------------------------------------------------------
Polygon2D::Polygon2D() : _area(0.0f), _mass(0.0f), _mass_inv(0.0f), _density(1.0f), _inertia_moment(0.0f), _inertia_moment_inv(0.0f) {

}


Polygon2D::~Polygon2D() {
    _pts.clear();
    _normals.clear();
}


void Polygon2D::set_points(float * points, unsigned int n_points) {
    _pts.clear();
    for (unsigned int i=0; i<n_points; ++i) {
        _pts.push_back(glm::vec2(points[2* i], points[2* i+ 1]));
    }
    update_attributes();
}


// calcul convex hull 2D bouquin computational geom
void Polygon2D::randomize(unsigned int n_points) {
    vector<glm::vec2> pts;
    for (unsigned int i=0; i<n_points; ++i) {
        pts.push_back(glm::vec2(rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f)));
    }
    sort(pts.begin(), pts.end(), cmp_points);

    vector<glm::vec2> pts_upper;
    pts_upper.push_back(pts[0]);
    pts_upper.push_back(pts[1]);
    for (unsigned int i=2; i<pts.size(); ++i) {
        pts_upper.push_back(pts[i]);
        while ((pts_upper.size()> 2) && (!right_turn(pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 2], pts_upper[pts_upper.size()- 1]))) {
            pts_upper.erase(pts_upper.end()- 2);
        }
    }

    vector<glm::vec2> pts_lower;
    pts_lower.push_back(pts[pts.size()- 1]);
    pts_lower.push_back(pts[pts.size()- 2]);
    for (int i=pts.size()- 3; i>=0; --i) {
        pts_lower.push_back(pts[i]);
        while ((pts_lower.size()> 2) && (!right_turn(pts_lower[pts_lower.size()- 3], pts_lower[pts_lower.size()- 2], pts_lower[pts_lower.size()- 1]))) {
            pts_lower.erase(pts_lower.end()- 2);
        }
    }

    pts_lower.erase(pts_lower.begin());
    pts_lower.erase(pts_lower.end()- 1);

    _pts.clear();
    _pts.insert(_pts.begin(), pts_upper.begin(), pts_upper.end());
    _pts.insert(_pts.end()  , pts_lower.begin(), pts_lower.end());

    // debug
    /*ofstream myfile;

    myfile.open("pts.txt");
    for (auto it_pt : pts) {
        myfile << it_pt.x << " " << it_pt.y << "\n";
    }
    myfile.close();

    myfile.open("hull.txt");
    for (auto it_pt : _pts) {
        myfile << it_pt.x << " " << it_pt.y << "\n";
    }
    myfile.close();*/

    update_attributes();
}


void Polygon2D::update_attributes(float density) {
    _density= density;
    
    // calcul aire et masse
    _area= 0.0f;
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _area+= 0.5f* (pt1.x* pt2.y- pt1.y* pt2.x);
    }
    _mass= _area* _density;

    // calcul centre de gravité
    _centroid= glm::vec2(0.0f);
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _centroid+= (0.5f* THIRD/ _area)* (pt1.x* pt2.y- pt1.y* pt2.x)* (pt1+ pt2);
    }

    // calcul moment d'inertie
    _inertia_moment= 0.0f;
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _inertia_moment+= 0.25f* THIRD* _density* (pt1.x* pt2.y- pt1.y* pt2.x)* (glm::dot(pt1, pt1)+ glm::dot(pt1, pt2)+ glm::dot(pt2, pt2));
    }

    // si clockwise -> anticlockwise
    if (_area< 0.0f) {
        _area*= -1.0f;
        _mass*= -1.0f;
        _inertia_moment*= -1.0f;
        reverse(_pts.begin(), _pts.end());
    }

    if (_mass== 0.0f) {
        _mass_inv= 0.0f;
    }
    else {
        _mass_inv= 1.0f/ _mass;
    }
    
    if (_inertia_moment== 0.0f) {
        _inertia_moment_inv= 0.0f;
    }
    else {
        _inertia_moment_inv= 1.0f/ _inertia_moment;
    }

    // calcul normales (norme == 1)
    // on doit etre en anticlockwise a ce moment ; on fait une rotation de -90 ie (x,y)->(y,-x)
    // pour que la normale pointe vers l'extérieur du polygone
    _normals.clear();
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _normals.push_back(glm::normalize(glm::vec2(pt2.y- pt1.y, pt1.x- pt2.x)));
    }
}


void Polygon2D::print() {
	cout << "area= " << _area << "\n";
    cout << "density= " << _density << "\n";
    cout << "mass= " << _mass << " ; mass_inv= " << _mass_inv << "\n";
    cout << "centroid= " << _centroid.x << " ; " << _centroid.y << "\n";
    cout << "inertia_moment= " << _inertia_moment << " ; inertia_moment_inv= " << _inertia_moment_inv << "\n";
}


glm::vec2 Polygon2D::farthest_pt_along_dir(glm::vec2 direction) {
    float dist_max= -FLT_MAX;
    glm::vec2 farthest_pt;
    for (unsigned int idx_pt=0; idx_pt<_pts.size(); ++idx_pt) {
        float dist= glm::dot(direction, _pts[idx_pt]);
        if (dist> dist_max) {
            dist_max= dist;
            farthest_pt= _pts[idx_pt];
        }
    }

    return farthest_pt;
}


// --------------------------------------------------------------------------------------------------------------
RigidBody2D::RigidBody2D() {
    _polygon= new Polygon2D();
    _polygon->randomize(10);

}


RigidBody2D::RigidBody2D(Polygon2D * polygon, glm::vec2 position, float orientation) :
    _polygon(polygon), _position(position), _velocity(glm::vec2(0.0f)), _angular_velocity(0.0f), _force(glm::vec2(0.0f)), _torque(0.0f),
    _static_friction(0.5f), _dynamic_friction(0.3f), _restitution(0.2f)
{
    set_orientation(orientation);
}


RigidBody2D::~RigidBody2D() {
    
}


void RigidBody2D::set_orientation(float orientation) {
    _orientation= orientation;
    // glm est en column-major order par défaut -> mat[col][row]
    _orientation_mat[0][0]= cos(_orientation);
    _orientation_mat[0][1]= sin(_orientation);
    _orientation_mat[1][0]= -sin(_orientation);
    _orientation_mat[1][1]= cos(_orientation);
}


void RigidBody2D::integrate_forces(float dt) {
    // inamovible
    if (_polygon->_mass_inv== 0.0f) {
        return;
    }
    _velocity+= (_force* _polygon->_mass_inv+ GRAVITY)* dt* 0.5f;
    _angular_velocity+= _torque* _polygon->_inertia_moment_inv* dt* 0.5f;
}


void RigidBody2D::integrate_velocity(float dt) {
    // inamovible
    if (_polygon->_mass_inv== 0.0f) {
        return;
    }
    _position+= _velocity* dt;
    set_orientation(_orientation+ _angular_velocity* dt);
    integrate_forces(dt); // pourquoi le refait-on là ?
}


void RigidBody2D::apply_impulse(glm::vec2 impulse, glm::vec2 contact) {
    _velocity+= _polygon->_mass_inv* impulse;
    _angular_velocity+= _polygon->_inertia_moment_inv* cross2d(contact, impulse);
}


void RigidBody2D::clear_forces() {
    _force= glm::vec2(0.0f);
    _torque= 0.0f;
}


// --------------------------------------------------------------------------------------------------------------
Collision2D::Collision2D() {

}


Collision2D::Collision2D(RigidBody2D * body_a, RigidBody2D * body_b) : _body_a(body_a), _body_b(body_b) {

}


Collision2D::~Collision2D() {
    
}


void Collision2D::solve() {
    unsigned int idx_pt_max_a, idx_pt_max_b;
    float penetration_max_a, penetration_max_b;
    
    _contacts.clear();

    // axe de moindre pénétration ; si pénétration >= 0 on a trouvé un axe de séparation entre les 2 polys, pas d'intersection
    axis_least_penetration(_body_a, _body_b, &idx_pt_max_a, &penetration_max_a);
    if (penetration_max_a>= 0.0f) {
        return;
    }

    axis_least_penetration(_body_b, _body_a, &idx_pt_max_b, &penetration_max_b);
    if (penetration_max_b>= 0.0f) {
        return;
    }

    RigidBody2D * body_reference;
    RigidBody2D * body_incident;
    unsigned int ref_face_idx;
    // identification face de réf, face d'incidence
    if (biased_cmp(penetration_max_a, penetration_max_b)) {
        body_reference= _body_a;
        body_incident = _body_b;
        ref_face_idx= idx_pt_max_a;
    }
    else {
        body_reference= _body_b;
        body_incident = _body_a;
        ref_face_idx= idx_pt_max_b;
    }

    // recup idx face incidence
    unsigned int incid_face_idx= incident_face_idx(body_reference, body_incident, ref_face_idx);

    // sommets impliqués
    glm::vec2 ref_1= body_reference->_polygon->_pts[ref_face_idx];
    glm::vec2 ref_2= body_reference->_polygon->_pts[(ref_face_idx+ 1)% body_reference->_polygon->_pts.size()];
    glm::vec2 incid_1= body_incident->_polygon->_pts[incid_face_idx];
    glm::vec2 incid_2= body_incident->_polygon->_pts[(incid_face_idx+ 1)% body_incident->_polygon->_pts.size()];

    // dans le système world
    ref_1= body_reference->_orientation_mat* ref_1+ body_reference->_position;
    ref_2= body_reference->_orientation_mat* ref_2+ body_reference->_position;
    incid_1= body_incident->_orientation_mat* incid_1+ body_incident->_position;
    incid_2= body_incident->_orientation_mat* incid_2+ body_incident->_position;

    // _normal dans le système world
    _normal= body_reference->_orientation_mat* body_reference->_polygon->_normals[ref_face_idx];

    // clip
    glm::vec2 inter_1(0.0f);
    glm::vec2 inter_2(0.0f);
    bool is_intersect_ref_1= segment_intersects_line(incid_1, incid_2, ref_1, _normal, &inter_1);
    bool is_intersect_ref_2= segment_intersects_line(incid_1, incid_2, ref_2, _normal, &inter_2);
    bool is_incid_1_unclipped= true;
    bool is_incid_2_unclipped= true;
    if ((!is_left(ref_1, _normal, incid_1)) || (is_left(ref_2, _normal, incid_1))) {
        is_incid_1_unclipped= false;
    }
    if ((!is_left(ref_1, _normal, incid_2)) || (is_left(ref_2, _normal, incid_2))) {
        is_incid_2_unclipped= false;
    }

    vector<glm::vec2> contacts;
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
    for (auto it_contact : contacts) {
        if (is_left(ref_1, ref_2- ref_1, it_contact)) {
            _contacts.push_back(it_contact);
        }
    }
    _penetration= 0.0f;
    for (auto it_contact : _contacts) {
        _penetration+= distance_segment_pt(ref_1, ref_2, it_contact);
    }
    _penetration/= _contacts.size();
}


glm::vec2 Collision2D::contact_relative_velocity(unsigned int idx_contact) {
    // incertitudes a ce niveau ; cf https://social.msdn.microsoft.com/Forums/vstudio/en-US/65526d49-5107-4ffc-b25e-1b5a619afcb3/2d-rigidbody-physics-engine-bug?forum=vbgeneral
    glm::vec2 va= _contacts[idx_contact]- _body_a->_position;
    glm::vec2 contact_velocity_a= _body_a->_velocity+ _body_a->_angular_velocity* glm::vec2(-va.y, va.x);

    glm::vec2 vb= _contacts[idx_contact]- _body_b->_position;
    glm::vec2 contact_velocity_b= _body_b->_velocity+ _body_b->_angular_velocity* glm::vec2(-vb.y, vb.x);

    return contact_velocity_b- contact_velocity_a;
}


void Collision2D::initialize(float dt) {
    // bizarrement dans le tuto il fait mixed_static = sqrt(static_a**2 + static_b**2) ; essayer ...
    _mixed_static_friction= sqrt(_body_a->_static_friction* _body_b->_static_friction);
    _mixed_dynamic_friction= sqrt(_body_a->_dynamic_friction* _body_b->_dynamic_friction);
    
    // Determine if we should perform a resting collision or not
    // The idea is if the only thing moving this object is gravity,
    // then the collision should be performed without any restitution
    _mixed_restitution= min(_body_a->_restitution, _body_b->_restitution);
    for (unsigned int idx_contact=0; idx_contact<_contacts.size(); ++idx_contact) {
        glm::vec2 rel_vel= contact_relative_velocity(idx_contact);
        if (glm::length2(rel_vel)< glm::length2(dt* GRAVITY)+ 0.0001f) {
            _mixed_restitution= 0.0f;
        }
    }
}


void Collision2D::apply_impulse() {
    // les 2 sont inamovibles
    if ((_body_a->_polygon->_mass_inv< 1e-5) && (_body_b->_polygon->_mass_inv< 1e-5)) {
        _body_a->_velocity= glm::vec2(0.0f);
        _body_b->_velocity= glm::vec2(0.0f);
        return;
    }

    for (unsigned int idx_contact=0; idx_contact<_contacts.size(); ++idx_contact) {
        // vecteurs du centre du corps au contact
        glm::vec2 ra= _contacts[idx_contact]- _body_a->_position;
        glm::vec2 rb= _contacts[idx_contact]- _body_b->_position;

        // impulse selon la normale ------------------------------------------------------
        
        // vitesse relative du contact en B par rapport à A
        glm::vec2 rel_vel= contact_relative_velocity(idx_contact);
        // projetée sur la normale
        float rel_vel_normal= glm::dot(rel_vel, _normal);

        // objets s'éloignent
        if (rel_vel_normal< 0.0f) {
            return;
        }

        // écart angulaire à _normal
        float cross_norm_a= cross2d(ra, _normal);
        float cross_norm_b= cross2d(rb, _normal);

        // dénominateur formule calcul j
        float mass_inv_sum= _body_a->_polygon->_mass_inv+ _body_b->_polygon->_mass_inv+ cross_norm_a* cross_norm_a* _body_a->_polygon->_inertia_moment_inv+ cross_norm_b* cross_norm_b* _body_b->_polygon->_inertia_moment_inv;

        // j est l'amplitude de l'impulsion à appliquer
        float j= -(1.0f+ _mixed_restitution)* rel_vel_normal;
        j/= mass_inv_sum;
        j/= _contacts.size(); // pourquoi ?

        // on applique l'impulsion
        _body_a->apply_impulse(-j* _normal, ra);
        _body_b->apply_impulse( j* _normal, rb);

        // impulse tangentielle -> friction --------------------------------------------

        // on doit recalculer car les apply_impulse ont modifié la vitesse
        rel_vel= contact_relative_velocity(idx_contact);

        // projetée sur la tangente et normalisée
        glm::vec2 rel_vel_tan= glm::normalize(rel_vel- glm::dot(rel_vel, _normal)* _normal);

        // jt est l'amplitude de l'impulsion tangentielle
        float jt= -glm::dot(rel_vel_tan, rel_vel);
        jt/= mass_inv_sum;
        jt/= _contacts.size(); // pourquoi ?

        // on ignore les petites frictions
        if (abs(jt)< 1e-5) {
            return;
        }

        // loi de Coulomb
        glm::vec2 tangent_impulse;
        if (abs(jt)< j* _mixed_static_friction) {
            tangent_impulse= rel_vel_tan* jt;
        }
        else {
            tangent_impulse= rel_vel_tan* (-j)* _mixed_dynamic_friction;
        }

        // Apply friction impulse
        _body_a->apply_impulse(-tangent_impulse, ra);
        _body_b->apply_impulse( tangent_impulse, rb);
    }
}


// empeche les corps de s'enfoncer les uns dans les autres ; on écarte légèrement les 2 corps le long de _normal
void Collision2D::positional_correction() {
    glm::vec2 correction= (max(_penetration- PENETRATION_ALLOWANCE, 0.0f)/ (_body_a->_polygon->_mass_inv+ _body_b->_polygon->_mass_inv))* _normal* PENETRATION_PERCENT2CORRECT;
    _body_a->_position-= correction* _body_a->_polygon->_mass_inv;
    _body_b->_position+= correction* _body_b->_polygon->_mass_inv;
}


// --------------------------------------------------------------------------------------------------------------
Physics2D::Physics2D() {

}


Physics2D::Physics2D(float dt, unsigned int n_iterations_collision) : _dt(dt), _n_iterations_collision(n_iterations_collision) {

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
}


void Physics2D::add_body(Polygon2D * polygon, glm::vec2 position, float orientation) {
    RigidBody2D * body= new RigidBody2D(polygon, position, orientation);
    _bodies.push_back(body);
}


void Physics2D::step() {
    // remplissage _collisions
    _collisions.clear();
    for (unsigned int i=0; i<_bodies.size()- 1; ++i) {
        for (unsigned int j=i+ 1; j<_bodies.size(); ++j) {
            RigidBody2D * body_a= _bodies[i];
            RigidBody2D * body_b= _bodies[j];
            
            // 2 corps inamovibles qui peuvent s'intersecter
            if ((body_a->_polygon->_mass_inv== 0.0f) && (body_b->_polygon->_mass_inv== 0.0f)) {
                continue;
            }
            
            Collision2D * collision= new Collision2D(body_a, body_b);
            collision->solve();
            if (collision->_contacts.size()) {
                _collisions.push_back(collision);
            }
        }
    }

    // integration forces
    for (auto it_body : _bodies) {
        it_body->integrate_forces(_dt);
    }

    // init contacts
    for (auto it_collision : _collisions) {
        it_collision->initialize(_dt);
    }

    // résolution contacts
    for (unsigned int iter=0; iter<_n_iterations_collision; ++iter) {
        for (auto it_collision : _collisions) {
            it_collision->apply_impulse();
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
}


// --------------------------------------------------------------------------------------------------------------
DebugPhysics2D::DebugPhysics2D() {

}


DebugPhysics2D::DebugPhysics2D(Physics2D * physics_2d, GLuint prog_draw_2d, ScreenGL * screengl) : _physics_2d(physics_2d), _prog_draw(prog_draw_2d), _screengl(screengl), _n_pts(0) {
	glGenBuffers(1, &_buffer);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float)* 16);
	glm::mat4 glm_ortho= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);

    update();
}


DebugPhysics2D::~DebugPhysics2D() {

}


void DebugPhysics2D::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINE_LOOP, 0, _n_pts);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void DebugPhysics2D::update() {
    _n_pts= 0;
    for (auto it_body : _physics_2d->_bodies) {
        _n_pts+= it_body->_polygon->_pts.size();
    }
	float data[_n_pts* 5];
    unsigned int compt= 0;
    for (auto it_body : _physics_2d->_bodies) {
        for (auto it_pt : it_body->_polygon->_pts) {
            glm::vec2 pt_world= it_body->_orientation_mat* it_pt+ it_body->_position;
            data[compt* 5+ 0]= pt_world.x;
            data[compt* 5+ 1]= pt_world.y;
            data[compt* 5+ 2]= 1.0f;
            data[compt* 5+ 3]= 1.0f;
            data[compt* 5+ 4]= 1.0f;
            ++compt;
        }
    }
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_pts* 5* sizeof(float), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
