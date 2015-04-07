#include "ProceduralGen.h"
#include "Engine\MobileCamera.h"

bool ProceduralGen::Startup()
{
	MobileCamera* camera = new MobileCamera(100.0f, 0.1f);
	camera->SetInputWindow(window);
	camera->SetupPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	camera->LookAt(glm::vec3(100, 100, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_camera = camera;

	return true;
}

void ProceduralGen::Shutdown()
{

}

bool ProceduralGen::Update(double _dt)
{
	m_camera->Update(_dt);

	return true;
}

void ProceduralGen::Render()
{
	unsigned int indexCount = (gridRows - 1) * (gridColumns - 1) * 6;
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
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

			vertices[r * _cols + c].colour = glm::vec4(colour, 1);
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

	////generating perlin data
	float* perlinData = GeneratePerlinData(64, 8);

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
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec4)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec4) + sizeof(glm::vec4)));

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

float* ProceduralGen::GeneratePerlinData(int _dims, int _scale)
{
	float* perlinData = new float[_dims * _dims];



	return perlinData;
}