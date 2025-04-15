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

// [!] TODO: Support doubles, shorts and half floats
enum UniformDataType
{
	LOST_TYPE_FLOAT,
	LOST_TYPE_VEC2,
	LOST_TYPE_VEC3,
	LOST_TYPE_VEC4,
	LOST_TYPE_INT,
	LOST_TYPE_IVEC2,
	LOST_TYPE_IVEC3,
	LOST_TYPE_IVEC4,
	LOST_TYPE_UINT,
	LOST_TYPE_UVEC2,
	LOST_TYPE_UVEC3,
	LOST_TYPE_UVEC4,
	LOST_TYPE_DOUBLE,
	LOST_TYPE_DVEC2,
	LOST_TYPE_DVEC3,
	LOST_TYPE_DVEC4,
	LOST_TYPE_BOOL,
	LOST_TYPE_BVEC2,
	LOST_TYPE_BVEC3,
	LOST_TYPE_BVEC4,
	LOST_TYPE_STRUCT,
	LOST_TYPE_ERROR = -1
};

namespace lost
{

	const static std::map<std::string, unsigned int> _UniformNameIDMap = {
		{ "float",	LOST_TYPE_FLOAT  },
		{ "vec2",	LOST_TYPE_VEC2   },
		{ "vec3",	LOST_TYPE_VEC3   },
		{ "vec4",	LOST_TYPE_VEC4   },
		{ "int",	LOST_TYPE_INT    },
		{ "ivec2",	LOST_TYPE_IVEC2  },
		{ "ivec3",	LOST_TYPE_IVEC3  },
		{ "ivec4",	LOST_TYPE_IVEC4  },
		{ "uint",	LOST_TYPE_UINT   },
		{ "uvec2",	LOST_TYPE_UVEC2  },
		{ "uvec3",	LOST_TYPE_UVEC3  },
		{ "uvec4",	LOST_TYPE_UVEC4  },
		{ "double",	LOST_TYPE_DOUBLE },
		{ "dvec2",	LOST_TYPE_DVEC2  },
		{ "dvec3",	LOST_TYPE_DVEC3  },
		{ "dvec4",	LOST_TYPE_DVEC4  },
		{ "bool",	LOST_TYPE_BOOL   },
		{ "bvec2",	LOST_TYPE_BVEC2  },
		{ "bvec3",	LOST_TYPE_BVEC3  },
		{ "bvec4",	LOST_TYPE_BVEC4  }
	};

	const static std::vector<std::string> _UniformIDName = {
		"float",
		"vec2",
		"vec3",
		"vec4",
		"int",
		"ivec2",
		"ivec3",
		"ivec4",
		"uint",
		"uvec2",
		"uvec3",
		"uvec4",
		"double",
		"dvec2",
		"dvec3",
		"dvec4",
		"bool",
		"bvec2",
		"bvec3",
		"bvec4",
		"struct"
	};

	class _Shader
	{
	public:
		struct UniformData
		{
			unsigned int location = 0;
			unsigned int type = 0;
			bool isEngine = false; // Specifies if the uniform is inbuilt
			bool isArray = false;
		};

		// Default constructor, does nothing
		_Shader();
		// Builds shader using vsDir as the vertex shader directory and fsDir as the fragment shader directory
		_Shader(const char* vsDir, const char* fsDir);
		~_Shader();

		// Builds shader using vsDir as the vertex shader directory and fsDir as the fragment shader directory
		void loadShader(const char* vsDir, const char* fsDir);
		// Builds shader using vs and fs and the code for the vertex and fragment shader respectively
		void buildShader(const char* vs, const char* fs);
		// Reloads the shader, using the stored values within the shader
		void reloadShader();

		const char* getVertexDir() const;
		const char* getFragmentDir() const;

		const char* getVertexSource() const;
		const char* getFragmentSource() const;

		void bind();

		inline const std::map<std::string, unsigned int>& getTextureNameMap() const { return m_TextureMap; };
		inline const std::map<std::string, UniformData>& getUniformNameMap() const { return m_UniformMap; };

		unsigned int getUniformLocation(const char* uniformName);
		unsigned int getUniformType(const char* uniformName);

		void setUniform(void* dataAt, const char* uniformName, unsigned int count = 1, unsigned int offset = 0);
		void setUniform(void* dataAt, unsigned int uniformLoc, unsigned int type, unsigned int count = 1, unsigned int offset = 0);

		inline unsigned int getShaderID() const { return m_ShaderID; };
	private:

		void buildModule(const char* code, uint32_t moduleType);

		unsigned int m_ResolutionUniformLoc = -1;

		unsigned int m_ShaderType = LOST_SHADER_TRANSPARENT;

		std::map<std::string, UniformData> m_UniformMap = {};

		std::map<std::string, unsigned int> m_TextureMap = {};

		bool m_Functional = false;

		std::string m_VSSourceLoc = "";
		std::string m_FSSourceLoc = "";
		std::string m_VSSource = "";
		std::string m_FSSource = "";

		uint32_t m_VertID = -1;
		uint32_t m_FragID = -1;

		uint32_t m_ShaderID = -1;
	};

	// A reference to a shader
	typedef _Shader* Shader;

	/// <summary>
	/// Set a uniform in the shader given. Automatically caches the uniform names and their locations.
	/// </summary>
	/// <param name="shader">The shader to set the uniform on</param>
	/// <param name="dataAt">A void* to the location of the data to set the uniform to</param>
	/// <param name="uniformName">The name of the uniform to set</param>
	/// <param name="count">If working with arrays, the amount of indices to set</param>
	/// <param name="offset">If working with arrays, the offset in the array to start setting at</param>
	void setUniform(Shader shader, void* dataAt, const char* uniformName, unsigned int count = 1, unsigned int offset = 0);

	/// <summary>
	/// Set a uniform in the shader given.
	/// </summary>
	/// <param name="shader">The shader to set the uniform on</param>
	/// <param name="dataAt">A void* to the location of the data to set the uniform to</param>
	/// <param name="uniformLoc">The location in the shader the uniform is at, use lost::getUniformLocation() to find this value</param>
	/// <param name="type">The type of the data the uniform takes, Eg. LOST_TYPE_VEC4, does not accept GL types</param>
	/// <param name="count">If working with arrays, the amount of indices to set</param>
	/// <param name="offset">If working with arrays, the offset in the array to start setting at</param>
	void setUniform(Shader shader, void* dataAt, unsigned int uniformLoc, unsigned int type, unsigned int count = 1, unsigned int offset = 0);

	// Returns the location of the uniform in the shader given
	unsigned int getUniformLocation(Shader shader, const char* uniformName);
	// Returns the type of the uniform in the shader given
	unsigned int getUniformType(Shader shader, const char* uniformName);
}
