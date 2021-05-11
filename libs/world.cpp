
#include "world.h"

using namespace std;
using namespace rapidxml;



ModelsPool::ModelsPool() {

}


ModelsPool::ModelsPool(GLuint prog_3d_anim, GLuint prog_3d_obj, GLuint prog_basic) : _prog_3d_anim(prog_3d_anim), _prog_3d_obj(prog_3d_obj), _prog_basic(prog_basic) {

}


ModelsPool::~ModelsPool() {
	clear();
}


void ModelsPool::add_static_model(string ch_config_file) {
	if (get_static_model(ch_config_file)) {
		return;
	}
	StaticModel * sm= new StaticModel(ch_config_file, _prog_3d_obj);
	_static_models.push_back(sm);
}


void ModelsPool::add_animated_model(string ch_config_file) {
	if (get_animated_model(ch_config_file)) {
		return;
	}
	AnimatedModel * am= new AnimatedModel(ch_config_file, _prog_3d_anim);
	_animated_models.push_back(am);
}


StaticModel * ModelsPool::get_static_model(string ch_config_file) {
	for (auto sm : _static_models) {
		if (sm->_ch_config_file== ch_config_file) {
			return sm;
		}
	}
	return NULL;
}


AnimatedModel * ModelsPool::get_animated_model(string ch_config_file) {
	for (auto am : _animated_models) {
		if (am->_ch_config_file== ch_config_file) {
			return am;
		}
	}
	return NULL;
}


void ModelsPool::clear() {
	for (auto am : _animated_models) {
		delete am;
	}
	_animated_models.clear();
	
	for (auto sm : _static_models) {
		delete sm;
	}
	_static_models.clear();
}


// ------------------------------------------------------------------------------------------
/*Square::Square() {

}


Square::Square(const glm::vec2 & vmin, const glm::vec2 & vmax) : _vmin(vmin), _vmax(vmax) {

}


Square::~Square() {

}
*/

// ---------------------------------------------------------------------------------------------------------------------
QuadNode::QuadNode() {

}


QuadNode::QuadNode(const glm::vec2 & vmin, const glm::vec2 & vmax, unsigned int depth) : _depth(depth) {
	// valeurs min , max initiales du z du AABB du node ; la valeur max sera ensuite écrasée avec le z max des objets associés au noeud
	// mais dans un 1er temps on a besoin d'une AABB assez haute pour le test d'intersection avec les objets qui determinera si 1 objet doit 
	// etre associé au noeud. ATTENTION de ne pas mettre un zmin trop bas car sinon lors du set_active la caméra intersectera tout le temps l'AABB
	float Z_MIN_INIT_QUADNODE= -1.0f;
	float Z_MAX_INIT_QUADNODE= 10000.0f;
	_aabb= new AABB(glm::vec3(vmin.x, vmin.y, Z_MIN_INIT_QUADNODE), glm::vec3(vmax.x, vmax.y, Z_MAX_INIT_QUADNODE));
}


QuadNode::~QuadNode() {
	delete _aabb;
}


// ---------------------------------------------------------------------------------------------------------------------
QuadTree::QuadTree() {

}


QuadTree::QuadTree(const glm::vec2 & vmin, const glm::vec2 & vmax, unsigned int depth) : _depth(depth) {
	_root= new QuadNode(vmin, vmax, 0);
	recursive_init(_root);
}


QuadTree::~QuadTree() {
	recursive_delete(_root);
}


void QuadTree::recursive_init(QuadNode * node) {
	if (node->_depth< _depth) {
		// on sépare le AABB en 4
		float x_min= node->_aabb->_vmin.x;
		float x_max= node->_aabb->_vmax.x;
		float x_middle= (x_min+ x_max)* 0.5f;
		float y_min= node->_aabb->_vmin.y;
		float y_max= node->_aabb->_vmax.y;
		float y_middle= (y_min+ y_max)* 0.5f;
		
		node->_child_so= new QuadNode(glm::vec2(x_min, y_min)      , glm::vec2(x_middle, y_middle), node->_depth+ 1);
		node->_child_se= new QuadNode(glm::vec2(x_middle, y_min)   , glm::vec2(x_max, y_middle)   , node->_depth+ 1);
		node->_child_no= new QuadNode(glm::vec2(x_min, y_middle)   , glm::vec2(x_middle, y_max)   , node->_depth+ 1);
		node->_child_ne= new QuadNode(glm::vec2(x_middle, y_middle), glm::vec2(x_max, y_max)      , node->_depth+ 1);
		
		recursive_init(node->_child_so);
		recursive_init(node->_child_se);
		recursive_init(node->_child_no);
		recursive_init(node->_child_ne);
	}
}


void QuadTree::recursive_delete(QuadNode * node) {
	if (node->_depth< _depth) {
		recursive_delete(node->_child_so);
		recursive_delete(node->_child_se);
		recursive_delete(node->_child_no);
		recursive_delete(node->_child_ne);
	}
	delete node;
}


void QuadTree::add_position(InstancePosRot * position) {
	recursive_add_position(_root, position);
}


