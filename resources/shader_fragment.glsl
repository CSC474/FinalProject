#version 410 core
out vec4 color;
in vec3 vertex_pos;
uniform vec3 campos;
uniform float Dancer;
uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
    if(Dancer == 2)
    {
        color = vec4(0,1,1,1);
    }

    if(Dancer == 0)
    {
        color = vec4(1,1,1,1);
    }
    if(Dancer == 0.005)
    {
        color = vec4(0.08,0.96,0.08,0.9);
    }
    if(Dancer == 0.010)
    {
        color = vec4(0.08,0.96,0.08,0.8);
    }
    if(Dancer == 0.015)
    {
        color = vec4(0.08,0.96,0.08,0.7);
    }
    if(Dancer == 0.020)
    {
        color = vec4(0.08,0.96,0.08,0.6);
    }
    if(Dancer == 0.025)
    {
        color = vec4(0.08,0.96,0.08,0.5);
    }
}
