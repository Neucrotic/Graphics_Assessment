#pragma once
#include "Engine/Application.h"
#include <vector>



class ProceduralGen : public Application
{
public:

struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
		glm::vec2 uv;
	};

	bool Startup();
	void Shutdown();
	bool Update(double _dt);
	void Render();

private:

	unsigned int shaderProg;

	int gridRows;
	int gridColumns;

	unsigned int perlinTexture;

	void GenerateHeightMap(int _rows, int _cols);
	float* GeneratePerlinData(int _dims, int _scale);
};