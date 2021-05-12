
#include "daefile.h"

using namespace std;
using namespace rapidxml;


// --------------------------------------------------------------------------------------------
bool cmp_joint_influence(const JointInfluence & a, const JointInfluence & b) {
	return a._weight > b._weight;
}


// --------------------------------------------------------------------------------------------
ModelJoint::ModelJoint() {
	
}


ModelJoint::~ModelJoint() {
	
}


void ModelJoint::print() {
	cout << "id=" << _id << " ; sid=" << _sid << " ; parent_sid=" << _parent->_sid << endl;
	cout << "_local_bind_mat=" << glm::to_string(_local_bind_mat) << endl;
	cout << "bind_mat=" << glm::to_string(_bind_mat) << endl;
	cout << "inv_bind_mat=" << glm::to_string(_inv_bind_mat) << endl;
	cout << "anims.size=" << _anims.size() << endl;
	for (auto & it_anim : _anims) {
		cout << it_anim._time << "  : " << glm::to_string(it_anim._mat) << endl;
	}
	cout << "--------------------------------------" << endl;
}


// --------------------------------------------------------------------------------------------
InstanceJoint::InstanceJoint() {

}


InstanceJoint::InstanceJoint(ModelJoint * model_joint) : _model_joint(model_joint), _anim_time(0.0f)  {

}


InstanceJoint::~InstanceJoint() {

}


void InstanceJoint::anim(unsigned int delta_time_ms) {
	if (_model_joint->_anims.size()== 0) {
		return;
	}
	/*_local_current_mat= _model_joint->_anims[0]._mat;
	return;*/
	
	// _anims[i]._time issu de export Blender en secondes
	_anim_time+= (float)(delta_time_ms)* 0.001f;
	if (_anim_time< _model_joint->_anims[0]._time) {
		_anim_time= _model_joint->_anims[0]._time;
	}
	while (_anim_time> _model_joint->_anims[_model_joint->_anims.size()- 1]._time) {
		_anim_time= _model_joint->_anims[0]._time+ _anim_time- _model_joint->_anims[_model_joint->_anims.size()- 1]._time;
	}

	int idx_anim_1= -1;
	int idx_anim_2= -1;
	float delta= 0.0f;
	for (unsigned int idx_anim=0; idx_anim<_model_joint->_anims.size(); ++idx_anim) {
		if (_model_joint->_anims[idx_anim]._time> _anim_time) {
			idx_anim_2= idx_anim;
			idx_anim_1= idx_anim- 1;
			delta= (_anim_time- _model_joint->_anims[idx_anim_1]._time)/ (_model_joint->_anims[idx_anim_2]._time- _model_joint->_anims[idx_anim_1]._time);
			break;
		}
	}
	glm::quat rotation_1, rotation_2, rotation_interpoled;
	glm::vec3 translation_1, translation_2, translation_interpoled;
	rotation_1= glm::quat_cast(_model_joint->_anims[idx_anim_1]._mat);
	rotation_2= glm::quat_cast(_model_joint->_anims[idx_anim_2]._mat);
	translation_1= glm::vec3(_model_joint->_anims[idx_anim_1]._mat[3]);
	translation_2= glm::vec3(_model_joint->_anims[idx_anim_2]._mat[3]);
	
	rotation_interpoled= glm::slerp(rotation_1, rotation_2, delta);
	glm::mat4 rotation_mat= mat4_cast(rotation_interpoled);
	
	translation_interpoled= glm::mix(translation_1, translation_2, delta);
	glm::mat4 translation_mat= glm::translate(glm::mat4(1.0f), translation_interpoled);

	_local_current_mat= translation_mat* rotation_mat;
}


void InstanceJoint::print() {
	cout << "model_joint ---" << endl;
	_model_joint->print();
	cout << "_local_current_mat=" << glm::to_string(_local_current_mat) << endl;
	cout << "_current_mat=" << glm::to_string(_current_mat) << endl;
	cout << "skinning_mat=" << glm::to_string(_skinning_mat) << endl;
}


// --------------------------------------------------------------------------------------------
ModelAnimation::ModelAnimation() {

}


ModelAnimation::ModelAnimation(string ch_dae_file, float moving_speed) : _id(""), _moving_speed(moving_speed) {
	parse_dae(ch_dae_file);

	// on applique les _bind_shape_matrix
	for (auto & it_geom : _geometries) {
		for (auto & it_vertex : it_geom._vertices) {
			it_vertex._position= glm::vec3(it_geom._bind_shape_matrix* glm::vec4(it_vertex._position, 1.0f));
		}
	}
	
	// tri des joints des vertex par poids, de sorte que si vertex._joints.size() > 4, les poids importants sont pris en compte
	for (auto & it_geom : _geometries) {
		for (auto & it_vertex : it_geom._vertices) {
			sort(it_vertex._joints.begin(), it_vertex._joints.end(), cmp_joint_influence);
		}
	}

	compute_mats();
}


ModelAnimation::~ModelAnimation() {
	for (auto it_joint : _joints) {
		delete it_joint;
	}
	_joints.clear();
}


// parcours récursif des joints
void ModelAnimation::parse_joint(xml_node<> * node, ModelJoint * parent_joint) {
	string str_matrix= node->first_node("matrix")->value();
	istringstream iss_matrix(str_matrix);
	vector<float> split_matrix((istream_iterator<float>(iss_matrix)), istream_iterator<float>());
	float mat[16];
	for (unsigned int idx=0; idx<16; ++idx) {
		mat[idx]= split_matrix[idx];
	}
	xml_attribute<> * id_node= node->first_attribute("id");
	xml_attribute<> * sid_node= node->first_attribute("sid");
	
	ModelJoint * j= new ModelJoint();
	j->_local_bind_mat= glm::make_mat4(mat);
	j->_parent= parent_joint;
	j->_id= id_node->value();
	
	// le joint root n'a pas d'attribut sid
	if (sid_node!= 0) {
		j->_sid= sid_node->value();
	}
	else {
		j->_sid= ROOT_JOINT_SID;
	}

	// ajout a joints
	_joints.push_back(j);
	
	// parcours des node enfants
	for (xml_node<> * subnode=node->first_node("node"); subnode; subnode=subnode->next_sibling()) {
		// il y a parfois des subnode de type NODE ; faut-il les prendre en compte ?
		if ((string(subnode->name())!= "node") || (string(subnode->first_attribute("type")->value())!= "JOINT")) {
			continue;
		}

		parse_joint(subnode, j);
	}
}

