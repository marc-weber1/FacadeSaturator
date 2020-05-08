
const char* VERTEX_2D_SHADER_BASIC = 
	"#version 330 core\n"

	"layout(location=0) in vec2 vertex_pos_modelspace;"

	"void main(){"
	"	gl_Position = vec4(vertex_pos_modelspace,0,1);"
	"}"
	;

const char* SINGLE_COLOR_FRAG_SHADER = 
	"#version 330 core\n"

	"uniform vec3 fragment_color;"

	"out vec3 color;"

	"void main(){"
	"	color = fragment_color;"
	"}"
	;
	
const char* IMAGE_FRAG_SHADER = 
	"#version 330 core\n"
	
	"uniform sampler2D bg_texture;"
	
	"out vec3 color;"
	
	"void main(){"
	"	color = texture(bg_texture,0.5*gl_FragCoord.xy+vec2(0.5));"
	"}"
	;
	
const char* POINT_CIRCLE_GEOMETRY_SHADER = 
	"#version 330 core\n"

	"#define TWO_PI 6.283185307\n"

	"layout (points) in;"
	"layout (line_strip, max_vertices=14) out;"

	"const float CIRCLE_RADIUS = 0.025;"

	"uniform float aspect_ratio;"

	"void main(){"
	"	for(int i=0;i<14;i++){"
	"		float angle = TWO_PI*i/(14-1);"
	"		gl_Position = gl_in[0].gl_Position + CIRCLE_RADIUS*vec4(cos(angle),aspect_ratio*sin(angle),0,0);"
	"		EmitVertex();"
	"	}"
		
	"	EndPrimitive();"
	"}"
	;