void QuadTree::recursive_add_position(QuadNode * node, InstancePosRot * pos_rot) {
	if (node->_depth== _depth) { // feuille
		node->_pos_rots.push_back(pos_rot);
	}
	else {
		if (aabb_intersects_bbox(node->_child_so->_aabb, pos_rot->_bbox)) {
			recursive_add_position(node->_child_so, pos_rot);
		}
		if (aabb_intersects_bbox(node->_child_se->_aabb, pos_rot->_bbox)) {
			recursive_add_position(node->_child_se, pos_rot);
		}
		if (aabb_intersects_bbox(node->_child_no->_aabb, pos_rot->_bbox)) {
			recursive_add_position(node->_child_no, pos_rot);
		}
		if (aabb_intersects_bbox(node->_child_ne->_aabb, pos_rot->_bbox)) {
			recursive_add_position(node->_child_ne, pos_rot);
		}
	}
}


// recherche du noeud qui comprend le point v
QuadNode * QuadTree::find_node(unsigned int depth, const glm::vec2 & v) {
	if (depth> _depth) {
		cout << "QuadTree::find_node depth " << depth << " non supporte" << endl;
		return NULL;
	}
	QuadNode * result= NULL;
	recursive_find_node(_root, depth, v, &result);
	return result;
}


void QuadTree::recursive_find_node(QuadNode * node, unsigned int depth, const glm::vec2 & v, QuadNode ** node_result) {
	if (node->_depth== depth) {
		*node_result= node;
	}
	else {
		float x_min= node->_aabb->_vmin.x;
		float x_max= node->_aabb->_vmax.x;
		float x_middle= (x_min+ x_max)* 0.5f;
		float y_min= node->_aabb->_vmin.y;
		float y_max= node->_aabb->_vmax.y;
		float y_middle= (y_min+ y_max)* 0.5f;

		if ((v.x< x_middle) && (v.y< y_middle)) {
			recursive_find_node(node->_child_so, depth, v, node_result);
		}
		else if ((v.x> x_middle) && (v.y< y_middle)) {
			recursive_find_node(node->_child_se, depth, v, node_result);
		}
		else if ((v.x< x_middle) && (v.y> y_middle)) {
			recursive_find_node(node->_child_no, depth, v, node_result);
		}
		else if ((v.x> x_middle) && (v.y> y_middle)) {
			recursive_find_node(node->_child_ne, depth, v, node_result);
		}
	}
}


// a faire une fois que les objets ont été ajoutés a l'arbre a travers add_position
void QuadTree::set_z_max(float * altis, unsigned int width_n, unsigned int height_n) {
	unsigned int k= pow(2, _depth);
	float step_x= (_root->_aabb->_vmax.x- _root->_aabb->_vmin.x)/ (float)(k);
	float step_y= (_root->_aabb->_vmax.y- _root->_aabb->_vmin.y)/ (float)(k);
	// on suppose que l'emprise du terrain est celle de _root
	float step_x_level= (_root->_aabb->_vmax.x- _root->_aabb->_vmin.x)/ (float)(width_n);
	float step_y_level= (_root->_aabb->_vmax.y- _root->_aabb->_vmin.y)/ (float)(height_n);

	// on commence par fixer le z max des feuilles a partir des altis et des objets associés a la feuille
	for (unsigned int i=0; i<k; ++i) {
		for (unsigned int j=0; j<k; ++j) {
			float xmin= _root->_aabb->_vmin.x+ step_x* (float)(i);
			float xmax= xmin+ step_x;
			float ymin= _root->_aabb->_vmin.y+ step_y* (float)(j);
			float ymax= ymin+ step_y;
			unsigned int imin= (unsigned int)(floor(xmin/ step_x_level));
			unsigned int imax= (unsigned int)(ceil(xmax/ step_x_level));
			unsigned int jmin= (unsigned int)(floor(ymin/ step_y_level));
			unsigned int jmax= (unsigned int)(ceil(ymax/ step_y_level));
			
			float alti_max= 0.0f;
			for (unsigned int ii=imin; ii<imax; ++ii) {
				for (unsigned int jj=jmin; jj<jmax; ++jj) {
					if (alti_max< altis[ii+ (width_n+ 1)* jj]) {
						alti_max= altis[ii+ (width_n+ 1)* jj];
					}
				}
			}

			// on calcule pour chaque objet associé a la feuille son z + son rayon
			QuadNode * node= find_node(_depth, glm::vec2((xmin+ xmax)* 0.5f, (ymin+ ymax)* 0.5f));
			for (auto pr : node->_pos_rots) {
				if (pr->_position.z+ pr->_bbox->_radius* 2.0f> alti_max) {
					alti_max= pr->_position.z+ pr->_bbox->_radius* 2.0f;
				}
			}

			node->_aabb->_vmax.z= alti_max;
		}
	}

	// ensuite on en déduit le zmax des feuilles parentes en prenant le max des enfants
	for (int depth=_depth- 1; depth>=0; --depth) {
		recursive_set_z_max(_root, depth);
	}

}