/*
void ModelAnimation::compute_mat_joint(unsigned int idx_joint) {
	if (_joints[idx_joint]->_sid== ROOT_JOINT_SID) {
		_joints[idx_joint]->_bind_mat= _joints[idx_joint]->_local_bind_mat;
	}
	else {
		_joints[idx_joint]->_bind_mat= _joints[idx_joint]->_local_bind_mat* _joints[idx_joint]->_parent->_bind_mat;
	}
	for (unsigned int idx_joint_2=0; idx_joint_2<_joints.size(); ++idx_joint_2) {
		if (_joints[idx_joint_2]->_parent->_sid== _joints[idx_joint]->_sid) {
			compute_mat_joint(idx_joint_2);
		}
	}
}


void ModelAnimation::compute_mats() {
	unsigned int idx_joint_root= 0;
	for (unsigned int idx_joint=0; idx_joint<_joints.size(); ++idx_joint) {
		if (_joints[idx_joint]->_sid== ROOT_JOINT_SID) {
			idx_joint_root= idx_joint;
			break;
		}
	}
	compute_mat_joint(idx_joint_root);
}
*/

// suppose que les joints sont triés de sorte qu'un enfant vient tjrs apres son parent.....
// + rapide que du récursif
void ModelAnimation::compute_mats() {
	for (auto it_joint : _joints) {
		if (it_joint->_sid== ROOT_JOINT_SID) {
			it_joint->_bind_mat= it_joint->_local_bind_mat;
		}
		else {
			it_joint->_bind_mat= it_joint->_local_bind_mat* it_joint->_parent->_bind_mat;
		}
	}
}


