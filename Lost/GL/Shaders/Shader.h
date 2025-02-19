#pragma once

#include "../../State.h"
#include "../../Log.h"
#include "../../FileIO.h"
#include <string>
#include <map>

enum MaterialType
{
	LOST_SHADER_OPAQUE = 1000,
	LOST_SHADER_TRANSPARENT = 2000
};

enum UniformDataType
{
	LOST_FLOAT,
	LOST_VEC2,
	LOST_VEC3,
	LOST_VEC4,
	LOST_INT
};

namespace lost
{
	const static std::map<std::string, unsigned int> _UniformNameIDMap = {
		{ "float", LOST_FLOAT },
		{ "vec2", LOST_VEC2 },
		{ "vec3", LOST_VEC3 },
		{ "vec4", LOST_VEC4 },
		{ "int", LOST_INT }
	};

	const static std::vector<std::string> _UniformIDName = {
		"float",
		"vec2",
		"vec3",
		"vec4",
		"int"
	};

	class _Shader
	{
	private:

		struct UniformData
		{
			unsigned int type = 0;
			unsigned int location = 0;
			bool isEngine = false;
		};

	public:
		// Default constructor, does nothing
		_Shader();
		// Builds shader using vsDir as the vertex shader directory and fsDir as the fragment shader directory
		_Shader(const char* vsDir, const char* fsDir);
		~_Shader();

		// Builds shader using vsDir as the vertex shader directory and fsDir as the fragment shader directory
		void loadShader(const char* vsDir, const char* fsDir);
		// Builds shader using vs and fs and the code for the vertex and fragment shader respectively
		void buildShader(const char* vs, const char* fs);

		void bind();

		inline const std::map<std::string, unsigned int>& getTextureNameMap() const { return m_TextureMap; };

		void setUniform(void* dataAt, const char* uniformName);

	private:

		void buildModule(const char* code, uint32_t moduleType);

		unsigned int m_ResolutionUniformLoc = -1;

		unsigned int m_ShaderType = LOST_SHADER_TRANSPARENT;

		std::map<std::string, UniformData> m_UniformMap;

		std::map<std::string, unsigned int> m_TextureMap;

		bool m_Functional = false;

		std::string m_VSSourceLoc;
		std::string m_FSSourceLoc;
		std::string m_VSSource;
		std::string m_FSSource;

		uint32_t m_VertID = -1;
		uint32_t m_FragID = -1;

		uint32_t m_ShaderID = -1;
	};

	// A reference to a shader
	typedef _Shader* Shader;

	class PPShader
	{
	public:
		PPShader();
		PPShader(Shader _shader);

		void bind();

		// Sets the bind override for the post processing shader
		// This lets the user add extra code to the post processing shader like texture resizing
		inline void setShaderBindOverride(void(*override)(PPShader*)) { m_ShaderBindOverride = override; };

		Shader shader = nullptr;
	private:
		void(*m_ShaderBindOverride)(PPShader*) = nullptr;
	};
}
