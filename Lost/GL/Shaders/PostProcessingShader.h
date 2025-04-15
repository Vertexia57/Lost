#pragma once

#include "Shader.h"

namespace lost
{
	// [!] TODO: Docs
	class _PostProcessingShader
	{
	public:
		_PostProcessingShader(Shader shader, void (*funcOverride)(_PostProcessingShader*) = nullptr);
		_PostProcessingShader(const std::vector<Shader>& shaders, void (*funcOverride)(_PostProcessingShader*) = nullptr);
		~_PostProcessingShader();

		// Takes in a function pointer to a function which does this shaders processing, more detail in docs
		// This function takes the form of "static void func(PostProcessingShader);"
		inline void setFunctionOverride(void (*funcOverride)(_PostProcessingShader*)) { m_FunctionOverride = funcOverride; };

		void run();

		Shader getShader(unsigned int index);

	private:
		std::vector<Shader> m_Shaders;

		void (*m_FunctionOverride)(_PostProcessingShader*); // Overrides the normal process for a post processing shader
	};

	typedef _PostProcessingShader* PostProcessingShader;

	void runPostProcessingShader(PostProcessingShader shader);
}