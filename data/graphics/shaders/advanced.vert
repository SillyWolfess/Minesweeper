#version 450 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_uv;

layout(location = 3) in vec3 tangents;
layout(location = 4) in vec3 bitangents;
layout(location = 5) in vec3 vertex_normal;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 VP;

out vec3 colour;
out vec2 uv;
out vec3 pos;
out mat3 TBN;
void main()
{
	gl_Position = MVP * vec4(vertex_position, 1);

	vec3 T = normalize(vec3(M * vec4(tangents, 0.0)));
  	vec3 B = normalize(vec3(M * vec4(bitangents, 0.0)));
 	vec3 N = normalize(vec3(M * vec4(vertex_normal, 0.0)));

	TBN = mat3(T, B, N);
	pos = vec3(VP * vec4(vertex_position, 1.0));
	colour = vertex_color;
	uv = vertex_uv;
}