void QuadTree::recursive_set_z_max(QuadNode * node, unsigned int depth) {
	if (node->_depth== depth) {
		node->_aabb->_vmax.z= node->_child_so->_aabb->_vmax.z;
		if (node->_aabb->_vmax.z< node->_child_se->_aabb->_vmax.z) {
			node->_aabb->_vmax.z= node->_child_se->_aabb->_vmax.z;
		}
		if (node->_aabb->_vmax.z< node->_child_no->_aabb->_vmax.z) {
			node->_aabb->_vmax.z= node->_child_no->_aabb->_vmax.z;
		}
		if (node->_aabb->_vmax.z< node->_child_ne->_aabb->_vmax.z) {
			node->_aabb->_vmax.z= node->_child_ne->_aabb->_vmax.z;
		}
	}
	else if (node->_depth< _depth) {
		recursive_set_z_max(node->_child_so, depth);
		recursive_set_z_max(node->_child_se, depth);
		recursive_set_z_max(node->_child_no, depth);
		recursive_set_z_max(node->_child_ne, depth);
	}
}


// methode la + importante : celle qui determinera quels objets doivent etre rendus
void QuadTree::set_actives(ViewSystem * view_system, float dist_max) {
	//_debug= 0;
	recursive_set_actives(_root, view_system, dist_max);
}


void QuadTree::recursive_set_actives(QuadNode * node, ViewSystem * view_system, float dist_max) {
	// l'ordre des tests est important ; tester ...
	if ((!view_system->intersects_aabb(node->_aabb)) || (aabb_distance_pt_2(node->_aabb, view_system->_eye)> dist_max* dist_max)) {
		return;
	}

	if (node->_depth== _depth) { // feuille
		for (auto pr : node->_pos_rots) {
			if (pr->_active) {
				continue;
			}

			//_debug++;
			
			// calcul du carré de la distance eye / position objet
			pr->update_dist2(view_system->_eye);
			if (pr->_dist2> dist_max* dist_max) {
				continue;
			}

			if (!view_system->intersects_bbox(pr->_bbox)) {
				continue;
			}

			pr->_active= true;
		}
	}
	else {
		recursive_set_actives(node->_child_so, view_system, dist_max);
		recursive_set_actives(node->_child_se, view_system, dist_max);
		recursive_set_actives(node->_child_no, view_system, dist_max);
		recursive_set_actives(node->_child_ne, view_system, dist_max);
	}
}


// fonction d'affichage pour debug
void QuadTree::print() {
	recursive_print(_root);
}


void QuadTree::recursive_print(QuadNode * node) {
	if (node->_depth== _depth) {
		unsigned int n_actives= 0;
		for (auto pr : node->_pos_rots) {
			if (pr->_active) {
				n_actives++;
			}
		}
		if (n_actives> 0) {
			cout << glm::to_string(node->_aabb->_vmin) << " ; " << glm::to_string(node->_aabb->_vmax) << " : " << n_actives << " / " << node->_pos_rots.size() << endl;
		}
	}
	else {
		recursive_print(node->_child_so);
		recursive_print(node->_child_se);
		recursive_print(node->_child_no);
		recursive_print(node->_child_ne);
	}
}


// ------------------------------------------------------------------------------------------
World::World() {

}


World::World(GLuint prog_3d_anim, GLuint prog_3d_terrain, GLuint prog_3d_obj, GLuint prog_3d_obj_instanced, GLuint prog_basic, GLuint prog_bbox,
	const WorldRandomConfig * world_random_config, string ch_directory) :
	_prog_3d_anim(prog_3d_anim), _prog_3d_terrain(prog_3d_terrain), _prog_3d_obj(prog_3d_obj), _prog_3d_obj_instanced(prog_3d_obj_instanced),
	_prog_basic(prog_basic), _prog_bbox(prog_bbox)
{
	_models_pool= new ModelsPool(_prog_3d_anim, _prog_3d_obj, _prog_basic);

	// random
	if (world_random_config!= NULL) {
		randomize(world_random_config);
	}
	// lecture dossier
	else if (ch_directory!= "") {
		//cout << "start read\n";
		read(ch_directory);
		//cout << "finished read\n";
	}
	else {
		cout << "Erreur constructeur World" << endl;
		return;
	}

	// calcul des groupes a partir des instances ; on regroupe dans un 1er temps les instances par modele
	// TODO : a terme se passer des instances ?
	map<string, vector<InstancePosRot *> > groups;
	for (auto si : _static_instances) {
		string key= si->_model->_ch_config_file;
		if (groups.find(key)== groups.end()) {
			groups[key]= vector<InstancePosRot *>();
		}
		groups[key].push_back(si->_pos_rot);
	}
	for (auto & group : groups) {
		_static_groups.push_back(new StaticGroup(group.first, _prog_3d_obj_instanced, group.second, STATIC_GROUP_DISTANCES));
	}

	// plus besoin des instances
	for (auto si : _static_instances) {
		delete si;
	}
	_static_instances.clear();

	// création quadtree et ajout des objets statiques ; le problème si on ajoutait les animés serait de les changer de noeud lorsqu'ils changent de 'région'
	_quad_tree= new QuadTree(_terrain->_config->_origin, _terrain->_config->_origin+ glm::vec2(_terrain->_config->_width, _terrain->_config->_height), QUAD_TREE_DEPTH);
	for (auto sg : _static_groups) {
		for (auto pr : sg->_pos_rots) {
			_quad_tree->add_position(pr);
		}
	}
	/*for (auto ai : _animated_instances) {
		_quad_tree->add_position(ai->_pos_rot);
	}*/
	_quad_tree->set_z_max(_terrain->_altis, _terrain->_config->_width_n, _terrain->_config->_height_n);

	// TODO
	/*_square_size_x= SQUARE_SIZE;
	_square_size_y= SQUARE_SIZE;
	_n_squares_x= (unsigned int)(WORLD_SIZE / _square_size_x);
	_n_squares_y= (unsigned int)(WORLD_SIZE / _square_size_y);
	for (unsigned int i=0; i<_n_squares_x; ++i) {
		for (unsigned int j=0; j<_n_squares_y; ++j) {
			glm::vec2 vmin= _terrain->_origin+ glm::vec2(i* _square_size_x, j* _square_size_y);
			glm::vec2 vmax= vmin+ glm::vec2(_square_size_x, _square_size_y);
			_squares.push_back(new Square(vmin, vmax));
		}
	}*/

	//sync_bbox_draws();
	//cout << "end init\n";
}


