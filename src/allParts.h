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
    std::vector<bool> mvInvolved;
    
    void ResetFall() {
        for (int i = 0; i < cParts.size(); i++)
            cParts[i].ResetFall();
    }
    
    void UpdateisFalling(int frame) {
        for (int i = 0; i < isFalling.size(); i++) {
            if (frame > (i+6)*100 + 30) 
                isFalling[i] = true;
            
        }
    }
    
    void UpdatemvInvolved(int frame) {
        for (int i = 0; i < mvInvolved.size(); i++) {
            if (frame > (i+6)*100 + 100)
                mvInvolved[i] = true;
        }
    }
};
#endif /* allParts_h */
