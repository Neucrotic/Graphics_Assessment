#pragma once
#include "Engine/Application.h"

class Scene : public Application
{
public:

	bool Startup();
	void Shutdown();
	bool Update(double _dt);
	void Render();

private:

	//tweak bar variables
	TwBar* tweakBar;
	float editAmplitude;
	float editPersistance;

	//variables used for Procedural Generation
	unsigned int terrainShader;
	int gridRows;
	int gridColumns;
	unsigned int perlinTexture;

	//variables used for Particles


	//variables used for Objs


};