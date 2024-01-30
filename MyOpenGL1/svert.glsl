#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aUV;

uniform float timePassed;
uniform mat4 transform;
uniform mat4 projection;

out vec3 color;

void main()
{
	gl_Position = projection * transform * vec4(aPos, 1.0);	
	color = aCol;
}