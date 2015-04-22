#pragma once
#include "Engine/Application.h"
#include <vector>
#include "stb_image.h"
#include "FBXFile.h"

class ProceduralGen : public Application
{
public:

struct Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 uv;
	};

	bool Startup();
	void Shutdown();
	bool Update(double _dt);
	void Render();

private:
	
	int gridRows;
	int gridColumns;

	unsigned int shaderProg;
	unsigned int untexturedProg;
	FBXFile* fbxModel;

	int LoadTexture(std::string texture);
	int seed;

	unsigned int perlinTexture, grassTexture, sandTexture, rockTexture;
	unsigned int indexCount;

	void GenerateTerrain(unsigned int rows);
	void GenerateGrid(unsigned int rows, unsigned int cols);

	void GenerateHeightMap(int _rows, int _cols);
	float* GeneratePerlinData(int _dims, int _scale);

	void CreateOpenGLBuffers(FBXFile* _model);
	void CleanupOpenGLBuffers(FBXFile* _model);
};