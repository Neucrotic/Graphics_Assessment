#include "ProceduralGen.h"
#include "Engine\MobileCamera.h"
#include "Engine\ShaderHandler.h"

bool ProceduralGen::Startup()
{
	MobileCamera* camera = new MobileCamera(100.0f, 0.1f);
	camera->SetInputWindow(window);
	camera->SetupPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	camera->LookAt(glm::vec3(100, 100, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_camera = camera;

	//GenerateHeightMap(200, 200);
	GenerateGrid(200, 200);
	GenerateTerrain(200);

	shaderProg = ShaderHandler::Get()->LoadShader((string)"GridShader", "Data/shaders/litTerrain.vert", "Data/shaders/litTerrain.frag", "Data/shaders/litTerrain.geom");
	untexturedProg = ShaderHandler::Get()->LoadShader((string)"UntexturedShader", "Data/shaders/untextured.vert", "Data/shaders/untextured.frag");

	grassTexture = LoadTexture("Data/textures/grass.jpg");
	sandTexture = LoadTexture("Data/textures/sand.jpg");
	rockTexture = LoadTexture("Data/textures/iceREPLACE.jpg");

	fbxModel = new FBXFile();
	fbxModel->load("Data/models/Bunny.fbx");

	CreateOpenGLBuffers(fbxModel);

	return true;
}

float* ProceduralGen::GeneratePerlinData(int _dims, int _scale)
{
	return new float;
}

void ProceduralGen::Shutdown()
{
	CleanupOpenGLBuffers(fbxModel);
}

bool ProceduralGen::Update(double _dt)
{
	m_camera->Update(_dt);



	return true;
}

void ProceduralGen::Render()
{
	glUseProgram(shaderProg);

	int location = glGetUniformLocation(shaderProg, "ProjectionView");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(m_camera->GetProjectionView()));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	location = glGetUniformLocation(shaderProg, "PerlinTexture");
	glUniform1i(location, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	location = glGetUniformLocation(shaderProg, "GrassTexture");
	glUniform1i(location, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sandTexture);
	location = glGetUniformLocation(shaderProg, "SandTexture");
	glUniform1i(location, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, rockTexture);
	location = glGetUniformLocation(shaderProg, "SnowTexture");
	glUniform1i(location, 3);


	glBindVertexArray(buffers->VAO);

	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(untexturedProg);
	location = glGetUniformLocation(untexturedProg, "ProjectionView");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(m_camera->GetProjectionView()));

	//rendering the bunny
	for (unsigned int i = 0; i < fbxModel->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = fbxModel->getMeshByIndex(i);

		unsigned int* glData = (unsigned int*)mesh->m_userData;

		glBindVertexArray(glData[0]);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}
}

int ProceduralGen::LoadTexture(std::string file)
{
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load(file.c_str(), &imageWidth, &imageHeight, &imageFormat, STBI_default);

	unsigned int texture;

	switch (imageFormat)
	{
		case 3:
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case 4:
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		default:
			return 0;
			break;
	}

	return texture;

	stbi_image_free(data);
}

void ProceduralGen::GenerateHeightMap(int _rows, int _cols)
{
	Vertex* vertices = new Vertex[_rows * _cols];
	for (unsigned int r = 0; r < _rows; ++r)
	{
		for (unsigned int c = 0; c < _cols; ++c)
		{
			vertices[r * _cols + c].position = glm::vec4((float)c, 0, (float)r, 1);

			glm::vec3 colour = glm::vec3(1, 1, 1);

			vertices[r * _cols + c].normal = glm::vec4(colour, 1);
			vertices[r * _cols + c].uv = glm::vec2((float)c / _cols, (float)r / _rows);
		}
	}

	//defining index count based of quad count (2 tri's = 1 quad)
	unsigned int* indices = new unsigned int[(_rows - 1) * (_cols - 1) * 6];
	unsigned int index = 0;

	for (unsigned int r = 0; r < (_rows - 1); ++r)
	{
		for (unsigned int c = 0; c < (_cols - 1); ++c)
		{
			//tri 1
			indices[index++] = r * _cols + c;
			indices[index++] = (r + 1) * _cols + c;
			indices[index++] = (r + 1) * _cols + (c + 1);

			//tri 2
			indices[index++] = r * _cols + c;
			indices[index++] = (r + 1) * _cols + (c + 1);
			indices[index++] = r * _cols + (c + 1);
		}
	}

	//generating perlin data
	float* perlinData = GeneratePerlinData(200, 50);

	//generating noise texture
	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 64, 64, 0, GL_RED, GL_FLOAT, perlinData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//generating VAO, VBO and IBO
	glGenBuffers(1, &buffers->VBO);
	glGenBuffers(1, &buffers->IBO);

	glGenVertexArrays(1, &buffers->VAO);
	glBindVertexArray(buffers->VAO);

	//binding and filling VBO
	glBindBuffer(GL_ARRAY_BUFFER, buffers->VBO);
	glBufferData(GL_ARRAY_BUFFER, (_rows * _cols) * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	//binding and filling IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (_rows - 1) * (_cols - 1) * 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	//binding VAO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//save function parametres for member varibles for external use
	gridRows = _rows;
	gridColumns = _cols;

	delete[] vertices;
	delete[] indices;
}

void ProceduralGen::GenerateTerrain(unsigned int rows)
{
	glm::vec2 dims(rows, rows);

	float *perlin_data = new float[(int)dims.x * (int)dims.y];
	float scale = (1.0f / dims.x) * 3;
	int octaves = 6;
	for (int x = 0; x < rows; ++x)
	{
		for (int y = 0; y < rows; ++y)
		{
			float amplitude = 1.f;
			float persistence = 0.3f;
			perlin_data[y * (int)dims.x + x] = 0;
			for (int o = 0; o < octaves; ++o)
			{
				float freq = powf(2, (float)o);
				float perlin_sample = glm::perlin(glm::vec3((float)x, (float)y, seed) * scale * freq) * 0.5f + 0.5f;
				perlin_data[y * (int)dims.x + x] += perlin_sample * amplitude;
				amplitude *= persistence;
			}
		}
	}

	glGenTextures(1, &perlinTexture);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, rows, rows, 0, GL_RED, GL_FLOAT, perlin_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// function to create a grid
void ProceduralGen::GenerateGrid(unsigned int rows, unsigned int cols)
{
	Vertex* aoVertices = new Vertex[rows * cols];

	//Nested loop for looping through rows then columns for the grid
	for (unsigned int r = 0; r < rows; ++r)
	{
		for (unsigned int c = 0; c < cols; ++c)
		{
			aoVertices[r * cols + c].position = glm::vec4((float)c, 0, (float)r, 1);
			aoVertices[r * cols + c].normal = glm::vec4(0, 1, 0, 1);

			// create some arbitrary colour based off something
			// that might not be related to tiling a texture
			aoVertices[r * cols + c].uv = glm::vec2((r / (float)rows), (c / (float)cols));
		}
	}

	// defining index count based off quad count (2 triangles per quad)
	unsigned int* auiIndices = new unsigned int[(rows - 1) * (cols - 1) * 6];
	unsigned int index = 0;

	for (unsigned int r = 0; r < (rows - 1); ++r) {
		for (unsigned int c = 0; c < (cols - 1); ++c) {
			// triangle 1
			auiIndices[index++] = r * cols + c;
			auiIndices[index++] = (r + 1) * cols + c;
			auiIndices[index++] = (r + 1) * cols + (c + 1);
			// triangle 2
			auiIndices[index++] = r * cols + c;
			auiIndices[index++] = (r + 1) * cols + (c + 1);
			auiIndices[index++] = r * cols + (c + 1);
		}
	}

	unsigned int ibo;
	indexCount = (rows - 1) * (cols - 1) * 6;

	//Create VBO
	glGenBuffers(1, &buffers->VBO);

	//Create IBO
	glGenBuffers(1, &ibo);

	//Create VAO
	glGenVertexArrays(1, &buffers->VAO);
	glBindVertexArray(buffers->VAO);

	//Bind VBO and define size of data
	glBindBuffer(GL_ARRAY_BUFFER, buffers->VBO);
	glBufferData(GL_ARRAY_BUFFER, (rows * cols) * sizeof(Vertex), aoVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);//pos
	glEnableVertexAttribArray(1);//normals
	glEnableVertexAttribArray(2);//texcoords

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec4)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec4)+sizeof(glm::vec4)));

	//Bind IBO and define size of data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), auiIndices, GL_STATIC_DRAW);

	//Unbind the VAO (which unbinds its VBO and IBO)
	glBindVertexArray(0);

	//Delete our temp indecies
	delete[] auiIndices;

	//Delete our temp verticies
	delete[] aoVertices;
}