World::~World() {
	clear();
	delete _quad_tree;
	_models_pool->clear();
	delete _path_finder;
}


bool World::key_down(InputState * input_state, SDL_Keycode key) {
	// héros avance
	if (key== SDLK_UP) {
		get_hero()->_current_idx_anim= 1;
		return true;
	}
	// debug
	else if (key== SDLK_SPACE) {
		print();
		return true;
	}
	return false;
}


bool World::key_up(InputState * input_state, SDL_Keycode key) {
	// héros s'arrete
	if (key== SDLK_UP) {
		get_hero()->_current_idx_anim= 0;
		return true;
	}
	return false;
}


bool World::mouse_motion(InputState * input_state) {
	// A REVOIR ; un peu sale car interfère avec view_system ; permet de faire une rotation de la vue mais pour l'instant quelle que soit le mode de view_system !
	//get_hero()->set_pos_rot_scale(get_hero()->_pos_rot->_position, glm::rotate(get_hero()->_pos_rot->_rotation, (float)(-input_state->_xrel)* 0.01f, glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(1.0f));
	return true;
}


void World::draw() {
	_terrain->draw();

	for (auto sg : _static_groups) {
		sg->draw();
	}

	for (auto ai : _animated_instances) {
		ai->draw();
	}
}


void World::anim(ViewSystem * view_system, unsigned int tikanim_delta) {
	if (view_system->_new_single_selection) {
		//cout << "new_single_selection\n";
		view_system->_new_single_selection= false;
		for (auto ai : _animated_instances) {
			ai->_pos_rot->_selected= false;
			if (view_system->single_selection_intersects_aabb(ai->_pos_rot->_bbox->_aabb)) {
				//cout << "single_selection success\n";
				ai->_pos_rot->_selected= true;
			}
		}
	}
	else if (view_system->_new_rect_selection) {
		//cout << "new_rect_selection\n";
		view_system->_new_rect_selection= false;
		for (auto ai : _animated_instances) {
			ai->_pos_rot->_selected= false;
			if (view_system->rect_selection_intersects_bbox(ai->_pos_rot->_bbox)) {
				//cout << "rect_selection success\n";
				ai->_pos_rot->_selected= true;
			}
		}
	}

	_terrain->anim(view_system);

	// determination des objets statiques a dessiner
	for (auto sg : _static_groups) {
		for (auto pr : sg->_pos_rots) {
			pr->_active= false;
		}
	}
	_quad_tree->set_actives(view_system, STATIC_GROUP_DISTANCES[STATIC_GROUP_DISTANCES.size()- 1]);
	//cout << _quad_tree->_debug << endl;

	// anim des groupes d'objets statiques
	for (auto sg : _static_groups) {
		sg->anim(view_system);
	}

	// objets animés
	for (auto ai : _animated_instances) {
		ai->_pos_rot->_active= true;

		// tester différents ordres pour ces tests
		if (!view_system->intersects_bbox(ai->_pos_rot->_bbox)) {
			ai->_pos_rot->_active= false;
			continue;
		}
		
		ai->_pos_rot->update_dist2(view_system->_eye);
		if (ai->_pos_rot->_dist2> ANIMATED_DIST_VISIBLE* ANIMATED_DIST_VISIBLE) {
			ai->_pos_rot->_active= false;
			continue;
		}
		
		if (ai->_pos_rot->_dist2> ANIMATED_DIST_ANIMATED* ANIMATED_DIST_ANIMATED) {
			ai->set_animated(false);
		}
		else {
			ai->set_animated(true);
		}

		if (ai!= get_hero()) {
			if (rand_int(0, 100)== 0) {
				ai->randomize();
			}
		}

		ai->anim(view_system, tikanim_delta);

		float x= ai->_pos_rot->_position.x;
		float y= ai->_pos_rot->_position.y;
		if ((x< _terrain->_config->_origin.x) || (x> _terrain->_config->_origin.x+ _terrain->_config->_width) || 
			(y< _terrain->_config->_origin.y) || (y> _terrain->_config->_origin.y+ _terrain->_config->_height)) {
			x= rand_float(_terrain->_config->_origin.x, _terrain->_config->_origin.x+ _terrain->_config->_width);
			y= rand_float(_terrain->_config->_origin.y, _terrain->_config->_origin.y+ _terrain->_config->_height);
		}
		float z= _terrain->get_alti(glm::vec2(x, y));
		ai->set_pos_rot_scale(glm::vec3(x, y, z), ai->_pos_rot->_rotation, ai->_pos_rot->_scale);
	}

}


