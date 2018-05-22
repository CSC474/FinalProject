#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in uint vertimat;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 Manim[200];

out vec3 vertex_pos;
void main()
{
    mat4 Ma = Manim[vertimat];
    vec4 pos= Ma*vec4(vertPos,1.0);
    
    //the animation matrix already holds the end position for the segment
    pos.x = Ma[3][0];
    pos.y = Ma[3][1];
    pos.z = Ma[3][2];
    gl_Position = P * V * M * pos;	
}
