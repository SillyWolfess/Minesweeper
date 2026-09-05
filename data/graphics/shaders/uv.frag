#version 450 core

in vec3 colour;
in vec2 uv;

out vec4 frag_colour;

void main()
{
	frag_colour = vec4(uv.xy, 0.0, 1.0);
}