void World::randomize(const WorldRandomConfig * world_random_config) {
	clear(true, false);

	_terrain= new Terrain(_prog_3d_terrain, world_random_config->_terrain_config, & world_random_config->_terrain_random_config);
	
	float size_factor_xy, size_factor_z;

	// objets statiques
	for (auto & si_rand_cfg : world_random_config->_si_random_configs) {
		_models_pool->add_static_model(si_rand_cfg._path);
		for (unsigned int i=0; i<si_rand_cfg._compt; ++i) {
			size_factor_xy= rand_float(si_rand_cfg._size_factor_min_xy, si_rand_cfg._size_factor_max_xy);
			if (si_rand_cfg._conserve_ratio) {
				size_factor_z= size_factor_xy;
			}
			else {
				size_factor_z= rand_float(si_rand_cfg._size_factor_min_z, si_rand_cfg._size_factor_max_z);
			}
			StaticInstance * si= new StaticInstance(_models_pool->get_static_model(si_rand_cfg._path), glm::vec3(size_factor_xy, size_factor_xy, size_factor_z));
			glm::vec3 position= glm::vec3(rand_float(_terrain->_config->_origin.x, _terrain->_config->_origin.x+ _terrain->_config->_width), rand_float(_terrain->_config->_origin.y, _terrain->_config->_origin.y+ _terrain->_config->_height), 0.0f);
			position.z= _terrain->get_alti(glm::vec2(position.x, position.y));
			si->set_pos_rot_scale(position, glm::quat(glm::vec3(0.0f, 0.0f, rand_float(si_rand_cfg._rand_angle_min, si_rand_cfg._rand_angle_max))), si->_pos_rot->_scale);
		
			_static_instances.push_back(si);
		}
	}

	// objets animés
	for (auto & ai_rand_cfg : world_random_config->_ai_random_configs) {
		_models_pool->add_animated_model(ai_rand_cfg._path);
		for (unsigned int i=0; i<ai_rand_cfg._compt; ++i) {
			size_factor_xy= rand_float(ai_rand_cfg._size_factor_min_xy, ai_rand_cfg._size_factor_max_xy);
			if (ai_rand_cfg._conserve_ratio) {
				size_factor_z= size_factor_xy;
			}
			else {
				size_factor_z= rand_float(ai_rand_cfg._size_factor_min_z, ai_rand_cfg._size_factor_max_z);
			}
			AnimatedInstance * ai= new AnimatedInstance(_models_pool->get_animated_model(ai_rand_cfg._path), glm::vec3(size_factor_xy, size_factor_xy, size_factor_z), _prog_3d_anim, _prog_basic);
			glm::vec3 position= glm::vec3(rand_float(_terrain->_config->_origin.x, _terrain->_config->_origin.x+ _terrain->_config->_width), rand_float(_terrain->_config->_origin.y, _terrain->_config->_origin.y+ _terrain->_config->_height), 0.0f);
			position.z= _terrain->get_alti(glm::vec2(position.x, position.y));
			ai->set_pos_rot_scale(position, glm::quat(glm::vec3(0.0f, 0.0f, rand_float(ai_rand_cfg._rand_angle_min, ai_rand_cfg._rand_angle_max))), ai->_pos_rot->_scale);

			//memcpy(ai->_mesh->_ambient, glm::value_ptr(glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f))), sizeof(float)* 3);
			//ai->randomize();
		
			_animated_instances.push_back(ai);
		}
	}
}


