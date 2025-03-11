#pragma once
#include "../../ResourceManager.h"
#include "../Texture/Texture.h"
#include "../Texture/Material.h"
#include "../Shaders/Shader.h"
#include "../Mesh/Mesh.h"
#include "../Text/Text.h"

namespace lost
{

	extern ResourceManager<Texture>* _textureRM;
	extern ResourceManager<Material>* _materialRM;
	extern ResourceManager<Shader>* _shaderRM;
	extern ResourceManager<Mesh>* _meshRM;
	extern ResourceManager<Font>* _fontRM;

	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	extern void _initRMs();
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	extern void _destroyRMs();

	Texture loadTexture(const char* fileLocation, const char* id = nullptr);
	Texture makeTexture(const char* data, int width, int height, const char* id, unsigned int format = LOST_FORMAT_RGBA);
	Texture makeTexture(unsigned int openGLTexture, const char* id);
	Texture getTexture(const char* id);
	void    unloadTexture(const char* id);
	void    unloadTexture(Texture texture);
	void    forceUnloadTexture(const char* id);
	void    forceUnloadTexture(Texture texture);

	// Returns the name of the texture given
	// NOTE: This is only used inside of the Lost engine, though it is static and has no effect
	const char* _getTextureID(Texture texture);

	// Takes the list of textures as the input into the shaders texture slots
	// If shader is not set, uses the default shader. This shader changes based on which renderer is being used.
	// RenderQueue is how it is ordered in rendering, lower is first, higher is last
	Material makeMaterial(std::vector<Texture> textures, const char* id, Shader shader = nullptr, unsigned int renderQueue = LOST_SHADER_OPAQUE);
	Material getMateral(const char* id);
	std::vector<Material> loadMaterialsFromOBJMTL(const char* objFile);
	void     destroyMaterial(const char* id);
	void     destroyMaterial(Material material);
	void     forceDestroyMaterial(const char* id);
	void     forceDestroyMaterial(Material material);

	// When either shader location is nullptr, loads the default module for that pipeline
	Shader loadShader(const char* vertexLoc, const char* fragmentLoc, const char* id);
	Shader makeShader(const char* vertexCode, const char* fragmentCode, const char* id);
	Shader getShader(const char* id);
	void   unloadShader(const char* id);
	void   unloadShader(Shader& shader);
	void   forceUnloadShader(const char* id);
	void   forceUnloadShader(Shader& shader);

	// Returns the name of the shader given
	// NOTE: This is only used inside of the Lost engine, though it is static and has no effect
	const char* _getShaderID(Shader shader);

	Mesh loadMesh(const char* objLoc, const char* id = nullptr);
	Mesh makeMesh(MeshData& meshData, const char* id);
	Mesh getMesh(const char* id);
	void unloadMesh(const char* id);
	void unloadMesh(Mesh& obj);
	void forceUnloadMesh(const char* id);
	void forceUnloadMesh(Mesh& obj);

	// Mesh Load Functions
	Mesh _loadMeshOBJ(const char* objLoc);

	// Font Load Functions
	Font loadFont(const char* fontLoc, float fontHeight, const char* id = nullptr);
	Font getFont(const char* id);
	void unloadFont(const char* id);
	void unloadFont(Font& font);
	void forceUnloadFont(const char* id);
	void forceUnloadFont(Font& font);
}