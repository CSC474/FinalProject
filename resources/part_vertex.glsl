#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 MA;

out vec3 vertex_pos;
out float opaci;
void main()
{
    mat4 Ma = MA;
    vec4 pos= Ma*vec4(vertPos,1.0);

//the animation matrix already holds the end position for the segment
    pos.x = Ma[3][0];
    pos.y = Ma[3][1];
    pos.z = Ma[3][2];

    gl_Position = P * V * M * pos;
    vertex_pos = pos.xyz;
}