void ModelAnimation::parse_dae(string ch_dae_file) {
	// création de l'arbre XML
	ifstream dae_file(ch_dae_file);
	stringstream buffer;
	buffer << dae_file.rdbuf();
	dae_file.close();
	string dae_content(buffer.str());

	xml_document<> doc;
	doc.parse<0>(&dae_content[0]);
	
	xml_node<> * root_node= doc.first_node();

	// parcours de library_geometries -----------------------------------------------------------------------------------------
	xml_node<> * libgeom_node= root_node->first_node("library_geometries");
	for (xml_node<> * geom_node=libgeom_node->first_node("geometry"); geom_node; geom_node=geom_node->next_sibling()) {
		Geometry geometry;
		
		// id geometry servira ensuite lors du lien vertex-(joint/weight) ; un controller par géométrie
		geometry._id= geom_node->first_attribute("id")->value();

		xml_node<> * mesh_node= geom_node->first_node("mesh");
		
		xml_node<> * triangles_node= mesh_node->first_node("triangles");
		// nombre de triangles
		int count_triangles= stoi(triangles_node->first_attribute("count")->value());

		// récup des noms et offsets des positions, normals, colors, textures
		string position_tag, normal_tag, color_tag, texture_tag;
		unsigned int position_offset, normal_offset, color_offset, texture_offset;
		for (xml_node<> * triangle_input_node=triangles_node->first_node("input"); triangle_input_node; triangle_input_node=triangle_input_node->next_sibling()) {
			if (string(triangle_input_node->name())!= "input") {
				continue;
			}

			string semantic= triangle_input_node->first_attribute("semantic")->value();
			string source= triangle_input_node->first_attribute("source")->value();
			unsigned int offset= stoi(triangle_input_node->first_attribute("offset")->value());
			
			if (semantic== "VERTEX") {
				xml_node<> * vertices_node= mesh_node->first_node("vertices");
				xml_node<> * vertices_input_node= vertices_node->first_node("input");
				position_tag= vertices_input_node->first_attribute("source")->value();
				position_tag= position_tag.substr(1, string::npos); // on enleve le #;
				position_offset= offset;
			}
			else if (semantic== "NORMAL") {
				normal_tag= source.substr(1, string::npos); // on enleve le #
				normal_offset= offset;
			}
			else if (semantic== "COLOR") {
				color_tag= source.substr(1, string::npos); // on enleve le #
				color_offset= offset;
			}
			else if (semantic== "TEXCOORD") {
				texture_tag= source.substr(1, string::npos); // on enleve le #
				texture_offset= offset;
			}
		}

		// nombre d'attributs récupérés ; au min 2 : position et normal
		unsigned int n_attrs= 2;
		if (!color_tag.empty()) {
			n_attrs++;
		}
		if (!texture_tag.empty()) {
			n_attrs++;
		}

		// indices des vertices
		string str_vertices_idx= triangles_node->first_node("p")->value();
		istringstream iss_vertices_idx(str_vertices_idx);
		vector<unsigned int> split_vertices_idx((istream_iterator<unsigned int>(iss_vertices_idx)), istream_iterator<unsigned int>());
		unsigned int vertices_idx[count_triangles* 3* n_attrs];
 		for (unsigned int idx=0; idx<split_vertices_idx.size(); ++idx) {
			vertices_idx[idx]= split_vertices_idx[idx];
		}

		// nombre et coordonnées des positions, normale, colors, textures
		string str_positions;
		string str_normals;
		string str_colors;
		string str_textures;
		unsigned int n_positions= 0;
		unsigned int n_normals= 0;
		unsigned int n_colors= 0;
		unsigned int n_textures= 0;
		for (xml_node<> * source_node=mesh_node->first_node("source"); source_node; source_node=source_node->next_sibling()) {
			if (string(source_node->name())!= "source") {
				continue;
			}

			if (source_node->first_attribute("id")->value()== position_tag) {
				str_positions= source_node->first_node("float_array")->value();
				n_positions= stoi(source_node->first_node("float_array")->first_attribute("count")->value());
			}
			else if (source_node->first_attribute("id")->value()== normal_tag) {
				str_normals= source_node->first_node("float_array")->value();
				n_normals= stoi(source_node->first_node("float_array")->first_attribute("count")->value());
			}
			else if (source_node->first_attribute("id")->value()== color_tag) {
				str_colors= source_node->first_node("float_array")->value();
				n_colors= stoi(source_node->first_node("float_array")->first_attribute("count")->value());
			}
			else if (source_node->first_attribute("id")->value()== texture_tag) {
				str_textures= source_node->first_node("float_array")->value();
				n_textures= stoi(source_node->first_node("float_array")->first_attribute("count")->value());
			}
		}

		//cout << n_positions << " ; " << n_normals << " ; " << n_colors << " ; " << n_textures << endl;
		
		// TODO : les nombres sont reliés par des relations logiques ; faire des assert pour robustifier
		//assert(n_normals==);

		istringstream iss_positions(str_positions);
		vector<float> positions((istream_iterator<float>(iss_positions)), istream_iterator<float>());

		istringstream iss_normals(str_normals);
		vector<float> normals((istream_iterator<float>(iss_normals)), istream_iterator<float>());

		istringstream iss_colors(str_colors);
		vector<float> colors((istream_iterator<float>(iss_colors)), istream_iterator<float>());

		istringstream iss_textures(str_textures);
		vector<float> textures((istream_iterator<float>(iss_textures)), istream_iterator<float>());

		// remplissage de vertices
		for (unsigned int i=0; i<count_triangles* 3; ++i) {
			Vertex v;

			v._idx= vertices_idx[n_attrs* i+ position_offset];

			v._position.x= positions[3* vertices_idx[n_attrs* i+ position_offset]+ 0];
			v._position.y= positions[3* vertices_idx[n_attrs* i+ position_offset]+ 1];
			v._position.z= positions[3* vertices_idx[n_attrs* i+ position_offset]+ 2];
			v._normal.x= normals[3* vertices_idx[n_attrs* i+ normal_offset]+ 0];
			v._normal.y= normals[3* vertices_idx[n_attrs* i+ normal_offset]+ 1];
			v._normal.z= normals[3* vertices_idx[n_attrs* i+ normal_offset]+ 2];

			if (color_tag.empty()) {
				v._color.x= 0.0f;
				v._color.y= 0.0f;
				v._color.z= 0.0f;
			}
			else {
				v._color.x= colors[3* vertices_idx[n_attrs* i+ color_offset]+ 0];
				v._color.y= colors[3* vertices_idx[n_attrs* i+ color_offset]+ 1];
				v._color.z= colors[3* vertices_idx[n_attrs* i+ color_offset]+ 2];
			}

			if (texture_tag.empty()) {
				v._texture.x= 0.0f;
				v._texture.y= 0.0f;
			}
			else {
				v._texture.x= textures[3* vertices_idx[n_attrs* i+ texture_offset]+ 0];
				v._texture.y= textures[3* vertices_idx[n_attrs* i+ texture_offset]+ 1];
			}
			
			geometry._vertices.push_back(v);
		}

		_geometries.push_back(geometry);
	}

	// parcours library_visual_scenes (joints) ------------------------------------------------------------------------------------------------------------
	xml_node<> * libscene_node= root_node->first_node("library_visual_scenes");
	xml_node<> * scene_node= libscene_node->first_node("visual_scene");
	// on cherche le node "node" qui a au - 1 enfant "node"
	xml_node<> * joints_root_node= 0;
	for (xml_node<> * node=scene_node->first_node("node"); node; node=node->next_sibling()) {
		xml_node<> * sub_node= node->first_node("node");
		if (sub_node!= 0) {
			joints_root_node= node;
			break;
		}
	}
	if (joints_root_node== 0) {
		cout << "library_visual_scenes est vide\n";
		return;
	}

	// parcours récursif de l'arbre des joints ; le parent de root est un joint bidon
	ModelJoint null_joint;
	null_joint._sid= NULL_JOINT_SID;
	parse_joint(joints_root_node, &null_joint);

	// parcours library_controllers (liens entre vertices et joints) --------------------------------------------------------------------------------------
	xml_node<> * libcontroller_node= root_node->first_node("library_controllers");

	// un controller par géométrie
	for (xml_node<> * controller_node=libcontroller_node->first_node("controller"); controller_node; controller_node=controller_node->next_sibling()) {
	
		xml_node<> * skin_node= controller_node->first_node("skin");
		// récup de la géométrie concernée
		string geom_id= skin_node->first_attribute("source")->value();
		geom_id= geom_id.substr(1, string::npos); // on enleve le #
		bool found= false;
		unsigned int idx_geom_ok= 0;
		for (unsigned int idx_geom=0; idx_geom<_geometries.size(); ++idx_geom) {
			if (_geometries[idx_geom]._id== geom_id) {
				found= true;
				idx_geom_ok= idx_geom;
				break;
			}
		}
		if (!found) {
			cout << "controller : geom not found : " << geom_id << endl;
			continue;
		}

		// matrice a appliquer au vertices de la geom concernée en 1er lieu
		string str_bsm= skin_node->first_node("bind_shape_matrix")->value();
		istringstream iss_bsm(str_bsm);
		vector<float> split_bsm((istream_iterator<float>(iss_bsm)), istream_iterator<float>());
		float bsm_mat[16];
			for (unsigned int idx=0; idx<16; ++idx) {
			bsm_mat[idx]= split_bsm[idx];
		}
		// il faut transposer toutes les matrices !!!
		_geometries[idx_geom_ok]._bind_shape_matrix= glm::transpose(glm::make_mat4(bsm_mat));

		// tag et offset joint / weight
		string joint_tag, weight_tag;
		unsigned int joint_offset, weight_offset;

		xml_node<> * vertex_weights_node= skin_node->first_node("vertex_weights");
		for (xml_node<> * vw_input_node=vertex_weights_node->first_node("input"); vw_input_node; vw_input_node=vw_input_node->next_sibling()) {
			if (string(vw_input_node->name())!= "input") {
				continue;
			}

			string semantic= vw_input_node->first_attribute("semantic")->value();
			string source= vw_input_node->first_attribute("source")->value();
			unsigned int offset= stoi(vw_input_node->first_attribute("offset")->value());
			
			if (semantic== "JOINT") {
				joint_tag= source.substr(1, string::npos); // on enleve le #
				joint_offset= offset;
			}
			else if (semantic== "WEIGHT") {
				weight_tag= source.substr(1, string::npos); // on enleve le #
				weight_offset= offset;
			}
		}

		// tag inv_bind_matrix
		string inv_bind_matrix_tag;

		xml_node<> * joints_node= skin_node->first_node("joints");
		for (xml_node<> * joints_input_node=joints_node->first_node("input"); joints_input_node; joints_input_node=joints_input_node->next_sibling()) {
			string semantic= joints_input_node->first_attribute("semantic")->value();
			string source= joints_input_node->first_attribute("source")->value();
			if (semantic== "INV_BIND_MATRIX") {
				inv_bind_matrix_tag= source.substr(1, string::npos); // on enleve le #
			}
		}

		// nombre et sids des joints ; nombre et valeur des weights ; inv_bind_matrices : a appliquer aux vertices pour les ramener dans le repere du joint
		unsigned int n_joints;
		vector<string> joints_sids;
		unsigned int n_weights;
		vector<float> weights;
		vector<glm::mat4> inv_bind_matrices;
		
		for (xml_node<> * source_node=skin_node->first_node("source"); source_node; source_node=source_node->next_sibling()) {
			if (string(source_node->name())!= "source") {
				continue;
			}

			if (source_node->first_attribute("id")->value()== joint_tag) {
				n_joints= stoi(source_node->first_node("Name_array")->first_attribute("count")->value());
				string str_joints_sids= source_node->first_node("Name_array")->value();
				istringstream iss_joints_sids(str_joints_sids);
				copy(istream_iterator<string>(iss_joints_sids), istream_iterator<string>(), back_inserter(joints_sids));
			}

			else if (source_node->first_attribute("id")->value()== weight_tag) {
				n_weights= stoi(source_node->first_node("float_array")->first_attribute("count")->value());
				string str_weights= source_node->first_node("float_array")->value();
				istringstream iss_weights(str_weights);
				copy(istream_iterator<float>(iss_weights), istream_iterator<float>(), back_inserter(weights));
			}

			else if (source_node->first_attribute("id")->value()== inv_bind_matrix_tag) {
				string str_ibm= source_node->first_node("float_array")->value();
				istringstream iss_ibm(str_ibm);
				vector<float> split_ibm((istream_iterator<float>(iss_ibm)), istream_iterator<float>());
				for (unsigned int i=0; i<n_joints; ++i) {
					float ibm[16];
					for (unsigned int j=0; j<16; ++j) {
						ibm[j]= split_ibm[i* 16+ j];
					}
					// il faut transposer toutes les matrices !!!
					inv_bind_matrices.push_back(glm::transpose(glm::make_mat4(ibm)));
				}
			}
		}

		// vcount : 1 valeur par vertex : nombre de couples d'indices de v a considérer pour ce vertex
		// v : liste de couples (index joint , index weight) (ordre dépend de joint_offset, weight_offset) qui vont influer les vertices
		string str_vcount= vertex_weights_node->first_node("vcount")->value();
		istringstream iss_vcount(str_vcount);
		vector<unsigned int> vcount;
		copy(istream_iterator<unsigned int>(iss_vcount), istream_iterator<unsigned int>(), back_inserter(vcount));

		string str_v= vertex_weights_node->first_node("v")->value();
		istringstream iss_v(str_v);
		vector<unsigned int> v;
		copy(istream_iterator<unsigned int>(iss_v), istream_iterator<unsigned int>(), back_inserter(v));

		// remplissage de vertex._joints
		unsigned int current_v_idx= 0;
		for (unsigned int idx_vertex=0; idx_vertex<vcount.size(); ++idx_vertex) {
			for (unsigned int offset=0; offset<vcount[idx_vertex]; ++offset) {
				// on récupère le SID du joint pour retrouver l'indice du joint dans joints
				string joint_sid= joints_sids[v[2* (current_v_idx+ offset)+ joint_offset]];
				float weight= weights[v[2* (current_v_idx+ offset)+ weight_offset]];
				unsigned int idx_joint_ok= 0;
				bool found= false;
				for (unsigned int idx_joint=0; idx_joint<_joints.size(); ++idx_joint) {
					if (_joints[idx_joint]->_sid== joint_sid) {
						idx_joint_ok= idx_joint;
						found= true;
						break;
					}
				}
				if (!found) {
					cout << "controller : join SID not found (1) : " << joint_sid << endl;
					continue;
				}
				JointInfluence ji{idx_joint_ok, weight};
				// attention ici idx_vertex n'est pas un indice dans _vertices mais dans le tableau temporaire position plus haut
				// d'ou la nécessité du champ _idx de Vertex
				for (auto & it_vertex : _geometries[idx_geom_ok]._vertices) {
					if (it_vertex._idx== idx_vertex) {
						it_vertex._joints.push_back(ji);
					}
				}
			}
			current_v_idx+= vcount[idx_vertex];
		}

		// remplissage de joint._inv_bind_matrix
		// attention n_joints peut etre différent (inférieur à priori) de joints.size()
		for (auto & it_joint : _joints) {
			it_joint->_inv_bind_mat= glm::mat4(1.0f);
		}

		for (unsigned int idx_joint=0; idx_joint<n_joints; ++idx_joint) {
			unsigned int idx_joint_ok= 0;
			bool found= false;
			for (unsigned int idx_joint_2=0; idx_joint_2<_joints.size(); ++idx_joint_2) {
				if (_joints[idx_joint_2]->_sid== joints_sids[idx_joint]) {
					idx_joint_ok= idx_joint_2;
					found= true;
					break;
				}
			}
			if (!found) {
				cout << "controller : join SID not found (2) : " << joints_sids[idx_joint] << endl;
				continue;
			}
			_joints[idx_joint_ok]->_inv_bind_mat= inv_bind_matrices[idx_joint];
		}
	
	}

	// animations ------------------------------------------------------------------------------------------------------------------------
	xml_node<> * libanimation_node= root_node->first_node("library_animations");
	if (libanimation_node== 0) {
		cout << "library_animations n'existe pas\n";
		return;
	}
	
	for (xml_node<> * animation_node=libanimation_node->first_node("animation"); animation_node; animation_node=animation_node->next_sibling()) {
		xml_node<> * channel_node= animation_node->first_node("channel");
		// id joint a animer
		string joint_target_id;
		istringstream iss_joint_target_id(channel_node->first_attribute("target")->value());
		getline(iss_joint_target_id, joint_target_id, '/');
		
		// tags input, output
		string input_tag, output_tag;
		xml_node<> * sampler_node= animation_node->first_node("sampler");
		for (xml_node<> * smp_input_node=sampler_node->first_node("input"); smp_input_node; smp_input_node=smp_input_node->next_sibling()) {
			string semantic= smp_input_node->first_attribute("semantic")->value();
			string source= smp_input_node->first_attribute("source")->value();

			if (semantic== "INPUT") {
				input_tag= source.substr(1, string::npos); // on enleve le #
			}
			else if (semantic== "OUTPUT") {
				output_tag= source.substr(1, string::npos); // on enleve le #
			}
		}

		// nombre d'indices temporels et temps ; a chaque temps correspond une matrice d'animation
		unsigned int n_times= 0;
		vector<float> times;
		vector<glm::mat4> anims_mats;
		for (xml_node<> * anim_source_node=animation_node->first_node("source"); anim_source_node; anim_source_node=anim_source_node->next_sibling()) {
			if (string(anim_source_node->name())!= "source") {
				continue;
			}

			if (anim_source_node->first_attribute("id")->value()== input_tag) {
				n_times= stoi(anim_source_node->first_node("float_array")->first_attribute("count")->value());
				string str_time= anim_source_node->first_node("float_array")->value();
				istringstream iss_time(str_time);
				copy(istream_iterator<float>(iss_time), istream_iterator<float>(), back_inserter(times));
			}
			else if (anim_source_node->first_attribute("id")->value()== output_tag) {
				string str_mat= anim_source_node->first_node("float_array")->value();
				istringstream iss_mat(str_mat);
				vector<float> mats;
				copy(istream_iterator<float>(iss_mat), istream_iterator<float>(), back_inserter(mats));
				for (unsigned int idx_time=0; idx_time<n_times; ++idx_time) {
					float mat[16];
					for (unsigned int i=0; i<16; ++i) {
						mat[i]= mats[idx_time* 16+ i];
					}
					// il faut transposer toutes les matrices !!!
					anims_mats.push_back(glm::transpose(glm::make_mat4(mat)));
				}
			}
		}

		// renseignement de joint->_anims
		unsigned int idx_joint_ok= 0;
		bool found= false;
		for (unsigned int idx_joint=0; idx_joint<_joints.size(); ++idx_joint) {
			// contrairement a auparavant le lien ici se fait avec l'id, PAS le sid !
			if (_joints[idx_joint]->_id== joint_target_id) {
				idx_joint_ok= idx_joint;
				found= true;
				break;
			}
		}
		if (!found) {
			cout << "anim :  join not found : " << joint_target_id << endl;
			continue;
		}
		
		for (unsigned int idx_time=0; idx_time<n_times; ++idx_time) {
			TimedMatrix tm{times[idx_time], anims_mats[idx_time]};
			_joints[idx_joint_ok]->_anims.push_back(tm);
		}
	}
}


