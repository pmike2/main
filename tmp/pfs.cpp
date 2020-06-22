
#include "pfs.h"

using namespace std;

PFS::PFS() {

}


PFS::PFS(unsigned int size_x, unsigned int size_y, unsigned int size_z, GLuint prog_draw) : _size_x(size_x), _size_y(size_y), _size_z(size_z), _prog_draw(prog_draw) {
    _tab= new unsigned int[_size_x* _size_y* _size_z];
   	for (unsigned int i=0; i< _size_x* _size_y* _size_z; ++i) {
		_tab[i]= (unsigned int)(rand_int(0, 2));
	}

    for (unsigned int idx_group=0; idx_group<3; ++idx_group) {
        stringstream ss;
        ss << "./data/cube" << idx_group+ 1 << ".xml";
        const float CUBE_SIZE= 1.0f;
        vector<InstancePosRot *> pos_rots;
        for (unsigned int i=0; i< _size_x* _size_y* _size_z; ++i) {
            glm::vec3 ipos= idx2pos(i);
            glm::vec3 pos= glm::vec3(-CUBE_SIZE* _size_x* 0.5f+ (float)(ipos.x)* CUBE_SIZE, -CUBE_SIZE* _size_y* 0.5f+ (float)(ipos.y)* CUBE_SIZE, -CUBE_SIZE* _size_z* 0.5f+ (float)(ipos.z)* CUBE_SIZE);
            pos_rots.push_back(new InstancePosRot(pos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(CUBE_SIZE* 0.2f)));
        }
        std::vector<float> distances;
        distances.push_back(1.0f);

        StaticGroup * sg= new StaticGroup(ss.str(), _prog_draw, pos_rots, distances);
        sg->_alpha= 0.1f;
        _group_cubes.push_back(sg);

        for (auto pr : pos_rots) {
            delete pr;
        }
    }
}


PFS::~PFS() {
    delete[] _tab;
    for (auto gc : _group_cubes) {
        delete gc;
    }
    _group_cubes.clear();
}


glm::uvec3 PFS::idx2pos(unsigned int idx) {
    unsigned int x= idx % _size_x;
    unsigned int y= (idx % (_size_x* _size_y))/ _size_x;
    unsigned int z= idx/ (_size_x* _size_y);
    return glm::uvec3(x, y, z);
}


unsigned int PFS::pos2idx(glm::uvec3 pos) {
    return pos.x+ _size_x* pos.y+ _size_x* _size_y* pos.z;
}


void PFS::draw() {
    for (auto gc : _group_cubes) {
        gc->draw();
    }
}


void PFS::anim(ViewSystem * view_system) {
    for (unsigned int i=0; i<1000; ++i) {
        update_tab();
    }

    for (unsigned int idx_group=0; idx_group<3; ++idx_group) {
        for (unsigned int i=0; i< _size_x* _size_y* _size_z; ++i) {
            _group_cubes[idx_group]->_pos_rots[i]->_active= false;
            if (_tab[i]== idx_group) {
                _group_cubes[idx_group]->_pos_rots[i]->_active= true;
            }
        }

        _group_cubes[idx_group]->anim(view_system);
    }
}


void PFS::update_tab() {
    int x= rand_int(0, _size_x- 1);
    int y= rand_int(0, _size_y- 1);
    int z= rand_int(0, _size_z- 1);
    unsigned int idx= pos2idx(glm::uvec3((unsigned int)(x), (unsigned int)(y), (unsigned int)(z)));
    vector<unsigned int> l;
    for (int i= -1; i<2; ++i) {
        for (int j= -1; j<2; ++j) {
            for (int k= -1; k<2; ++k) {
                if ((i== 0) && (j== 0) && (k== 0)) {
                    continue;
                }
                if ((x+ i< 0) || (x+ i>= _size_x) || (y+ j< 0) || (y+ j>= _size_y) || (z+ k< 0) || (z+ k>= _size_z)) {
                    continue;
                }
                unsigned int idx_neighbor= pos2idx(glm::uvec3((unsigned int)(x+ i), (unsigned int)(y+ j), (unsigned int)(z+ k)));
                l.push_back(idx_neighbor);
            }
        }
    }
    unsigned int ii= (unsigned int)(rand_int(0, l.size()- 1));
    unsigned int neighbor= l[ii];
    if (_tab[idx]== 0) {
        if (_tab[neighbor]== 1) {
            _tab[idx]= 1;
        }
        else if (_tab[neighbor]== 2) {
            _tab[neighbor]= 0;
        }
    }
    else if (_tab[idx]== 1) {
        if (_tab[neighbor]== 2) {
            _tab[idx]= 2;
        }
        else if (_tab[neighbor]== 0) {
            _tab[neighbor]= 1;
        }
    }
    else if (_tab[idx]== 2) {
        if (_tab[neighbor]== 0) {
            _tab[idx]= 0;
        }
        else if (_tab[neighbor]== 1) {
            _tab[neighbor]= 2;
        }
    }
}