// sauvegarde du monde dans un dossier
void World::write(string ch_directory) {
	string cmd;
	cmd= "rm -r "+ ch_directory+ " 2>/dev/null";
	system(cmd.c_str());
	cmd= "mkdir -p "+ ch_directory;
	system(cmd.c_str());

	string ch_world_file= ch_directory+ "/world.xml";
	ofstream xml_file(ch_world_file);
	xml_document<> doc;
	
	xml_node<> * decl= doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
	doc.append_node(decl);
	
	xml_node<> * root= doc.allocate_node(node_element, "world");
	doc.append_node(root);

	// ---------------------------------------------------------------------
	xml_node<> * terrain_node= doc.allocate_node(node_element, "terrain");
	
	xml_node<> * altis_node= doc.allocate_node(node_element, "altis");
	string ch_alti_data= ch_directory+ "/altis.tif";
	_terrain->save(ch_alti_data);
	altis_node->value(ch_alti_data.c_str());
	terrain_node->append_node(altis_node);

	xml_node<> * config_node= doc.allocate_node(node_element, "config");
	
	xml_node<> * origin_node= doc.allocate_node(node_element, "origin");
	xml_node<> * x_node= doc.allocate_node(node_element, "x");
	x_node->value(to_string(_terrain->_config->_origin.x).c_str());
	origin_node->append_node(x_node);
	xml_node<> * y_node= doc.allocate_node(node_element, "y");
	y_node->value(to_string(_terrain->_config->_origin.y).c_str());
	origin_node->append_node(y_node);
	config_node->append_node(origin_node);

	xml_node<> * width_sub_node= doc.allocate_node(node_element, "width_sub");
	width_sub_node->value(to_string(_terrain->_config->_width_sub).c_str());
	config_node->append_node(width_sub_node);
	xml_node<> * height_sub_node= doc.allocate_node(node_element, "height_sub");
	height_sub_node->value(to_string(_terrain->_config->_height_sub).c_str());
	config_node->append_node(height_sub_node);

	xml_node<> * width_step_node= doc.allocate_node(node_element, "width_step");
	width_step_node->value(to_string(_terrain->_config->_width_step).c_str());
	config_node->append_node(width_step_node);
	xml_node<> * height_step_node= doc.allocate_node(node_element, "height_step");
	height_step_node->value(to_string(_terrain->_config->_height_step).c_str());
	config_node->append_node(height_step_node);

	xml_node<> * n_subs_x_node= doc.allocate_node(node_element, "n_subs_x");
	n_subs_x_node->value(to_string(_terrain->_config->_n_subs_x).c_str());
	config_node->append_node(n_subs_x_node);
	xml_node<> * n_subs_y_node= doc.allocate_node(node_element, "n_subs_y");
	n_subs_y_node->value(to_string(_terrain->_config->_n_subs_y).c_str());
	config_node->append_node(n_subs_y_node);
	
	terrain_node->append_node(config_node);

	root->append_node(terrain_node);

	// ---------------------------------------------------------------------
	xml_node<> * static_models_node= doc.allocate_node(node_element, "static_models");
	for (auto sm : _models_pool->_static_models) {
		xml_node<> * static_model_node= doc.allocate_node(node_element, "static_model");
		xml_node<> * ch_config_node= doc.allocate_node(node_element, "ch_config");
		ch_config_node->value(sm->_ch_config_file.c_str());
		static_model_node->append_node(ch_config_node);
		static_models_node->append_node(static_model_node);
	}
	root->append_node(static_models_node);

	xml_node<> * animated_models_node= doc.allocate_node(node_element, "animated_models");
	for (auto am : _models_pool->_animated_models) {
		xml_node<> * animated_model_node= doc.allocate_node(node_element, "animated_model");
		xml_node<> * ch_config_node= doc.allocate_node(node_element, "ch_config");
		ch_config_node->value(am->_ch_config_file.c_str());
		animated_model_node->append_node(ch_config_node);
		animated_models_node->append_node(animated_model_node);
	}
	root->append_node(animated_models_node);

	// ---------------------------------------------------------------------
	xml_node<> * static_instances_node= doc.allocate_node(node_element, "static_instances");
	for (auto sg : _static_groups) {
		for (auto pr : sg->_pos_rots) {
			xml_node<> * static_instance_node= doc.allocate_node(node_element, "static_instance");
			xml_node<> * ch_config_node= doc.allocate_node(node_element, "ch_config");
			ch_config_node->value(sg->_ch_config_file.c_str());
			static_instance_node->append_node(ch_config_node);

			xml_node<> * position_node= doc.allocate_node(node_element, "position");
			char * position_string= doc.allocate_string(glm::to_string(pr->_position).c_str());
			position_node->value(position_string);
			static_instance_node->append_node(position_node);

			xml_node<> * rotation_node= doc.allocate_node(node_element, "rotation");
			char * rotation_string= doc.allocate_string(glm::to_string(pr->_rotation).c_str());
			rotation_node->value(rotation_string);
			static_instance_node->append_node(rotation_node);

			xml_node<> * scale_node= doc.allocate_node(node_element, "scale");
			char * scale_string= doc.allocate_string(glm::to_string(pr->_scale).c_str());
			scale_node->value(scale_string);
			static_instance_node->append_node(scale_node);

			static_instances_node->append_node(static_instance_node);
		}
	}
	
	root->append_node(static_instances_node);

	xml_node<> * animated_instances_node= doc.allocate_node(node_element, "animated_instances");
	for (auto ai : _animated_instances) {
		xml_node<> * animated_instance_node= doc.allocate_node(node_element, "animated_instance");
		xml_node<> * ch_config_node= doc.allocate_node(node_element, "ch_config");
		ch_config_node->value(ai->_model->_ch_config_file.c_str());
		animated_instance_node->append_node(ch_config_node);

		xml_node<> * position_node= doc.allocate_node(node_element, "position");
		char * position_string= doc.allocate_string(glm::to_string(ai->_pos_rot->_position).c_str());
		position_node->value(position_string);
		animated_instance_node->append_node(position_node);

		xml_node<> * scale_node= doc.allocate_node(node_element, "scale");
		char * scale_string= doc.allocate_string(glm::to_string(ai->_pos_rot->_scale).c_str());
		scale_node->value(scale_string);
		animated_instance_node->append_node(scale_node);

		animated_instances_node->append_node(animated_instance_node);
	}
	root->append_node(animated_instances_node);

	xml_file << doc;
	xml_file.close();
	doc.clear();
}