void ModelAnimation::print() {
	cout << "id=" << _id << endl;
	/*cout << "geometries ========" << endl;
	for (auto & it_geom  : _geometries) {
		cout << "geom.id=" << it_geom._id << " ; n_vertices=" << it_geom._vertices.size() << endl;
		for (auto & it_vertex : it_geom._vertices) {
			cout << "n_joints=" << it_vertex._joints.size() << " ; ";
			for (auto & it_ji : it_vertex._joints) {
				cout << "(" << it_ji._idx_joint << "," << it_ji._weight << ") ";
			}
			cout << endl;
		}
		cout << "----------------" << endl;
	}*/
	
	cout << "n_joints=" << _joints.size() << endl;
	cout << "joints =======" << endl;
	for (auto it_joint : _joints) {
		it_joint->print();
	}
}


void ModelAnimation::clean() {
	_geometries.clear();
}


// --------------------------------------------------------------------------------------------
InstanceAnimation::InstanceAnimation() {

}


InstanceAnimation::InstanceAnimation(ModelAnimation * model_animation) : _model_animation(model_animation) {
	for (auto it_joint : _model_animation->_joints) {
		InstanceJoint * ij= new InstanceJoint(it_joint);
		_joints.push_back(ij);
	}

	for (auto it_joint : _joints) {
		bool ok= false;
		for (auto it_joint_2 : _joints) {
			if (it_joint->_model_joint->_parent->_sid== it_joint_2->_model_joint->_sid) {
				it_joint->_parent= it_joint_2;
				ok= true;
				break;
			}
		}
	}

	// on init avec bind
	for (auto it_joint : _joints) {
		it_joint->_local_current_mat= it_joint->_model_joint->_local_bind_mat;
	}

	compute_mats();
}