void ProceduralGen::CreateOpenGLBuffers(FBXFile* _model)
{
	for (unsigned int i = 0; i < _model->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = _model->getMeshByIndex(i);

		unsigned int* glData = new unsigned int[3];

		glGenVertexArrays(1, &glData[0]);
		glBindVertexArray(glData[0]);

		glGenBuffers(1, &glData[1]);
		glGenBuffers(1, &glData[2]);

		glBindBuffer(GL_ARRAY_BUFFER, glData[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData[2]);

		glBufferData(GL_ARRAY_BUFFER,
			mesh->m_vertices.size() * sizeof(FBXVertex),
			mesh->m_vertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			mesh->m_indices.size() * sizeof(unsigned int),
			mesh->m_indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); //position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);

		glEnableVertexAttribArray(1); //normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh->m_userData = glData;
	}
}

void ProceduralGen::CleanupOpenGLBuffers(FBXFile* _model)
{
	for (unsigned int i = 0; i < _model->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = _model->getMeshByIndex(i);

		unsigned int* glData = (unsigned int*)mesh->m_userData;

		glDeleteVertexArrays(1, &glData[0]);
		glDeleteBuffers(1, &glData[1]);
		glDeleteBuffers(1, &glData[2]);

		delete[] glData;
	}
}