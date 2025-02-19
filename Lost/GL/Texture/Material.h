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

namespace lost
{

	class _Material
	{
	public:
		_Material(Shader shader, std::vector<Texture> textures = {}, unsigned int renderQueue = 1000);
		~_Material();

		inline Shader getShader() const { return m_Shader; };
		inline unsigned int getQueueLevel() const { return m_QueueLevel; };

		inline unsigned int getDepthTestFunc() const { return m_DepthTestFunc; };
		inline void			setDepthTestFunc(unsigned int func) { m_DepthTestFunc = func; };
		inline bool getDepthWrite() const { return m_WriteDepth; };
		inline void setDepthWrite(bool writeDepth) { m_WriteDepth = writeDepth; };

		void setTexture(const char* slotName, Texture texture);

		void bindTextures() const;
		void bindShader() const;

	private:
		unsigned int m_QueueLevel;

		unsigned int m_DepthTestFunc = LOST_DEPTH_TEST_LESS;
		bool m_WriteDepth = true;

		Shader m_Shader;
		std::vector<Texture> m_Textures;
	};

	// A reference to a material
	typedef _Material* Material;

}