InstanceAnimation::~InstanceAnimation() {
	for (auto it_joint : _joints) {
		delete it_joint;
	}
	_joints.clear();
}


// suppose que les joints sont triés de sorte qu'un enfant vient tjrs apres son parent.....
// + rapide que du récursif
void InstanceAnimation::compute_mats() {
	for (auto it_joint : _joints) {
		if (it_joint->_model_joint->_sid== ROOT_JOINT_SID) {
			it_joint->_current_mat= it_joint->_local_current_mat;
		}
		else {
			it_joint->_current_mat= it_joint->_parent->_current_mat* it_joint->_local_current_mat;
		}
		it_joint->_skinning_mat= it_joint->_current_mat* it_joint->_model_joint->_inv_bind_mat;
	}
}


void InstanceAnimation::anim(unsigned int delta_time_ms) {
	for (auto it_joint : _joints) {
		if (it_joint->_model_joint->_sid== ROOT_JOINT_SID)
			continue;
		
		it_joint->anim(delta_time_ms);
	}

	compute_mats();
}


void InstanceAnimation::print() {
	cout << "model_animation ......." << endl;
	_model_animation->print();
	cout << "joints ......." << endl;
	for (auto it_joint : _joints) {
		it_joint->print();
	}
}


// --------------------------------------------------------------------------------------------
ModelMesh::ModelMesh() {

}


