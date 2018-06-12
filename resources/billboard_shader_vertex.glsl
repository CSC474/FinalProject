#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 MFollow;
out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;
void main()
{
    vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;
    vec4 pos = MFollow*vec4(vertPos,1.0);
    
    //the animation matrix already holds the end position for the segment
    pos.x = MFollow[3][0];
    pos.y = MFollow[3][1];
    pos.z = MFollow[3][2];
    gl_Position = P * V * M * pos;
    vertex_tex = vertTex;
    vertex_pos = pos.xyz;
}
