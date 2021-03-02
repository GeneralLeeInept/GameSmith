#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

layout(location = 0) out vec4 color;

void main()
{
	gl_Position = vec4(aPosition, 1);
	color = vec4(aNormal, 1) * 0.5 + 0.5;
}
