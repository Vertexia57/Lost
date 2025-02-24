#include "GLResourceManagers.h"
#include "../LostGL.h"
#include <unordered_map>
#include <iostream>
//#include <hash_set>

template <typename Out>
static void split(const std::string& s, char delim, Out result) {
	std::istringstream iss(s);
	std::string item;
	while (std::getline(iss, item, delim)) {
		*result++ = item;
	}
}

static std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

struct IndexData
{
	int vertex;
	int texcoord;
	int normal;
	std::size_t hash = 0;

	IndexData(int _vertex, int _texcoord, int _normal)
	{
		vertex = _vertex;
		texcoord = _texcoord;
		normal = _normal;
		hash = ((std::hash<int>()(vertex)
			^ (std::hash<int>()(texcoord) << 1)) >> 1)
			^ (std::hash<int>()(normal) << 1);
	};

	bool operator==(const IndexData& other) const
	{
		return vertex == other.vertex && texcoord == other.texcoord && normal == other.normal;
	}
};

template <>
struct std::hash<IndexData>
{
	std::size_t operator()(const IndexData& k) const
	{
		return k.hash;
	}
};

namespace lost
{

	ResourceManager<Texture>* _textureRM = nullptr;
	ResourceManager<Material>* _materialRM = nullptr;
	ResourceManager<Shader>* _shaderRM = nullptr;
	ResourceManager<Mesh>* _meshRM = nullptr;
	ResourceManager<Font>* _fontRM = nullptr;

	void _initRMs()
	{
		_textureRM = new ResourceManager<Texture>();
		_materialRM = new ResourceManager<Material>();
		_shaderRM = new ResourceManager<Shader>();
		_meshRM = new ResourceManager<Mesh>();
		_fontRM = new ResourceManager<Font>();
	}

	void _destroyRMs()
	{
		delete _fontRM;
		delete _textureRM;
		delete _materialRM;
		delete _shaderRM;
		delete _meshRM;
	}

#pragma region Texture

	Texture loadTexture(const char* fileLocation, const char* id)
	{
		lost::Texture tex = nullptr;

		// If "id" is nullptr set it to the filename
		if (!id) id = fileLocation;

		if (!_textureRM->hasValue(id))
		{
			tex = new lost::_Texture();
			tex->loadTexture(fileLocation);
		}
		else
			tex = _textureRM->getValue(id);

		_textureRM->addValue(tex, id);
		return tex;
	}

	Texture makeTexture(const char* data, int width, int height, const char* id, unsigned int format)
	{
		lost::Texture tex = nullptr;

		if (!_textureRM->hasValue(id))
		{
			tex = new lost::_Texture();
			tex->makeTexture(data, width, height, format);
		}
		else
			tex = _textureRM->getValue(id);

		_textureRM->addValue(tex, id);
		return tex;
	}

	Texture getTexture(const char* id)
	{
		return _textureRM->getValue(id);
	}

	void unloadTexture(const char* id)
	{
		_textureRM->destroyValue(id);
	}

	void unloadTexture(Texture texture)
	{
		_textureRM->destroyValueByValue(texture);
	}

	void forceUnloadTexture(const char* id)
	{
		_textureRM->forceDestroyValue(id);
	}

	void forceUnloadTexture(Texture texture)
	{
		_textureRM->forceDestroyValueByValue(texture);
	}
#pragma endregion

#pragma region Material
	Material makeMaterial(std::vector<Texture> textures, const char* id, Shader shader, unsigned int renderQueue)
	{
		if (shader == nullptr)
			shader = lost::_defaultShader;

		lost::Material mat = nullptr;

		if (!_materialRM->hasValue(id)) 
			mat = new lost::_Material(shader, textures, renderQueue);
		else
			mat = _materialRM->getValue(id);

		_materialRM->addValue(mat, id);
		return mat;
	}

	Material getMateral(const char* id)
	{
		return _materialRM->getValue(id);
	}

