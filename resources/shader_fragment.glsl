#version 410 core
out vec4 color;
in vec3 vertex_pos;
uniform vec3 campos;
uniform int Dancer;
uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
    if(Dancer == 1)
    {
        color = vec4(0.08,0.96,0.08,1);
    }
    if(Dancer == 2)
    {
        color = vec4(0,1,1,1);
    }
}
