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

	GenerateHeightMap(64, 64);

	shaderProg = ShaderHandler::Get()->LoadShader((string)"GridShader", "Data/shaders/proceduralGen.vert", "Data/shaders/proceduralGen.frag");

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
	glUseProgram(shaderProg);

	int location = glGetUniformLocation(shaderProg, "ProjectionView");
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(m_camera->GetProjectionView()));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, perlinTexture);

	location = glGetUniformLocation(shaderProg, "perlinTexture");
	glUniform1i(location, 0);

	glBindVertexArray(buffers->VAO);

	unsigned int indexCount = (gridRows - 1) * (gridColumns - 1) * 6;
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
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

	//generating perlin data
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
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colour));
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

float* ProceduralGen::GeneratePerlinData(int _dims, int _scale)
{
	float* perlinData = new float[_dims * _dims];

	float scale = (1.0f / _dims) * _scale;
	float octaves = 6;

	for (int x = 0; x < _dims; ++x)
	{
		for (int y = 0; y < _dims; ++y)
		{
			float amplitude = 2.0f;
			float persistance = 0.3f;
			perlinData[y * _dims + x] = 0;

			for (int o = 0; o < octaves; ++o)
			{
				float freq = powf(2, (float)o);
				float perlinSample = glm::perlin(glm::vec2((float)x, (float)y) * scale * freq) * 0.5f + 0.5f;

				perlinData[y * _dims + x] += perlinSample * amplitude;
				amplitude *= persistance;
			}
		}
	}

	return perlinData;
}