// lecture a partir d'un dossier
void World::read(std::string ch_directory) {
	clear(true, false);

	string ch_world_file= ch_directory+ "/world.xml";

	ifstream xml_file(ch_world_file);
	stringstream buffer;
	buffer << xml_file.rdbuf();
	xml_file.close();
	string xml_content(buffer.str());
	xml_document<> doc;
	doc.parse<0>(&xml_content[0]);
	xml_node<> * root_node= doc.first_node();

	// -----------------------------------------------------------------
	xml_node<> * terrain_node= root_node->first_node("terrain");
	xml_node<> * altis_node= terrain_node->first_node("altis");
	string ch_alti_data= altis_node->value();

	xml_node<> * config_node= terrain_node->first_node("config");

	xml_node<> * origin_node= config_node->first_node("origin");
	xml_node<> * x_node= origin_node->first_node("x");
	xml_node<> * y_node= origin_node->first_node("y");
	glm::vec2 origin= glm::vec2(stof(x_node->value()), stof(y_node->value()));

	xml_node<> * width_sub_node= config_node->first_node("width_sub");
	float width_sub= stof(width_sub_node->value());
	xml_node<> * height_sub_node= config_node->first_node("height_sub");
	float height_sub= stof(height_sub_node->value());

	xml_node<> * width_step_node= config_node->first_node("width_step");
	float width_step= stof(width_step_node->value());
	xml_node<> * height_step_node= config_node->first_node("height_step");
	float height_step= stof(height_step_node->value());

	xml_node<> * n_subs_x_node= config_node->first_node("n_subs_x");
	unsigned int n_subs_x= stoi(n_subs_x_node->value());
	xml_node<> * n_subs_y_node= config_node->first_node("n_subs_y");
	unsigned int n_subs_y= stoi(n_subs_y_node->value());

	const TerrainConfig * config= new TerrainConfig(origin, width_sub, height_sub, width_step, height_step, n_subs_x, n_subs_y);
	_terrain= new Terrain(_prog_3d_terrain, config, NULL, ch_alti_data);

	// -----------------------------------------------------------------
	xml_node<> * static_models_node= root_node->first_node("static_models");
	for (xml_node<> * static_model_node=static_models_node->first_node("static_model"); static_model_node; static_model_node=static_model_node->next_sibling()) {
		xml_node<> * ch_config_node= static_model_node->first_node("ch_config");
		string ch_config_file= ch_config_node->value();
		
		_models_pool->add_static_model(ch_config_file);
	}

	xml_node<> * animated_models_node= root_node->first_node("animated_models");
	for (xml_node<> * animated_model_node=animated_models_node->first_node("animated_model"); animated_model_node; animated_model_node=animated_model_node->next_sibling()) {
		xml_node<> * ch_config_node= animated_model_node->first_node("ch_config");
		string ch_config_file= ch_config_node->value();
		
		_models_pool->add_animated_model(ch_config_file);
	}

	// -----------------------------------------------------------------
 	xml_node<> * static_instances_node= root_node->first_node("static_instances");
	for (xml_node<> * static_instance_node=static_instances_node->first_node("static_instance"); static_instance_node; static_instance_node=static_instance_node->next_sibling()) {
		string ch_config_file= static_instance_node->first_node("ch_config")->value();
		glm::vec3 position;
		sscanf(static_instance_node->first_node("position")->value(), "vec3(%f, %f, %f)", &position.x, &position.y, &position.z);
		position.z= _terrain->get_alti(glm::vec2(position.x, position.y));
		glm::quat rotation;
		sscanf(static_instance_node->first_node("rotation")->value(), "quat(%f, {%f, %f, %f})", &rotation.w, &rotation.x, &rotation.y, &rotation.z);
		glm::vec3 scale;
		sscanf(static_instance_node->first_node("scale")->value(), "vec3(%f, %f, %f)", &scale.x, &scale.y, &scale.z);

		StaticInstance * si= new StaticInstance(_models_pool->get_static_model(ch_config_file), scale);
		si->set_pos_rot_scale(position, rotation, scale);
		_static_instances.push_back(si);
	}

	xml_node<> * animated_instances_node= root_node->first_node("animated_instances");
	for (xml_node<> * animated_instance_node=animated_instances_node->first_node("animated_instance"); animated_instance_node; animated_instance_node=animated_instance_node->next_sibling()) {
		string ch_config= animated_instance_node->first_node("ch_config")->value();
		glm::vec3 position;
		sscanf(animated_instance_node->first_node("position")->value(), "vec3(%f, %f, %f)", &position.x, &position.y, &position.z);
		glm::vec3 scale;
		sscanf(animated_instance_node->first_node("scale")->value(), "vec3(%f, %f, %f)", &scale.x, &scale.y, &scale.z);

		AnimatedInstance * ai= new AnimatedInstance(_models_pool->get_animated_model(ch_config), scale, _prog_3d_anim, _prog_basic);
		ai->set_pos_rot_scale(position, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), scale);
		_animated_instances.push_back(ai);
	}

	// -----------------------------------------------------------------
	xml_node<> * path_finder_node= root_node->first_node("path_finder");
	xml_node<> * n_ligs_node= path_finder_node->first_node("n_ligs");
	unsigned int n_ligs= stoi(n_ligs_node->value());
	xml_node<> * n_cols_node= path_finder_node->first_node("n_cols");
	unsigned int n_cols= stoi(n_cols_node->value());
	xml_node<> * obstacles_node= path_finder_node->first_node("obstacles");
	string shp_path= obstacles_node->value();
	
	_path_finder= new PathFinder(n_ligs, n_cols, _terrain->_config->_origin, glm::vec2(_terrain->_config->_width, _terrain->_config->_height), true);
	_path_finder->read_shapefile(shp_path, glm::vec2(0.0f ,0.0f), glm::vec2(_terrain->_config->_width+ 1.0f, _terrain->_config->_height+ 1.0f), true);
	statics2obstacles();
	_path_finder->update_grid();
}