	void destroyMaterial(const char* id)
	{
		_materialRM->destroyValue(id);
	}

	void destroyMaterial(Material material)
	{
		_materialRM->destroyValueByValue(material);
	}

	void forceDestroyMaterial(const char* id)
	{
		_materialRM->forceDestroyValue(id);
	}

	void forceDestroyMaterial(Material material)
	{
		_materialRM->forceDestroyValueByValue(material);
	}

#pragma endregion

#pragma region Shader

	Shader loadShader(const char* vertexLoc, const char* fragmentLoc, const char* id)
	{
		lost::Shader shader = nullptr;

		if (!_shaderRM->hasValue(id))
		{
			shader = new lost::_Shader();
			shader->loadShader(vertexLoc, fragmentLoc);
		}
		else
			shader = _shaderRM->getValue(id);

		_shaderRM->addValue(shader, id);
		return shader;
	}

	Shader makeShader(const char* vertexCode, const char* fragmentCode, const char* id)
	{
		lost::Shader shader = nullptr;

		if (!_shaderRM->hasValue(id))
		{
			shader = new lost::_Shader();
			shader->buildShader(vertexCode, fragmentCode);
		}
		else
			shader = _shaderRM->getValue(id);

		_shaderRM->addValue(shader, id);
		return shader;
	}

	Shader getShader(const char* id)
	{
		return _shaderRM->getValue(id);
	}

	void unloadShader(const char* id)
	{
		_shaderRM->destroyValue(id);
	}

	void unloadShader(Shader& shader)
	{
		_shaderRM->destroyValueByValue(shader);
	}

	void forceUnloadShader(const char* id)
	{
		_shaderRM->forceDestroyValue(id);
	}

	void forceUnloadShader(Shader& shader)
	{
		_shaderRM->forceDestroyValueByValue(shader);
	}

#pragma endregion

#pragma region Mesh

	Mesh loadMesh(const char* objLoc, const char* id)
	{
		lost::Mesh mesh = nullptr;

		// If "id" is nullptr set it to the filename
		if (!id) id = objLoc;

		std::string loc = objLoc;

		if (!_meshRM->hasValue(id))
		{
			if (loc.substr(loc.size() - 3, 3) == "obj")
				mesh = _loadMeshOBJ(objLoc);
			else
				debugLog("Object file type not supported! \"" + loc + "\" Currently Lost only supports .obj", LOST_LOG_ERROR);
		}
		else
			mesh = _meshRM->getValue(id);

		_meshRM->addValue(mesh, id);
		return mesh;
	}

	Mesh makeMesh(MeshData& meshData, const char* id)
	{
		lost::Mesh mesh = nullptr;

		if (!_meshRM->hasValue(id))
		{
			mesh = new CompiledMeshData();

			// This works only becausee the Vertex class is essentially just a list of floats, in order:
			// Position, Texcoord, Color
			for (Vertex& vertex : meshData.verticies)
			{
				for (int i = 0; i < sizeof(vertex.data) / sizeof(float); i++)
					((CompiledMeshData*)mesh)->vectorData.push_back(vertex.data[i]);
			}
			((CompiledMeshData*)mesh)->indexData.insert(((CompiledMeshData*)mesh)->indexData.end(), meshData.indexArray.begin(), meshData.indexArray.end());

			// Copy material take over inducies
			((CompiledMeshData*)mesh)->materialSlotIndicies.insert(((CompiledMeshData*)mesh)->materialSlotIndicies.begin(), meshData.materialSlotIndicies.begin(), meshData.materialSlotIndicies.end());

			debugLog(std::string("Successfully created new mesh with ") + std::to_string(meshData.verticies.size()) + " verticies", LOST_LOG_SUCCESS);
		}
		else
			mesh = _meshRM->getValue(id);

		_meshRM->addValue(mesh, id);
		return mesh;
	}

	Mesh getMesh(const char* id)
	{
		return _meshRM->getValue(id);
	}

