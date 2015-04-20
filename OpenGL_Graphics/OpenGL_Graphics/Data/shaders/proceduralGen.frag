#version 410

in vec2 fragTexcoord;
in vec4 fragPos;

out vec4 outColour;

uniform sampler2D perlinTexture;

void main()
{
	outColour = texture(perlinTexture, fragTexcoord).rrrr;
	outColour.a = 1;
	
	if (fragPos.y < 10)
	{
		outColour.r = -1.0f;
		outColour.b = -1.0f;
		//outColour.g = -1.0f;
	}
	if (fragPos.y > 25)
	{
		outColour.a = 1;
	}
}