ModelMesh::ModelMesh(GLuint prog_draw, vector<Geometry> & geometries) : _prog_draw(prog_draw) {

	_n_faces= 0;
	for (unsigned int idx_geom=0; idx_geom<geometries.size(); ++idx_geom) {
		_n_faces+= geometries[idx_geom]._vertices.size()/ 3;
	}
	
	_vertices= new float[(3+ 3+ 3+ 4+ 4)* 3* _n_faces];
	for (unsigned int i=0; i<(3+ 3+ 3+ 4+ 4)* 3* _n_faces; ++i) {
		_vertices[i]= 0.0f;
	}

	_faces= new unsigned int[3* _n_faces];
	for (unsigned int i=0; i<3* _n_faces; ++i) {
		_faces[i]= 0;
	}
	
	unsigned int idx_vertex_total= 0;
	for (unsigned int idx_geom=0; idx_geom<geometries.size(); ++idx_geom) {
		for (unsigned int idx_vertex=0; idx_vertex<geometries[idx_geom]._vertices.size(); ++idx_vertex) {

				float joints_indices[4];
				for (unsigned int idx_joint=0; idx_joint<4; ++idx_joint) {
					if (idx_joint< geometries[idx_geom]._vertices[idx_vertex]._joints.size()) {
						joints_indices[idx_joint]= (float)(geometries[idx_geom]._vertices[idx_vertex]._joints[idx_joint]._idx_joint);
					}
					else {
						joints_indices[idx_joint]= 0.0f; // bidon ...
					}
				}
				
				float weights[4];
				for (unsigned int idx_joint=0; idx_joint<4; ++idx_joint) {
					if (idx_joint< geometries[idx_geom]._vertices[idx_vertex]._joints.size()) {
						weights[idx_joint]= geometries[idx_geom]._vertices[idx_vertex]._joints[idx_joint]._weight;
					}
					else {
						weights[idx_joint]= 0.0f;
					}
				}
				
				_vertices[17* idx_vertex_total+  0]= geometries[idx_geom]._vertices[idx_vertex]._position.x;
				_vertices[17* idx_vertex_total+  1]= geometries[idx_geom]._vertices[idx_vertex]._position.y;
				_vertices[17* idx_vertex_total+  2]= geometries[idx_geom]._vertices[idx_vertex]._position.z;
				_vertices[17* idx_vertex_total+  3]= geometries[idx_geom]._vertices[idx_vertex]._normal.x;
				_vertices[17* idx_vertex_total+  4]= geometries[idx_geom]._vertices[idx_vertex]._normal.y;
				_vertices[17* idx_vertex_total+  5]= geometries[idx_geom]._vertices[idx_vertex]._normal.z;
				_vertices[17* idx_vertex_total+  6]= geometries[idx_geom]._vertices[idx_vertex]._color.x;
				_vertices[17* idx_vertex_total+  7]= geometries[idx_geom]._vertices[idx_vertex]._color.y;
				_vertices[17* idx_vertex_total+  8]= geometries[idx_geom]._vertices[idx_vertex]._color.z;
				_vertices[17* idx_vertex_total+  9]= joints_indices[0];
				_vertices[17* idx_vertex_total+ 10]= joints_indices[1];
				_vertices[17* idx_vertex_total+ 11]= joints_indices[2];
				_vertices[17* idx_vertex_total+ 12]= joints_indices[3];
				_vertices[17* idx_vertex_total+ 13]= weights[0];
				_vertices[17* idx_vertex_total+ 14]= weights[1];
				_vertices[17* idx_vertex_total+ 15]= weights[2];
				_vertices[17* idx_vertex_total+ 16]= weights[3];

				idx_vertex_total++;
		}
		//break;
	}

	// ----------------------------------------------------------------------------------------------
	// Buffer d'indices : puisque l'on duplique tous les sommets pour ne pas avoir de normale partag�e, 
	// faces = { 0,1,2,3,4,5,6,7,8,9,10,... }
	for (unsigned int i=0; i<3* _n_faces; ++i) {
		_faces[i]= i;
	}
	
	/* buffers est un tableau de 2 indices qui nous permettra de rappeler le tableau de donn�es
		(sommets, couleurs, normales, ...) et le tableau d'indices des triangles */
	glGenBuffers(2, _buffers);
	
	// TODO : tester DYNAMIC ...
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 3+ 4+ 4)* 3* _n_faces* sizeof(float), _vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3* _n_faces* sizeof(unsigned int), _faces, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(_prog_draw);

	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc       = glGetAttribLocation(_prog_draw, "normal_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_indices_loc      = glGetAttribLocation(_prog_draw, "indices");
	_weights_loc      = glGetAttribLocation(_prog_draw, "weights");

	_ambient_color_loc= glGetUniformLocation(_prog_draw, "ambient_color");
	_shininess_loc    = glGetUniformLocation(_prog_draw, "shininess");
	_alpha_loc        = glGetUniformLocation(_prog_draw, "alpha");
	
	_model2clip_loc  = glGetUniformLocation(_prog_draw, "model2clip_matrix");
	_model2camera_loc= glGetUniformLocation(_prog_draw, "model2camera_matrix");
	_normal_mat_loc  = glGetUniformLocation(_prog_draw, "normal_matrix");
	
	_joints_loc= glGetUniformLocation(_prog_draw, "joints");
	
	glUseProgram(0);

	// AABB
	glm::vec3 vmin= glm::vec3(1e8, 1e8, 1e8);
	glm::vec3 vmax= glm::vec3(-1e8, -1e8, -1e8);
	for (unsigned int i=0; i< 3*_n_faces; ++i) {
		if (_vertices[17* i+ 0]< vmin.x) vmin.x= _vertices[17* i+ 0];
		if (_vertices[17* i+ 0]> vmax.x) vmax.x= _vertices[17* i+ 0];
		if (_vertices[17* i+ 1]< vmin.y) vmin.y= _vertices[17* i+ 1];
		if (_vertices[17* i+ 1]> vmax.y) vmax.y= _vertices[17* i+ 1];
		if (_vertices[17* i+ 2]< vmin.z) vmin.z= _vertices[17* i+ 2];
		if (_vertices[17* i+ 2]> vmax.z) vmax.z= _vertices[17* i+ 2];
	}
	_aabb= new AABB(vmin, vmax);

	// nettoyage des objets qui ne sont plus necessaires
	delete[] _faces;
	_faces= NULL;
	delete[] _vertices;
	_vertices= NULL;
}


ModelMesh::~ModelMesh() {
	if (_faces) {
		delete[] _faces;
	}
	if (_vertices) {
		delete[] _vertices;
	}
	delete _aabb;
}


// --------------------------------------------------------------------------------------------
InstanceMesh::InstanceMesh() {

}


InstanceMesh::InstanceMesh(ModelMesh * model_mesh) : 
	_model_mesh(model_mesh), _alpha(1.0f), _model2world(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f)), _model2camera(glm::mat4(1.0f)),
	_normal(glm::mat3(1.0f))
{
	// TODO : a faire évoluer ----------
	_shininess= 1.0f;
	_ambient= glm::vec3(0.6f, 0.6f, 0.6f);
}


InstanceMesh::~InstanceMesh() {
	
}