	void unloadMesh(const char* id)
	{
		_meshRM->destroyValue(id);
	}

	void unloadMesh(Mesh& mesh)
	{
		_meshRM->destroyValueByValue(mesh);
	}

	void forceUnloadMesh(const char* id)
	{
		_meshRM->forceDestroyValue(id);
	}

	void forceUnloadMesh(Mesh& mesh)
	{
		_meshRM->forceDestroyValueByValue(mesh);
	}

	Mesh _loadMeshOBJ(const char* objLoc)
	{

		// Load file
		std::string meshFile = loadfile(objLoc);

		std::vector<std::string> fileLines = split(meshFile, '\n');

#ifdef LOST_DEBUG_MODE
		int processingLine = 0;
		int fileLength = fileLines.size();
		int percentAcross = 0;
		if (fileLength > 10000)
			std::cout << "\x1B[38;2;100;150;255m[ Info ] Loading large object (\"" << std::string(objLoc) << "\")\x1B[0m |          |\b\b\b\b\b\b\b\b\b\b\b";
#endif

		std::unordered_map<std::string, int> vertexIndexMap;
		vertexIndexMap.reserve(fileLines.size());

		std::vector<Vec4> vertexData;
		std::vector<Vec3> normalData;
		std::vector<Vec3> textureData;
		vertexData.reserve(fileLines.size());
		normalData.reserve(fileLines.size());
		textureData.reserve(fileLines.size());

		MeshData meshData = {};

		for (std::string& line : fileLines)
		{
			std::vector<std::string> tokens = split(line, ' ');

#ifdef LOST_DEBUG_MODE
			if (fileLength > 10000)
			{
				processingLine++;

				if ((processingLine * 100 / fileLength) >= percentAcross + 1)
				{
					percentAcross++;
					if (percentAcross % 10 == 0)
					{
						if (percentAcross == 100)
							std::cout << "\b\xB0\xB2\b\b\b\b\b\b\b\b\b\b\xB2\xB1 DONE \xB1\xB2\n";
						else if (percentAcross <= 10)
							std::cout << "\xB2";
						else
							std::cout << "\b\xB0\xB2";
					}
				}
			}
#endif

			// All of these are done before the faces
			if (tokens[0] == "v")
			{
#ifdef LOST_DEBUG_MODE
				if (tokens.size() < 4 || tokens.size() > 5)
				{
					debugLog(std::string("Malformed .obj file, \"") + objLoc + "\" vertex with less than 3 or more than 4 floats", LOST_LOG_ERROR);
					Vec4 vertex = { 0.0f, 0.0f, 0.0f, 0.0f };
					vertexData.push_back(vertex);
					continue;
				}
#endif
				Vec4 vertex = { std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), tokens.size() == 5 ? std::stof(tokens[4]) : 1.0f };
				vertexData.push_back(vertex);
				continue;
			}

			if (tokens[0] == "vn")
			{
#ifdef LOST_DEBUG_MODE
				if (tokens.size() != 4)
				{
					debugLog(std::string("Malformed .obj file, \"") + objLoc + "\" normal with more or less than 3 floats", LOST_LOG_ERROR);
					Vec3 normal = { 0.0f, 0.0f, 0.0f };
					normalData.push_back(normal);
					continue;
				}
#endif
				Vec3 normal = { std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
				normalData.push_back(normal);
				continue;
			}

			if (tokens[0] == "vt")
			{
#ifdef LOST_DEBUG_MODE
				if (tokens.size() < 2 || tokens.size() > 4)
				{
					debugLog(std::string("Malformed .obj file, \"") + objLoc + "\" normal with less than 1 or more than 3 floats", LOST_LOG_ERROR);
					Vec3 texture = { 0.0f, 0.0f, 0.0f };
					textureData.push_back(texture);
					continue;
				}
#endif
				Vec3 texture = { std::stof(tokens[1]), tokens.size() >= 3 ? std::stof(tokens[2]) : 0.0f, tokens.size() >= 4 ? std::stof(tokens[3]) : 0.0f };
				textureData.push_back(texture);
				continue;
			}

			// Faces, turn arbitrary data into verticies
			if (tokens[0] == "f")
			{
#ifdef LOST_DEBUG_MODE
				if (tokens.size() != 4)
				{
					debugLog(std::string("Malformed .obj file, \"") + objLoc + "\" face with less than or more than 3 verticies\nMake sure mesh is triangulated!", LOST_LOG_ERROR);
					continue;
				}
#endif

				for (int i = 1; i < tokens.size(); i++)
				{
					std::vector<std::string> subTokens = split(tokens[i], '/');
					//IndexData indexData = { std::stoi(subTokens[0]) - 1, std::stoi(subTokens[1]) - 1, std::stoi(subTokens[2]) - 1 };

					if (vertexIndexMap.count(tokens[i]) == 0)
					{
						Vertex vert = {};

						vert.position = vertexData[std::stoi(subTokens[0]) - 1].xyz;
						vert.textureCoord = { textureData[std::stoi(subTokens[1]) - 1].x, 1.0f - textureData[std::stoi(subTokens[1]) - 1].y };
						vert.vertexColor = { 1.0f, 1.0f, 1.0f, 1.0f };
						vert.vertexNormal = normalData[std::stoi(subTokens[2]) - 1];

						vertexIndexMap[tokens[i]] = meshData.verticies.size();
						meshData.verticies.push_back(vert);
					}

					meshData.indexArray.push_back(vertexIndexMap[tokens[i]]);
				}

				continue;
			}

			// Materials
			if (tokens[0] == "usemtl")
			{
				meshData.materialSlotIndicies.push_back(meshData.indexArray.size());
			}
		}

		// Compile mesh data into usable mesh data
		CompiledMeshData* data = new CompiledMeshData();

		// This works only because the Vertex class is essentially just a list of floats, in order:
		// Position, Texcoord, Color
		for (Vertex& vertex : meshData.verticies)
		{
			for (int i = 0; i < sizeof(vertex.data) / sizeof(float); i++)
				data->vectorData.push_back(vertex.data[i]);
		}
		//newMeshData->vectorData.insert(newMeshData->vectorData.end(), meshData.verticies.begin(), meshData.verticies.end());
		data->indexData.insert(data->indexData.end(), meshData.indexArray.begin(), meshData.indexArray.end());
		//newMeshData->stride = sizeof(Vertex);

		// Copy material take over inducies
		data->materialSlotIndicies.insert(data->materialSlotIndicies.begin(), meshData.materialSlotIndicies.begin(), meshData.materialSlotIndicies.end());

		debugLog(std::string("Successfully created new mesh with ") + std::to_string(meshData.verticies.size()) + " verticies", LOST_LOG_SUCCESS);
		
		return data;
	}

#pragma endregion

#pragma region Font

	Font loadFont(const char* fontLoc, float fontHeight, const char* id)
	{
		lost::Font font = nullptr;

		// If "id" is nullptr set it to the filename
		if (!id) id = fontLoc;

		if (!_fontRM->hasValue(id))
		{
			font = _loadFontNoManager(fontLoc, fontHeight);
		}
		else
			font = _fontRM->getValue(id);

		_fontRM->addValue(font, id);
		return font;
	}

	Font getFont(const char* id)
	{
		return _fontRM->getValue(id);
	}

	void unloadFont(const char* id)
	{
		_fontRM->destroyValue(id);
	}

	void unloadFont(Font& font)
	{
		_fontRM->destroyValueByValue(font);
	}

	void forceUnloadFont(const char* id)
	{
		_fontRM->forceDestroyValue(id);
	}

	void forceUnloadFont(Font& font)
	{
		_fontRM->forceDestroyValueByValue(font);
	}

#pragma endregion
}