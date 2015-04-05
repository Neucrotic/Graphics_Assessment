#version 410

layout(location=0) in vec4 position;
layout(location=1) in vec4 colour;
layout(location=2) in vec2 texcoord;

uniform mat4 viewProjection;
uniform sampler2D perlinTexture;

out vec2 fragTexcoord;

void main()
{
	vec4 pos = position;
	pos.y += texture(perlinTexture, texcoord).r * 10;

	fragTexcoord = texcoord;
	gl_Position = viewProjection * pos;
}