void InstanceMesh::draw() {
	glUseProgram(_model_mesh->_prog_draw);
	// On precise les donnees que l'on souhaite utiliser
	glBindBuffer(GL_ARRAY_BUFFER, _model_mesh->_buffers[0]);
	// On precise le tableau d'indices de triangle a utiliser
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _model_mesh->_buffers[1]);
	
	glUniformMatrix4fv(_model_mesh->_model2clip_loc  , 1, GL_FALSE, glm::value_ptr(_model2clip));
	glUniformMatrix4fv(_model_mesh->_model2camera_loc, 1, GL_FALSE, glm::value_ptr(_model2camera));
	glUniformMatrix3fv(_model_mesh->_normal_mat_loc  , 1, GL_FALSE, glm::value_ptr(_normal));
	glUniform3fv(_model_mesh->_ambient_color_loc, 1, glm::value_ptr(_ambient));
	glUniform1f(_model_mesh->_shininess_loc, _shininess);
	glUniform1f(_model_mesh->_alpha_loc, _alpha);
	glUniformMatrix4fv(_model_mesh->_joints_loc, N_JOINTS, GL_FALSE, glm::value_ptr(_joints_mats[0]));

	// Enables the attribute indices
	glEnableVertexAttribArray(_model_mesh->_position_loc);
	glEnableVertexAttribArray(_model_mesh->_normal_loc);
	glEnableVertexAttribArray(_model_mesh->_diffuse_color_loc);
	glEnableVertexAttribArray(_model_mesh->_indices_loc);
	glEnableVertexAttribArray(_model_mesh->_weights_loc);

	// Modifie les tableaux associes au buffer en cours d'utilisation
	glVertexAttribPointer(_model_mesh->_position_loc     , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3+ 4+ 4)* sizeof(float), 0);
	glVertexAttribPointer(_model_mesh->_normal_loc       , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3+ 4+ 4)* sizeof(float), (void *)(3* sizeof(float)));
	glVertexAttribPointer(_model_mesh->_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3+ 4+ 4)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
	glVertexAttribPointer(_model_mesh->_indices_loc      , 4, GL_FLOAT, GL_FALSE, (3+ 3+ 3+ 4+ 4)* sizeof(float), (void *)((3+ 3+ 3)* sizeof(float)));
	glVertexAttribPointer(_model_mesh->_weights_loc      , 4, GL_FLOAT, GL_FALSE, (3+ 3+ 3+ 4+ 4)* sizeof(float), (void *)((3+ 3+ 3+ 4)* sizeof(float)));
	
	// Rendu de notre geometrie
	glDrawElements(GL_TRIANGLES, _model_mesh->_n_faces* 3, GL_UNSIGNED_INT, 0);

	// Disables the attribute indices
	glDisableVertexAttribArray(_model_mesh->_position_loc);
	glDisableVertexAttribArray(_model_mesh->_normal_loc);
	glDisableVertexAttribArray(_model_mesh->_diffuse_color_loc);
	glDisableVertexAttribArray(_model_mesh->_indices_loc);
	glDisableVertexAttribArray(_model_mesh->_weights_loc);
	
	// on reinit a 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void InstanceMesh::anim(ViewSystem * view_system, vector<InstanceJoint *> & joints) {
	// on remplit _joint_mats avec les _skinning_mats
	for (unsigned int idx_joint=0; idx_joint<N_JOINTS; ++idx_joint) {
		_joints_mats[idx_joint]= glm::mat4(1.0f);
	}
	for (unsigned int idx_joint=0; idx_joint<joints.size(); ++idx_joint) {
		if (idx_joint< N_JOINTS) {
			_joints_mats[idx_joint]= joints[idx_joint]->_skinning_mat;
		}
	}

	_model2camera= view_system->_world2camera* _model2world;
	_model2clip= view_system->_camera2clip* _model2camera;
	// theoriquement il faudrait prendre la transposee de l'inverse mais si model2camera est 
	// une matrice orthogonale, TRANS(INV(M)) == M, ce qui est le cas lorsqu'elle ne comprend que 
	// des translations et rotations
	_normal= glm::mat3(_model2camera);
}


// --------------------------------------------------------------------------------------------
Skeleton::Skeleton() {

}


Skeleton::Skeleton(GLuint prog_draw, vector<InstanceJoint *> & joints) :
	_prog_draw(prog_draw), _model2world(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f))
{
	_n_joints= joints.size();

	_data= new float[_n_joints* 2* 6];
	for (unsigned int i=0; i<_n_joints* 2* 6; ++i)
		_data[i]= 0.0f;
	
	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_joints* 2* 6* sizeof(float), _data, GL_STATIC_DRAW);

	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_model2clip_loc= glGetUniformLocation(_prog_draw, "model2clip_matrix");
}


Skeleton::~Skeleton() {
	delete _data;
}


void Skeleton::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_model2clip_loc, 1, GL_FALSE, glm::value_ptr(_model2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, _n_joints* 2);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Skeleton::anim(ViewSystem * view_system, vector<InstanceJoint *> & joints) {
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_joints* 2* 6* sizeof(float), _data, GL_STATIC_DRAW);
	for (unsigned int idx_joint=0; idx_joint<joints.size(); ++idx_joint) {
		glm::vec3 translation_1, translation_2;
		translation_1= glm::vec3(joints[idx_joint]->_current_mat[3]);

		if (joints[idx_joint]->_model_joint->_sid== ROOT_JOINT_SID) {
			translation_2= glm::vec3(joints[idx_joint]->_current_mat[3]);
		}
		else {
			translation_2= glm::vec3(joints[idx_joint]->_parent->_current_mat[3]);
		}

		_data[12* idx_joint+ 0]= translation_1.x;
		_data[12* idx_joint+ 1]= translation_1.y;
		_data[12* idx_joint+ 2]= translation_1.z;
		_data[12* idx_joint+ 3]= 1.0f;
		_data[12* idx_joint+ 4]= 0.0f;
		_data[12* idx_joint+ 5]= 0.0f;
		_data[12* idx_joint+ 6]= translation_2.x;
		_data[12* idx_joint+ 7]= translation_2.y;
		_data[12* idx_joint+ 8]= translation_2.z;
		_data[12* idx_joint+ 9]= 0.0f;
		_data[12* idx_joint+ 10]= 1.0f;
		_data[12* idx_joint+ 11]= 0.0f;
	}

	_model2clip= view_system->_camera2clip* view_system->_world2camera* _model2world;
}


// --------------------------------------------------------------------------------------------
AnimatedModel::AnimatedModel() {

}


AnimatedModel::AnimatedModel(string ch_config_file, GLuint prog_3d) : _ch_config_file(ch_config_file) {
	parse_config(ch_config_file);

	_mesh= new ModelMesh(prog_3d, _animations[0]->_geometries);

	for (auto it_anim : _animations) {
		it_anim->clean();
	}
}


AnimatedModel::~AnimatedModel() {
	delete _mesh;
	for (auto it_anim : _animations) {
		delete it_anim;
	}
	_animations.clear();
}


