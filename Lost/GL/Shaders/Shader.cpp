#include "Shader.h"
#include <glad/glad.h>
#include <vector>
#include "../LostGL.h"
#include <set>
#include "ShaderCode.h"

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

namespace lost
{

	const static std::set<std::string> _BuiltInUniforms = {
		"resolution"
	};

	_Shader::_Shader()
	{
	}

	_Shader::_Shader(const char* vsDir, const char* fsDir)
	{
		loadShader(vsDir, fsDir);
	}

	_Shader::~_Shader()
	{
		glDeleteProgram(m_ShaderID);
	}

	void _Shader::loadShader(const char* vsDir, const char* fsDir)
	{
		setLogContext("Loading Shader");

		if (vsDir)
		{
			m_VSSourceLoc = vsDir;
			m_VSSource = loadfile(vsDir);
		}
		else
		{
			m_VSSourceLoc = "built-in";
			m_VSSource = _baseVSCode;
		}

		if (fsDir)
		{
			m_FSSourceLoc = fsDir;
			m_FSSource = loadfile(fsDir);
		}
		else
		{
			m_FSSourceLoc = "built-in";
			m_FSSource = _baseFSCode;
		}

		buildShader(m_VSSource.c_str(), m_FSSource.c_str());

		if (!m_Functional)
			log(std::string("Failed to load shader with VS at \"") + m_VSSourceLoc + "\" and FS at \"" + m_FSSourceLoc + "\"", LOST_LOG_ERROR);

		clearLogContext();
	}

	void _Shader::buildShader(const char* vs, const char* fs)
	{
		setLogContext("Building Shader");

		m_Functional = true;

		buildModule(vs, GL_VERTEX_SHADER);
		buildModule(fs, GL_FRAGMENT_SHADER);

		m_ShaderID = glCreateProgram();
		glAttachShader(m_ShaderID, m_VertID);
		glAttachShader(m_ShaderID, m_FragID);
		glLinkProgram(m_ShaderID);

#ifdef LOST_DEBUG_MODE
		// Check if the program succesfully linkes
		int successfullyLinked = 0;
		glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &successfullyLinked);
		if (successfullyLinked == 0)
		{
			const unsigned int bufferSize = 256;
			char buffer[bufferSize];
			int bufferLength = 0;
			glGetProgramInfoLog(m_ShaderID, bufferSize, &bufferLength, buffer);

			lost::log("Failed to link shaders, info:\n" + std::string(buffer), LOST_LOG_ERROR);
		}
#endif

		// They are no longer useful after glLinkProgram
		glDeleteShader(m_VertID);
		glDeleteShader(m_FragID);

		GLint prog = 0; // Get program currently being active
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		 
		glUseProgram(m_ShaderID); // Swap to this current shader program

#ifdef LOST_DEBUG_MODE
		std::vector<std::string> textureNames;
		std::vector<std::string> uniformNames;
		std::vector<std::string> uniformTypes;
#endif

		std::vector<std::string> splitString = split(fs, '\n');

		// [!] TODO: This is a very shoddy way of finding the uniforms and samplers, it will need a complete rework

		for (std::string& string : splitString)
		{
			if (string.substr(0, 8) == "uniform ")
			{
				// check if uniform is a sampler2D, and if it is hook it up to the texture map
				if (string.substr(8, 10) == "sampler2D ")
				{
					std::string textureName = split(split(string.substr(18, string.length() - 19), ';')[0], '[')[0];
					unsigned int id = m_TextureMap.size();
					glUniform1i(glGetUniformLocation(m_ShaderID, textureName.c_str()), id);
					m_TextureMap[textureName] = id;
#ifdef LOST_DEBUG_MODE
					textureNames.push_back(textureName);
#endif
				}
				else // Otherwise add the value as a uniform value
				{
					std::vector<std::string> uniformData = split(split(split(split(string.substr(8, string.length() - 9), ';')[0], '=')[0], '[')[0], ' ');
					bool isArray = split(split(split(string.substr(8, string.length() - 9), ';')[0], '=')[0], '[').size() > 1;
					for (int i = uniformData.size() - 1; i >= 0; i--)
					{
						if (uniformData[i].empty())
							uniformData.erase(uniformData.begin() + i);
					}
					std::string uniformName = uniformData.back();
					// Get the type and convert it to the associated enum, unknown values are considered structs (including errors)
					std::string uniformTypename = uniformData.at(uniformData.size() - 2);
					unsigned int uniformType = _UniformNameIDMap.count(uniformTypename) ? _UniformNameIDMap.at(uniformTypename) : LOST_TYPE_STRUCT;

					m_UniformMap[uniformName] = UniformData{ 
						(unsigned int)glGetUniformLocation(m_ShaderID, uniformName.c_str()), 
						uniformType,
						_BuiltInUniforms.find(uniformName) != _BuiltInUniforms.end(),
						isArray
					};
#ifdef LOST_DEBUG_MODE
					uniformNames.push_back(uniformName);
					uniformTypes.push_back(uniformTypename);
#endif
				}
			}
		}