void World::statics2obstacles() {
	for (auto si : _static_instances) {
		Polygon2D * poly= new Polygon2D();
		/*glm::vec2 origin= glm::vec2(si->_pos_rot->_bbox->_vmin.x, si->_pos_rot->_bbox->_vmin.y);
		glm::vec2 size= glm::vec2(si->_pos_rot->_bbox->_vmax.x, si->_pos_rot->_bbox->_vmax.y)- origin;
		poly->set_rectangle(origin, size);*/
		float pts[16];
		for (unsigned int i=0; i<8; ++i) {
			pts[2* i+ 0]= si->_pos_rot->_bbox->_pts[i].x;
			pts[2* i+ 1]= si->_pos_rot->_bbox->_pts[i].y;
		}
		poly->set_points(pts, 8, true);
		_path_finder->_polygons.push_back(poly);
	}
}


void World::clear(bool clear_instances, bool clear_terrain) {
	if (clear_instances) {
		for (auto ai : _animated_instances) {
			delete ai;
		}
		_animated_instances.clear();

		for (auto si : _static_instances) {
			delete si;
		}
		_static_instances.clear();

		for (auto sg : _static_groups) {
			delete sg;
		}
		_static_groups.clear();
	}

	if (clear_terrain) {
		/*for (auto square : _squares) {
			delete square;
		}*/

		delete _terrain;
	}
}


// renvoie le centre du monde
glm::vec2 World::get_center() {
	return _terrain->_config->_origin+ glm::vec2(_terrain->_config->_width* 0.5f, _terrain->_config->_height* 0.5f);
}


// affichage pour debug
void World::print() {
	int n_ai_actives= 0;
	for (auto ai : _animated_instances) {
		if (ai->_pos_rot->_active) {
			n_ai_actives++;
		}
	}
	cout << "ai_actives=" << n_ai_actives << " / " << _animated_instances.size() << endl;

	unsigned int n_si_actives= 0;
	unsigned int n_total= 0;
	for (auto sg : _static_groups) {
		for (auto pr : sg->_pos_rots) {
			n_total++;
			if (pr->_active) {
				n_si_actives++;
			}
		}
	}
	cout << "si_actives=" << n_si_actives << " / " << n_total << endl;

	_quad_tree->print();
}

// TODO
/*
void World::update_squares() {
	for (auto square : _squares) {
		square->_state= WALKABLE;
		for (auto si : _static_instances) {
			
		}
	}
}
*/


// renvoie un pointeur vers le héros
AnimatedInstance * World::get_hero() {
	return _animated_instances[0];
}


// synchro des bboxs ; Tout ce truc est très lent ; a revoir
/*void World::sync_bbox_draws() {
	for (auto bbd : _static_bbox_draws) {
		delete bbd;
	}
	_static_bbox_draws.clear();

	for (auto bbd : _animated_bbox_draws) {
		delete bbd;
	}
	_animated_bbox_draws.clear();

	for (auto sg : _static_groups) {
		for (auto pr : sg->_pos_rots) {
			BBoxDraw * bbd= new BBoxDraw(_prog_bbox, pr->_bbox, STATIC_BBOX_COLOR);
			_static_bbox_draws.push_back(bbd);
		}
	}

	for (unsigned int idx=0; idx<_terrain->_config->_n_subs; ++idx) {
		BBoxDraw * bbd= new BBoxDraw(_prog_bbox, _terrain->_subterrains[idx]->_aabb, TERRAIN_BBOX_COLOR);
		_static_bbox_draws.push_back(bbd);
	}

	for (auto ai : _animated_instances) {
		BBoxDraw * bbd= new BBoxDraw(_prog_bbox, ai->_pos_rot->_bbox, ANIMATED_BBOX_COLOR);
		_animated_bbox_draws.push_back(bbd);
	}
}
*/
