#version 450 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 VP;

out vec3 colour;
void main()
{
	gl_Position = MVP * vec4 (vertex_position, 1);
	colour = vertex_color;
}