		m_ResolutionUniformLoc = glGetUniformLocation(m_ShaderID, "resolution");

		glUseProgram(prog); // Change back to old program

#ifdef LOST_DEBUG_MODE
		debugLog("\n======[    Shader Created    ]======\n", LOST_LOG_NONE);
		debugLog(" - VS: \"" + (m_VSSourceLoc.empty() ? std::string("built-in") : m_VSSourceLoc) + "\"", LOST_LOG_NONE);
		debugLog(" - FS: \"" + (m_FSSourceLoc.empty() ? std::string("built-in") : m_FSSourceLoc) + "\"", LOST_LOG_NONE);
		debugLog(" - Texture Inputs for materials:", LOST_LOG_NONE);
		for (int i = 0; i < textureNames.size(); i++)
			debugLog("    - " + textureNames[i] + ", slot: " + std::to_string(i), LOST_LOG_NONE);
		debugLog(" - Uniform Inputs for materials: " + ((uniformNames.empty()) ? std::string("(none)") : std::string()), LOST_LOG_NONE);
		for (int i = 0; i < uniformNames.size(); i++)
		{
			bool builtIn = (_BuiltInUniforms.find(uniformNames[i]) != _BuiltInUniforms.end());
			debugLog("    - " + uniformNames[i] + ", type: " + uniformTypes[i] + (builtIn ? " (built in)" : ""), LOST_LOG_NONE);
		}
		debugLog("\n======[    Shader Created    ]======\n", LOST_LOG_NONE);
#endif

