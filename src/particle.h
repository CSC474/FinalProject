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
    std::vector<vec3> speed;
    std::vector<vec3> impulse;
    
    void ResetFall() {
        for (int i = 0; i < speed.size(); i++) {
            speed[i] = vec3(0,0,0);
            impulse[i] = vec3(0,0,0);
        }
    }
};

#endif /* particle_h */
