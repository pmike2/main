#ifndef PFS_H
#define PFS_H

#include <sstream>
#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "utile.h"
#include "objfile.h"
#include "bbox.h"


class PFS {
public:
    PFS();
    PFS(unsigned int size_x, unsigned int size_y, unsigned int size_z, GLuint prog_draw);
    ~PFS();
    glm::uvec3 idx2pos(unsigned int idx);
    unsigned int pos2idx(glm::uvec3 pos);
    void draw();
    void anim(ViewSystem * view_system);
    void update_tab();


    unsigned int _size_x, _size_y, _size_z;
    GLuint _prog_draw;
    unsigned int * _tab;

    std::vector<StaticGroup *> _group_cubes;
};


#endif
