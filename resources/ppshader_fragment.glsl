#version 330 core
out vec4 color;
in vec2 vertex_tex;
uniform sampler2D tex;

void main()
{
	vec4 tcol = texture(tex, vertex_tex);

	float hd_x = 1./1920.;
	float hd_y = 1./1061.;
	float n = 5;
	for (float i = 1; i < n; i++){
		for (float j = 1; j < n; j++){
			//float d = dist(vec2(i,j),)
			float fade = 1 / (length(vec2(i,j)) * 4);
			vec4 temptex = texture(tex,vertex_tex + vec2(hd_x * i,hd_y * j),2);
			tcol += temptex* fade;
		}
	}
	color = tcol;
	color.r = pow(color.r,1.25);
	color.g = pow(color.g,1.25);
	color.b = pow(color.b,1.25);
	color.a=1;
}
