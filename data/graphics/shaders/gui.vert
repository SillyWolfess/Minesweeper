#version 450 core

layout(location = 0) in vec3 vertex_position;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 VP;

void main()
{
	gl_Position = MVP * vec4 (vertex_position, 1);
}