#version 410 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec2 to;
uniform vec2 to2;
uniform float t;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
    vec2 vtex = vertex_tex;

    vec4 tcol = texture(tex, vtex);
    
    color = tcol;
}
