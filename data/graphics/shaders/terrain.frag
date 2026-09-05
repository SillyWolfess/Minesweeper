#version 450 core

in vec3 colour;
in vec3 pos;

out vec4 frag_colour;

void main()
{
	vec3 resColor = vec3(1.0, 1.0, 1.0);
	float height = pos.y;
	float waterLevel = 3;
	float snowLevel = 8;
	if (height < waterLevel ) {
		resColor = vec3(0.0, 0.0, -(waterLevel - height) * 0.3 + 1.0);
	}
	else if (height < snowLevel) {
		resColor = vec3(0.0, -(height - waterLevel) * 0.2 + 1.0, 0.0);
	} else {
		resColor = vec3(0.9);
	}
	frag_colour = vec4(resColor, 1.0);
}
