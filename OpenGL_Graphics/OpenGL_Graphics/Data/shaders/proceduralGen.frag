#version 410

in vec2 fragTexcoord;

out vec4 outColour;

uniform sampler2D perlinTexture;

void main()
{
	outColour = texture(perlinTexture, fragTexcoord).rrrr;
	outColour.a = 1;
}