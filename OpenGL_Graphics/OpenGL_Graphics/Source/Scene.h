#pragma once
#include "Engine/Application.h"
#include "AntTweakBar.h"

struct Vertex
{
	glm::vec4 position;
	glm::vec4 colour;
	glm::vec2 uv;
};

class Scene : public Application
{
public:

	bool Startup();
	void Shutdown();
	bool Update(double _dt);
	void Render();

private:

	//tweak bar properties
	TwBar* tweakBar;
	float editAmplitude;
	float editScale;
	bool barChanged;
	float oldAmp;
	float oldPers;

	//properties used for Procedural Generation
	unsigned int terrainShader;
	unsigned int planeShader;
	int gridRows;
	int gridColumns;
	unsigned int perlinTexture;
	unsigned int planeTexture;
		
	float* GeneratePerlinData(int _dims, int _scale, float _amplitude);
	void GenerateTerrain(int _dimensions, int _terrScale, float _terrAmplitude);
	void CreatePlane(int _dimensions);

	//properties used for Particles


	//properties used for Objs


};