		clearLogContext();
	}

	void _Shader::reloadShader()
	{
		glDeleteProgram(m_ShaderID);

		m_ResolutionUniformLoc = -1;
		m_ShaderType = LOST_SHADER_TRANSPARENT;
		m_UniformMap = {};
		m_TextureMap = {};
		m_Functional = false;
		m_VertID = -1;
		m_FragID = -1;
		m_ShaderID = -1;

		loadShader(
			(m_VSSourceLoc.empty() || m_VSSourceLoc == "built-in") ? nullptr : m_VSSourceLoc.c_str(),
			(m_FSSourceLoc.empty() || m_FSSourceLoc == "built-in") ? nullptr : m_FSSourceLoc.c_str()
		);
	}

	const char* _Shader::getVertexDir() const
	{
		return m_VSSourceLoc.empty() ? nullptr : m_VSSourceLoc.c_str();
	}

	const char* _Shader::getFragmentDir() const
	{
		return m_FSSourceLoc.empty() ? nullptr : m_FSSourceLoc.c_str();
	}

	const char* _Shader::getVertexSource() const
	{
		return m_VSSource.empty() ? nullptr : m_VSSource.c_str();
	}

	const char* _Shader::getFragmentSource() const
	{
		return m_FSSource.empty() ? nullptr : m_FSSource.c_str();
	}

	void _Shader::bind()
	{
		glUseProgram(m_ShaderID);
		if (m_ResolutionUniformLoc != -1)
			glUniform2f(m_ResolutionUniformLoc, (float)getWidth(getCurrentWindow()), (float)getHeight(getCurrentWindow()));
	}

	unsigned int _Shader::getUniformLocation(const char* uniformName)
	{
		if (m_UniformMap.count(uniformName) != 0)
			return m_UniformMap.at(uniformName).location;

		unsigned int uniformLoc = glGetUniformLocation(m_ShaderID, uniformName);
		m_UniformMap[uniformName] = {};
		m_UniformMap[uniformName].location = uniformLoc;
		return uniformLoc;
	}

	unsigned int _Shader::getUniformType(const char* uniformName)
	{
		if (m_UniformMap.count(uniformName) != 0)
			return LOST_TYPE_ERROR; 

		return m_UniformMap[uniformName].type;
	}

	void _Shader::setUniform(void* dataAt, const char* uniformName, unsigned int count, unsigned int offset)
	{
		int uniformLoc = 0;

		// Just in case the preprocess didn't pick up on the uniform
		if (m_UniformMap.count(uniformName) != 0)
			uniformLoc = m_UniformMap[uniformName].location;
		else
		{
			uniformLoc = glGetUniformLocation(m_ShaderID, uniformName);
			if (uniformLoc == -1)
			{
				debugLog(std::string("Failed to set uniform value with name: ") + uniformName + ", Uniform doesn't exist", LOST_LOG_WARNING);
				return;
			}
			m_UniformMap[uniformName].location = uniformLoc;
		}

		// We need to make sure this shader is bound, we will store the current shader running so we can bind it again
		// just in case this was ran in the middle of a call
		int prog = 0;
		// [!] TODO: Replace this with a static global singleton that can be used instead of querying the OpenGL state
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

		if (prog != m_ShaderID)
			glUseProgram(m_ShaderID);

		switch (m_UniformMap[uniformName].type)
		{
		case LOST_TYPE_FLOAT:
			glUniform1fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC2:
			glUniform2fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC3:
			glUniform3fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC4:
			glUniform4fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_INT:
			glUniform1iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC2:
			glUniform2iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC3:
			glUniform3iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC4:
			glUniform4iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_UINT:
			glUniform1uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC2:
			glUniform2uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC3:
			glUniform3uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC4:
			glUniform4uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
			// [!] TODO: Doubles
		case LOST_TYPE_BOOL:
			glUniform1iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC2:
			glUniform2iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC3:
			glUniform3iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC4:
			glUniform4iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		default:
			debugLog("Tried to set uniform with invalid data type.", LOST_LOG_WARNING);
			break;
		}

		// Switch back to the old shader
		if (prog != m_ShaderID)
			glUseProgram(prog);
	}

	void _Shader::setUniform(void* dataAt, unsigned int uniformLoc, unsigned int type, unsigned int count, unsigned int offset)
	{
		// We need to make sure this shader is bound, we will store the current shader running so we can bind it again
		// just in case this was ran in the middle of a call
		int prog = 0;
		// [!] TODO: Replace this with a static global singleton that can be used instead of querying the OpenGL state
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

		if (prog != m_ShaderID)
			glUseProgram(m_ShaderID);

		switch (type)
		{
		case LOST_TYPE_FLOAT:
			glUniform1fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC2:
			glUniform2fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC3:
			glUniform3fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_VEC4:
			glUniform4fv(uniformLoc + offset, count, (float*)dataAt);
			break;
		case LOST_TYPE_INT:
			glUniform1iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC2:
			glUniform2iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC3:
			glUniform3iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_IVEC4:
			glUniform4iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_UINT:
			glUniform1uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC2:
			glUniform2uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC3:
			glUniform3uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
		case LOST_TYPE_UVEC4:
			glUniform4uiv(uniformLoc + offset, count, (unsigned int*)dataAt);
			break;
			// [!] TODO: Doubles
		case LOST_TYPE_BOOL:
			glUniform1iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC2:
			glUniform2iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC3:
			glUniform3iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		case LOST_TYPE_BVEC4:
			glUniform4iv(uniformLoc + offset, count, (int*)dataAt);
			break;
		default:
			debugLog("Tried to set uniform with invalid data type.", LOST_LOG_WARNING);
			break;
		}

		// Switch back to the old shader
		if (prog != m_ShaderID)
			glUseProgram(prog);
	}

	void _Shader::buildModule(const char* code, uint32_t moduleType)
	{
		// Compile module
		uint32_t moduleID = glCreateShader(moduleType);
		glShaderSource(moduleID, 1, &code, NULL);
		glCompileShader(moduleID);

		// Check if module compiled
		int compileStatus;
		glGetShaderiv(moduleID, GL_COMPILE_STATUS, &compileStatus); // Load the compile status into compileStatus
		if (!compileStatus)
		{
			char errorBuffer[1024];
			glGetShaderInfoLog(moduleID, 1024, NULL, errorBuffer); // Load the error into errorBuffer

			log(std::string("Failed to compile shader module!\n") + errorBuffer, LOST_LOG_ERROR);
			m_Functional = false;
		}

		// Set proper module references
		if (moduleType == GL_VERTEX_SHADER)
			m_VertID = moduleID;
		else
			m_FragID = moduleID;
	}

	void setUniform(Shader shader, void* dataAt, const char* uniformName, unsigned int count, unsigned int offset)
	{
		shader->setUniform(dataAt, uniformName, count, offset);
	}

	void setUniform(Shader shader, void* dataAt, unsigned int uniformLoc, unsigned int type, unsigned int count, unsigned int offset)
	{
		shader->setUniform(dataAt, uniformLoc, type, count, offset);
	}

	unsigned int getUniformLocation(Shader shader, const char* uniformName)
	{
		return shader->getUniformLocation(uniformName);
	}

	unsigned int getUniformType(Shader shader, const char* uniformName)
	{
		return shader->getUniformType(uniformName);
	}
}