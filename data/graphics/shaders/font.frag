#version 450 core

uniform vec4 colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
out vec4 frag_colour;

void main()
{
	frag_colour = vec4(colour.r, colour.g, colour.b, 1.0f);
}
