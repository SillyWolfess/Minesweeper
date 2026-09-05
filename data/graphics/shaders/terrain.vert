#version 450 core

layout(location = 0) in vec3 vertex_position;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 VP;

out vec3 colour;
out vec3 pos;
void main()
{
	gl_Position = MVP * vec4 (vertex_position, 1);
	colour = vec3(vertex_position.y / 10);
	pos = vertex_position;
}

