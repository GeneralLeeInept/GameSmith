#version 450

vec2 verts[] =
{
	{ 0.0, 0.5 },
	{ -0.5, -0.5 },
	{ 0.5, -0.5 }
};

vec3 colors[] =
{
	{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 }
};

layout(location = 0) out vec4 color;

void main()
{
	vec2 vpos = verts[gl_VertexIndex];
	gl_Position = vec4(vpos, 0, 1);
	color = vec4(colors[gl_VertexIndex], 1);
}
