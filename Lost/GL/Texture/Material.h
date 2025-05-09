#pragma once
#include "../Shaders/Shader.h"
#include "Texture.h"
#include <vector>

enum DepthTestMode
{
	// Uses the depth test of the material given
	LOST_DEPTH_TEST_AUTO = 1,

	// Test never passes
	LOST_DEPTH_TEST_NEVER = GL_NEVER,

	// Test passes if depth is less than but not equal (Recommended)
	LOST_DEPTH_TEST_LESS = GL_LESS,

	// Test passes if depth is equal
	LOST_DEPTH_TEST_EQUAL = GL_EQUAL,

	// Test passes if depth is less than or equal
	LOST_DEPTH_TEST_LEQUAL = GL_LEQUAL,

	// Test passes if depth is greater than but not equal
	LOST_DEPTH_TEST_GREATER = GL_GREATER,

	// Test passes if depth is not equal
	LOST_DEPTH_TEST_NOTEQUAL = GL_NOTEQUAL,

	// Test passes if depth greater than or equal
	LOST_DEPTH_TEST_GEQUAL = GL_GEQUAL,

	// Test always passes
	LOST_DEPTH_TEST_ALWAYS = GL_ALWAYS
};

enum BackfaceCullingMode
{
	// Uses the backface cull mode of the material given
	LOST_CULL_AUTO = 1,

	// Disables backface culling
	LOST_CULL_NONE = 0,
	
	// Discards all front faces (Frontface culling)
	LOST_CULL_FRONT = GL_FRONT,

	// Discards all back faces (Backface culling)
	LOST_CULL_BACK = GL_BACK,
	
	// Discards all faces
	LOST_CULL_FRONT_AND_BACK = GL_FRONT_AND_BACK
};

enum ZSortMode
{
	// Batching, sorts in the most optimal way, does not work with transparency
	LOST_ZSORT_NORMAL,

	// Uses depth, sorts in an unoptimal way but allows for transparency
	LOST_ZSORT_DEPTH,

	// Disables sorting, preserving order
	LOST_ZSORT_NONE
};

namespace lost
{

	// Specifies a uniform that is set by the material
	struct MaterialUniform
	{
		std::string uniformID = "";
		unsigned int location = -1;

		void* data = nullptr; // Pointer of data
		unsigned int type = LOST_TYPE_ERROR; // Data type of the data
	};

	class _Material
	{
	public:
		_Material(Shader shader, std::vector<Texture> textures = {}, unsigned int renderQueue = 1000);
		~_Material();

		inline Shader getShader() const { return m_Shader; };

		inline unsigned int getQueueLevel() const { return m_QueueLevel; };
		inline void         setQueueLevel(unsigned int queueLevel) { m_QueueLevel = queueLevel; };

		inline unsigned int getDepthTestFunc() const { return m_DepthTestFunc; };
		inline void			setDepthTestFunc(unsigned int func) { m_DepthTestFunc = func; };

		inline bool getDepthWrite() const { return m_WriteDepth; };
		inline void setDepthWrite(bool writeDepth) { m_WriteDepth = writeDepth; };

		inline unsigned int getFaceCullMode() const { return m_FaceCullMode; };
		inline void			setFaceCullMode(unsigned int mode) { m_FaceCullMode = mode; };

		inline unsigned int getZSortMode() const { return m_ZSortMode; };
		inline void			setZSortMode(unsigned int mode) { m_ZSortMode = mode; };

		void	setTexture(const char* slotName, Texture texture);
		Texture getTexture(const char* slotName) const;
		Texture getTextureWithID(unsigned int slot) const;

		void bindTextures() const;
		void bindShader() const;

		inline bool hasMaterialUniforms() const { return !m_MaterialUniforms.empty(); };
		void setMaterialUniform(const char* uniformName, const void* data, unsigned int dataType);
		void deleteMaterialUniforms();
		inline const std::vector<MaterialUniform>& getMaterialUniforms() const { return m_MaterialUniforms; };
		void bindMaterialUniforms();

	private:
		unsigned int m_QueueLevel;

		// [!] TODO:
		unsigned int m_ZSortMode = LOST_ZSORT_NORMAL;

		unsigned int m_FaceCullMode = LOST_CULL_BACK;

		// The materials depth test function, this will be overriden by the renderer
		// if the render call specifies
		unsigned int m_DepthTestFunc = LOST_DEPTH_TEST_LESS;
		
		// Tell OpenGL if the depth of this material should be written to the depth buffer
		// This will be overriden by the renderer if the render call specifies
		bool m_WriteDepth = true;

		Shader m_Shader;
		std::vector<Texture> m_Textures;
		std::vector<MaterialUniform> m_MaterialUniforms;
	};

	// A reference to a material
	typedef _Material* Material;
	
	/// <summary>
	/// Will set a value in the material which when the material is used will set a uniform in the shader
	/// This is useful if the material needs to set specific values that aren't textures, like specular intensity and likewise.
	/// </summary>
	/// <param name="mat">The material to apply to</param>
	/// <param name="uniformName">The name of the uniform within the shader</param>
	/// <param name="data">The address of the data given (cast to void*)</param>
	/// <param name="dataType">The data type following LOST_TYPE_xxxx where x is the type</param>
	void setMaterialUniform(Material mat, const char* uniformName, const void* data, unsigned int dataType);
}