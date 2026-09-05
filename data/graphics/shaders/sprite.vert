#version 450 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 VP;

out vec2 uv;
void main()
{
	gl_Position = MVP * vec4 (vertex_position, 1);
	uv = vertex_uv;
}