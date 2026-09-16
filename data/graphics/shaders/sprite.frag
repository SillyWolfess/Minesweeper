#version 450 core

uniform vec4 colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
in vec2 uv;

uniform sampler2D textureDiff;
out vec4 frag_colour;

void main()
{
	vec2 textCord = vec2(uv.s, 1.0 - uv.t);
	vec4 tex = texture(textureDiff, textCord);
	if (tex.a < 0.1)
	   discard;
	frag_colour = vec4(colour.rgb * tex.rgb, tex.aaaa);
}
