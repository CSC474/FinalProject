//
//  particle.h
//  fp
//
//  Created by Liam Schmid on 6/1/18.
//

#ifndef particle_h
#define particle_h

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct particles {
    std::vector<vec3> pos;
    std::vector<mat4*> ma;
};

#endif /* particle_h */
