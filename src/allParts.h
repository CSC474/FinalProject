//
//  allParts.h
//  fp
//
//  Created by Liam Schmid on 6/5/18.
//

#ifndef allParts_h
#define allParts_h
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "particle.h"
struct allParts {
    std::vector<particles> cParts;
    std::vector<bool> isFalling;
    
    void ResetFall() {
        for (int i = 0; i < cParts.size(); i++)
            cParts[i].ResetFall();
    }
    
    void UpdateisFalling(int frame) {
        for (int i = 0; i < isFalling.size(); i++) {
            if (i == 0 && frame > 30)
                isFalling[i] = true;
            else if (i == 1 && frame > 90)
                isFalling[i] = true;
            else if (i == 2 && frame > 130)
                isFalling[i] = true;
        }
    }
};
#endif /* allParts_h */
