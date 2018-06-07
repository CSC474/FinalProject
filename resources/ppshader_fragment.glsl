#version 330 core
out vec4 color;
in vec2 vertex_tex;
uniform sampler2D tex;
uniform float timef;
uniform vec2 resolution;

void main()
{
    vec4 tcol = texture(tex, vertex_tex);
    
    float hd_x = 1./800.;
    float hd_y = 1./600.;
    float n = 5;
    for (float i = 1; i < n; i++){
        for (float j = 1; j < n; j++){
            //float d = dist(vec2(i,j),)
            float fade = 1 / (length(vec2(i,j)) );
            vec4 temptex = texture(tex,vertex_tex + vec2(hd_x * i,hd_y * j),2);
            tcol += temptex* fade;
        }
    }
    color = tcol;
    color.r = pow(color.r,1.25);
    color.g = pow(color.g,1.25);
    color.b = pow(color.b,1.25);
    color.a=1;
   
   vec2 uv = gl_FragCoord.xy / (resolution.xy *2);
   
   float f3 = 1.0 - abs(uv.y + sin(uv.x * 1.618033 * 4.0 + timef)*0.5 - 0.5);
   float f2 = 1.0 - abs(uv.y + sin(uv.x * 1.618033 * 3.0 + timef)*0.5 - 0.5);
   float f = 1.0 - abs(uv.y + sin(uv.x * 1.618033 * 2.0 + timef)*0.5 - 0.5);
   vec4 colorNew = vec4(pow(f,1.0)*pow(f2,3.0), f*f*f*f2, pow(f3,1.0) * f2 * f,1);
   //float t = 1 - (color.r +color.g+color.b)/3.;
   color += colorNew;
    
}