void AnimatedModel::parse_config(std::string ch_config_file) {
	// création de l'arbre XML
	ifstream config_file(ch_config_file);
	stringstream buffer;
	buffer << config_file.rdbuf();
	config_file.close();
	string config_content(buffer.str());

	xml_document<> doc;
	doc.parse<0>(&config_content[0]);
	
	xml_node<> * root_node= doc.first_node();
	

	float angle_0= stof(root_node->first_node("rot_0")->first_node("angle")->value());
	string str_axis= root_node->first_node("rot_0")->first_node("axis")->value();
	istringstream iss_axis(str_axis);
	vector<float> split_axis((istream_iterator<float>(iss_axis)), istream_iterator<float>());
	glm::vec3 axis_0= glm::vec3(split_axis[0], split_axis[1], split_axis[2]);
	_rotation_0= glm::angleAxis(angle_0, axis_0);
	
	xml_node<> * anims_node= root_node->first_node("anims");
	for (xml_node<> * anim_node=anims_node->first_node("anim"); anim_node; anim_node=anim_node->next_sibling()) {
		//string anim_id= anim_node->first_attribute("id")->value();
		string anim_path= anim_node->first_node("path")->value();
		float moving_speed= stof(anim_node->first_node("moving_speed")->value());
		
		ModelAnimation * anim= new ModelAnimation(anim_path, moving_speed);
		_animations.push_back(anim);
	}
}


void AnimatedModel::print() {
	cout << "animations ...." << endl;
	for (auto it_anim : _animations) {
		it_anim->print();
	}
}


// --------------------------------------------------------------------------------------------
AnimatedInstance::AnimatedInstance() {

}


AnimatedInstance::AnimatedInstance(AnimatedModel * model, const glm::vec3 & scale, GLuint prog_3d, GLuint prog_basic) :
	_model(model), _draw_mesh(true), _draw_skeleton(false), _animated(true), _current_idx_anim(0), _status(STATIC)
{
	for (auto it_anim : _model->_animations) {
		InstanceAnimation * ia= new InstanceAnimation(it_anim);
		_animations.push_back(ia);
	}
	_skeleton= new Skeleton(prog_basic, _animations[_current_idx_anim]->_joints);
	_mesh= new InstanceMesh(_model->_mesh);

	_pos_rot= new InstancePosRot(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f), _model->_mesh->_aabb);
	set_pos_rot_scale(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), scale);

	// init des matrices et tout le tintouin
	for (unsigned int idx_anim=0; idx_anim<_model->_animations.size(); ++idx_anim) {
		_animations[idx_anim]->anim(0);
	}

	set_status(STATIC);
}


AnimatedInstance::~AnimatedInstance() {
	delete _skeleton;
	delete _mesh;
	delete _pos_rot;
	for (auto anim : _animations) {
		delete anim;
	}
	_animations.clear();
}


void AnimatedInstance::draw() {
	if (!_pos_rot->_active) {
		return;
	}

	if (_draw_mesh) {
		_mesh->draw();
	}
	
	if (_draw_skeleton) {
		_skeleton->draw();
	}
}


void AnimatedInstance::anim(ViewSystem * view_system, unsigned int delta_time_ms) {
	if (!_pos_rot->_active) {
		return;
	}

	if (_animated) {
		_animations[_current_idx_anim]->anim(delta_time_ms);
	}

	if (_draw_mesh) {
		_mesh->anim(view_system, _animations[_current_idx_anim]->_joints);
	}

	if (_draw_skeleton) {
		_skeleton->anim(view_system, _animations[_current_idx_anim]->_joints);
	}
}


void AnimatedInstance::set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale) {
	_pos_rot->set_pos_rot_scale(position, rotation, scale);
	_mesh->_model2world= _pos_rot->_model2world;
	_skeleton->_model2world= _pos_rot->_model2world;
}


void AnimatedInstance::set_animated(bool b) {
	_animated= b;
}


void AnimatedInstance::randomize() {
	_current_idx_anim= rand_int(0, _model->_animations.size()- 1);
	set_pos_rot_scale(_pos_rot->_position, glm::angleAxis(rand_float(0.0f, M_PI* 2.0f), glm::vec3(0.0f, 0.0f, 1.0f)), _pos_rot->_scale);
	//memcpy(_mesh->_ambient, glm::value_ptr(glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f))), sizeof(float)* 3);
	//_mesh->_shininess= rand_float(0.0f, 100.0f);
}


void AnimatedInstance::print() {
	cout << "position=" << glm::to_string(_pos_rot->_position) << " ; rotation=" << glm::to_string(_pos_rot->_rotation) << " ; current_idx_anim=" << _current_idx_anim << endl;
	cout << "model ----------------------------------------" << endl;
	_model->print();
	cout << "animations --------------------------------" << endl;
	for (auto it_anim : _animations) {
		it_anim->print();
	}
}


void AnimatedInstance::set_path(const vector<glm::vec3> & path) {
	_path.clear();
	for (auto pt : path) {
		_path.push_back(pt);
	}
	_next_path_idx= 0;
	if (_path.size()> 0) {
		set_status(WAITING);
	}
	else {
		set_status(STATIC);
	}
}


void AnimatedInstance::compute_next_pos_rot() {
	if (_path.size()> 0) {
		while ((glm::distance2(glm::vec2(_path[_next_path_idx]), glm::vec2(_pos_rot->_position))< 0.1f) && (_next_path_idx< _path.size())) {
			_next_path_idx++;
		}
		if (_next_path_idx>= _path.size()) {
			_path.clear();
			_next_path_idx= 0;
			set_status(STATIC);
		}
	}

	if ((_status!= STATIC) && (_path.size()> 0)) {
		glm::vec3 direction= glm::vec3(glm::normalize(glm::vec2(_path[_next_path_idx])- glm::vec2(_pos_rot->_position)), 0.0f);
		float angle= acosf(direction.x);
		if (direction.y< 0.0f) {
			angle= 2.0f* (float)(M_PI)- angle;
		}
		glm::quat rotation= glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 forward_dir= rotation* glm::vec3(1.0f, 0.0f, 0.0f);

		set_status(MOVING);
		// TODO : stocker glm::inverse(_model->_rotation_0) au lieu de _model->_rotation_0 ?
		_next_position= _pos_rot->_position+ forward_dir* _animations[_current_idx_anim]->_model_animation->_moving_speed;
		set_status(WAITING);
		_next_rotation= rotation* glm::inverse(_model->_rotation_0);
	}
}


void AnimatedInstance::move2next_pos_rot() {
	set_pos_rot_scale(_next_position, _next_rotation, _pos_rot->_scale);
}


void AnimatedInstance::set_status(AnimatedInstanceStatus status) {
	_status= status;
	if (_status== STATIC) {
		_current_idx_anim= 0;
	}
	else if (_status== WAITING) {
		_current_idx_anim= 0;
	}
	else if (_status== MOVING) {
		_current_idx_anim= 1;
	}
}

