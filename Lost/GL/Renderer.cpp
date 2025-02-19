#include "Renderer.h"
#include "LostGL.h"
#include <glad/glad.h>

#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <iostream>

#include "../DeltaTime.h"

// ImGui setup, only active if necessary
#ifndef IMGUI_DISABLE
#include "../lostImGui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#endif

namespace lost
{

	Renderer* _renderer = nullptr;

#pragma region RendererBase
	Renderer::Renderer()
	{
		m_CurrentPPShader = lost::PPShader(lost::_defaultShader);
	}

	Renderer::~Renderer()
	{
		for (RenderPass renderPass : m_MainRenderPasses)
			delete renderPass;

		glDeleteVertexArrays(VAOs.size(), VAOs.data());
		glDeleteVertexArrays(1, &m_EmptyVAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &MBO);
		glDeleteBuffers(1, &EBO);
	}

	void Renderer::setRenderMode(unsigned int renderMode)
	{
		m_RenderMode = renderMode;
		switch (m_RenderMode)
		{
		case LOST_RENDER_MODE_INSTANT:
		case LOST_RENDER_MODE_AUTO_QUEUE:
			renderInstanceQueue();
			break;
		case LOST_RENDER_MODE_QUEUE:
			break;
		default:
			debugLog("Render mode enum out of bounds, reverting to LOST_RENDER_MODE_AUTO_QUEUE", LOST_LOG_WARNING);
			m_RenderMode = LOST_RENDER_MODE_AUTO_QUEUE;
			break;
		}
	}

	void Renderer::generateBufferObjects()
	{
		glGenVertexArrays(1, &m_EmptyVAO);
		glBindVertexArray(m_EmptyVAO);

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &MBO);
		glGenBuffers(1, &EBO);

		bindBufferDescriptions();
	}

	void Renderer::resizeFrameBuffers(int windowID, int width, int height)
	{
		m_MainRenderPasses[windowID]->resize(width, height, getWindow(windowID));
	}

	void Renderer::setPostProcessingShader(PPShader shader)
	{
		m_CurrentPPShader = shader;
	}

	// Empty for overloading
	void Renderer::addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
	}
	void Renderer::addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
	}
	void Renderer::renderInstanceQueue()
	{
	}
	void Renderer::renderMesh(Mesh mesh, glm::mat4x4& transform)
	{
	}
	
	void Renderer::startRender()
	{
		m_MainRenderPasses[getCurrentWindowID()]->bind();

		for (int i = 0; i < m_MainRenderPasses[getCurrentWindowID()]->storedBuffers.size(); i++)
		{
			const RenderBufferData& renderBuffer = m_MainRenderPasses[getCurrentWindowID()]->storedBuffers[i];
			glClearBufferfv(GL_COLOR, i, renderBuffer.defaultColor.v);
		}

#ifndef IMGUI_DISABLE
		if (m_UsingImGui && getCurrentWindow()->_hasImGui)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
#endif
	}

	void Renderer::finalize()
	{
#ifndef IMGUI_DISABLE
		if (m_UsingImGui && getCurrentWindow()->_hasImGui)
		{
			//ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			//lost::pushWindow();
			//ImGui::UpdatePlatformWindows();
			//ImGui::RenderPlatformWindowsDefault();
			//lost::popWindow();
		}
#endif
	}

	void Renderer::generateNewVAO()
	{
		pushWindow();

		// Get the newly made context
		const std::vector<Window>& contexts = getWindows();
		size_t latestIndex = contexts.size() - 1;
		_setWindow(latestIndex);

		VAOs.push_back(0);

		// Generate VAO inside of this context
		glDeleteVertexArrays(VAOs.size(),VAOs.data());
		glGenVertexArrays(VAOs.size(), VAOs.data());
		for (int i = 0; i < VAOs.size(); i++)
		{
			glBindVertexArray(VAOs[i]);
			bindBufferDescriptions();
			glBindVertexArray(0);
		}

		debugLog("Generated new VAO for new context", LOST_LOG_INFO);

		m_MainRenderPasses.push_back(new lost::_RenderPass(getWidth(getWindow(latestIndex)), getHeight(getWindow(latestIndex)), getLostState().currentBuffers, contexts[latestIndex]));

		popWindow();
	}

	void Renderer::regenerateVAO(unsigned int id)
	{
		pushWindow();

		// Get the newly made context
		_setWindow(id);

		VAOs[id] = 0;

		// Generate VAO inside of this context
		glDeleteVertexArrays(1, &VAOs[id]);
		glGenVertexArrays(1, &VAOs[id]);
		glBindVertexArray(VAOs[id]);

		bindBufferDescriptions();

		debugLog("Regenerated VAO for context with id " + std::to_string(id), LOST_LOG_INFO);

		popWindow();
	}

	void Renderer::destroyVAO(unsigned int contextIndex)
	{
		delete m_MainRenderPasses[contextIndex];
		m_MainRenderPasses.erase(m_MainRenderPasses.begin() + contextIndex);

		glDeleteVertexArrays(1, &VAOs[contextIndex]);
		VAOs.erase(VAOs.begin() + contextIndex);
	}

	void Renderer::bindBufferDescriptions()
	{
		// Bind vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// [ VERTEX BUFFER BOUND ]

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// TexCoord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1); 

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// Normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		// Bind matrix buffer mat4x4's are stored as 4 * vec4's
		glBindBuffer(GL_ARRAY_BUFFER, MBO);
		glVertexAttribPointer(4,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, 0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(4  * sizeof(float)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(6,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(8  * sizeof(float)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(7,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(7);

		glVertexAttribPointer(8,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(16 * sizeof(float)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(9,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(20 * sizeof(float)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(24 * sizeof(float)));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(28 * sizeof(float)));
		glEnableVertexAttribArray(11);

		glVertexAttribDivisor(4, 1); // Make it so this input only updates once per INSTANCE rather than per vertex
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		glVertexAttribDivisor(8,  1);
		glVertexAttribDivisor(9,  1);
		glVertexAttribDivisor(10, 1);
		glVertexAttribDivisor(11, 1);

		// Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	}

	void Renderer::fillWindow(Color color)
	{
		color.normalize();
		glClearBufferfv(GL_COLOR, 0, color.v);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

#pragma endregion

#pragma region 2DRenderer
	Renderer2D::Renderer2D()
		: Renderer()
	{
		debugLog("Renderer set to 2D Mode, to use 3D put LOST_RENDER_3D in lost::init()", LOST_LOG_INFO);
	}

	Renderer2D::~Renderer2D()
	{
	}

	void Renderer2D::addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
		if (m_RenderMode == LOST_RENDER_MODE_INSTANT)
		{
			//renderMesh(mesh, transform);
			return;
		}

		// Active during LOST_RENDER_MODE_QUEUE and LOST_RENDER_MODE_AUTO_QUEUE
		// Fulfils the rule of "A non identical mesh is rendered

		if (m_CurrentMeshID == nullptr)
			m_CurrentMeshID = mesh;

		m_TransformArray.push_back(mvpTransform);
		if (m_CurrentMeshID != mesh)
		{
			renderInstanceQueue();
			m_CurrentMeshID = mesh;
		}
	}

	void Renderer2D::addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
	}

	void Renderer2D::renderInstanceQueue()
	{
		if (!m_TransformArray.empty())
		{
			// Vertex Array Object
			glBindVertexArray(VAOs[getCurrentWindowID()]);

			// Vertex Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)m_CurrentMeshID)->vectorData.size() * sizeof(float), ((CompiledMeshData*)m_CurrentMeshID)->vectorData.data(), GL_STATIC_DRAW);
			// Matrix Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, MBO);
			glBufferData(GL_ARRAY_BUFFER, m_TransformArray.size() * sizeof(glm::mat4x4), m_TransformArray.data(), GL_STATIC_DRAW);
			// Element Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((CompiledMeshData*)m_CurrentMeshID)->indexData.size() * sizeof(int), ((CompiledMeshData*)m_CurrentMeshID)->indexData.data(), GL_STATIC_DRAW);

			glDrawElementsInstanced(GL_TRIANGLES, ((CompiledMeshData*)m_CurrentMeshID)->indexData.size(), GL_UNSIGNED_INT, 0, m_TransformArray.size());

			m_TransformArray.clear();
			m_CurrentMeshID = nullptr;
		}

		m_RawMeshes.clear();
	}

	void Renderer2D::finalize()
	{
		glm::mat4x4 transform = glm::mat4x4(
			getWidth(), 0.0f, 0.0f, 0.0f,
			0.0f, getHeight(), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		glBindVertexArray(VAOs[getCurrentWindowID()]);

		// Vertex Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->vectorData.size() * sizeof(float), ((CompiledMeshData*)standardQuad)->vectorData.data(), GL_STATIC_DRAW);
		// Matrix Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, MBO);
		glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4x4), &transform, GL_STATIC_DRAW);
		// Element Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->indexData.size() * sizeof(int), ((CompiledMeshData*)standardQuad)->indexData.data(), GL_STATIC_DRAW);

		glBindTexture(GL_TEXTURE_2D, m_MainRenderPasses[getCurrentWindowID()]->textures[0]);

		glDrawElementsInstanced(GL_TRIANGLES, ((CompiledMeshData*)standardQuad)->indexData.size(), GL_UNSIGNED_INT, 0, 1);

		Renderer::finalize();
	}

	void Renderer2D::renderMesh(Mesh mesh, glm::mat4x4& transform)
	{
#ifdef LOST_DEBUG_MODE
		debugLogIf((mesh == nullptr), "Mesh was null, this may be because it was destroyed or was never initialized in the firstplace", LOST_LOG_WARNING);

		if (mesh != nullptr)
		{
			glBindVertexArray(VAOs[getCurrentWindowID()]);
			glDrawElements(GL_TRIANGLES, ((CompiledMeshData*)mesh)->indexData.size(), GL_UNSIGNED_INT, 0);
		}
#else
		glBindVertexArray(VAOs[getCurrentWindowID()]);
		glDrawElements(GL_TRIANGLES, ((CompiledMeshData*)mesh)->indexData.size(), GL_UNSIGNED_INT, 0);
#endif
	}
#pragma endregion

#pragma region 3DRenderer
	Renderer3D::Renderer3D()
		: Renderer()
	{
		debugLog("Renderer set to 3D Mode, to use 2D put LOST_RENDER_2D in lost::init() or leave it empty", LOST_LOG_INFO);
	}

	Renderer3D::~Renderer3D()
	{
	}

	void Renderer3D::addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
		if (m_RenderMode == LOST_RENDER_MODE_INSTANT)
		{
			// [!] TODO: Make this work with the material system
			//renderMesh(mesh, transform);
			return;
		}

		for (int i = 0; i < materials.size(); i++)
		{
			MeshRenderData renderData = {};

			renderData.mesh = mesh;
			renderData.material = materials[i];
			renderData.startIndex = ((CompiledMeshData*)mesh)->materialSlotIndicies[i];

			// Gets the amount of indicies until the next material slot, going to the end of the index
			// list if there are no more materials
			renderData.indicies = (i + 1 < ((CompiledMeshData*)mesh)->materialSlotIndicies.size()) ?
				((CompiledMeshData*)mesh)->materialSlotIndicies[i + 1] - renderData.startIndex :
				((CompiledMeshData*)mesh)->indexData.size() - renderData.startIndex;

			renderData.depthVal = 0; // [!] TODO
			renderData.mvpTransform = mvpTransform;
			renderData.modelTransform = modelTransform;
			renderData.depthMode = (depthTestFuncOverride == LOST_DEPTH_TEST_AUTO ? materials[i]->getDepthTestFunc() : depthTestFuncOverride);
			renderData.depthWrite = depthWrite ? materials[i]->getDepthWrite() : false;

			m_MainRenderData.push_back(renderData);
		}

		// Active during LOST_RENDER_MODE_QUEUE and LOST_RENDER_MODE_AUTO_QUEUE
		// Fulfils the rule of "A non identical mesh is rendered
	}

	void Renderer3D::addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
		CompiledMeshData* newMesh = new CompiledMeshData{
			meshData.vectorData,
			meshData.materialSlotIndicies,
			meshData.indexData
		};
		m_RawMeshes.push_back(newMesh);

		addMeshToQueue(newMesh, materials, mvpTransform, modelTransform, depthTestFuncOverride, depthWrite);
	}

	void Renderer3D::renderInstanceQueue()
	{
		if (!m_MainRenderData.empty())
		{
			// Needs to be stable to preserve the order of depth tested meshes
			std::stable_sort(m_MainRenderData.begin(), m_MainRenderData.end(), &Renderer3D::meshSortFunc);

			Mesh         currentMesh          = m_MainRenderData[0].mesh;
			Material     currentMaterial      = m_MainRenderData[0].material;
			unsigned int currentIndexOffset   = m_MainRenderData[0].startIndex;
			unsigned int currentIndexCount    = m_MainRenderData[0].indicies;
			unsigned int currentDepthTestFunc = m_MainRenderData[0].depthMode;
			bool         currentDepthWrite    = m_MainRenderData[0].depthWrite;
			
			// Setup first meshes material and vertex data
			currentMaterial->bindShader();
			currentMaterial->bindTextures();

			// Set the depth test function to the first one
			glDepthMask(currentDepthWrite);
			glDepthFunc(currentDepthTestFunc);

			// Vertex Array Object
			glBindVertexArray(VAOs[getCurrentWindowID()]);
			// Vertex Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vectorData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vectorData.data(), GL_STATIC_DRAW);

			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vectorData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vectorData.data(), GL_STATIC_DRAW); // [!] TODO: WTF?

			// Create transform list, reserving the maximum size that could occur
			std::vector<glm::mat4x4> transforms;
			transforms.reserve(m_MainRenderData.size() * 2);

			for (int i = 0; i < m_MainRenderData.size(); i++)
			{
				MeshRenderData& renderData = m_MainRenderData[i];

				if (renderData.mesh != currentMesh || renderData.material != currentMaterial || currentIndexOffset != renderData.startIndex || currentDepthTestFunc != renderData.depthMode || currentDepthWrite != renderData.depthWrite)
				{
					// Render all matching meshes with instancing

					// Matrix Buffer Data
					glBindBuffer(GL_ARRAY_BUFFER, MBO);
					glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4x4), transforms.data(), GL_STATIC_DRAW);
					// Element Buffer Data
					glBindBuffer(GL_ARRAY_BUFFER, EBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, currentIndexCount * sizeof(int), ((CompiledMeshData*)currentMesh)->indexData.data() + currentIndexOffset, GL_STATIC_DRAW);

					glDrawElementsInstanced(GL_TRIANGLES, currentIndexCount, GL_UNSIGNED_INT, 0, transforms.size() / 2);

					// Recreate transform list, reserving the maximum size that could occur
					transforms.clear();
					transforms.reserve(2 * (m_MainRenderData.size() - i));

					currentIndexOffset = renderData.startIndex;
					currentIndexCount = renderData.indicies;

					if (renderData.depthMode != currentDepthTestFunc)
					{
						currentDepthTestFunc = renderData.depthMode;
						glDepthFunc(currentDepthTestFunc);
					}

					if (renderData.depthWrite != currentDepthWrite)
					{
						currentDepthWrite = renderData.depthWrite;
						glDepthMask(currentDepthWrite);
					}

					// Update mesh vertex data if necessary
					if (renderData.mesh != currentMesh)
					{
						currentMesh = renderData.mesh;

						// Vertex Buffer Data
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vectorData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vectorData.data(), GL_STATIC_DRAW);

					}

					// Update mesh material if necessary
					if (renderData.material != currentMaterial)
					{
						if (renderData.material->getShader() != currentMaterial->getShader())
							renderData.material->bindShader();

						currentMaterial = renderData.material;
						currentMaterial->bindTextures();
					}

				}

				transforms.push_back(renderData.mvpTransform);
				transforms.push_back(renderData.modelTransform);
				if (i == 0)
				{
					glBindBuffer(GL_ARRAY_BUFFER, MBO);
					glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4x4), transforms.data(), GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, EBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, currentIndexCount * sizeof(int), ((CompiledMeshData*)currentMesh)->indexData.data() + currentIndexOffset, GL_STATIC_DRAW);
				}

			}

			// Render the final batch, as it's not included in the loop
			
			// Matrix Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, MBO);
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4x4), transforms.data(), GL_STATIC_DRAW);
			// Element Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MainRenderData[m_MainRenderData.size() - 1].indicies * sizeof(int), ((CompiledMeshData*)m_MainRenderData[m_MainRenderData.size() - 1].mesh)->indexData.data() + m_MainRenderData[m_MainRenderData.size() - 1].startIndex, GL_STATIC_DRAW);
			
			glDrawElementsInstanced(GL_TRIANGLES, m_MainRenderData[m_MainRenderData.size() - 1].indicies, GL_UNSIGNED_INT, 0, transforms.size());

			glDepthMask(true);
			glDepthFunc(LOST_DEPTH_TEST_LESS);
		}

		m_MainRenderData.clear();

		for (CompiledMeshData* mesh : m_RawMeshes)
			delete mesh;
		m_RawMeshes.clear();
	}

	void Renderer3D::finalize()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Bind the current post processing shader

		int width, height;
		glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
		glViewport(0, 0, width, height);

		// Setup quad over entire screen

		glm::mat4x4 transform = glm::mat4x4(
			2.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f
		);
		glBindVertexArray(VAOs[getCurrentWindowID()]);
		// Vertex Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->vectorData.size() * sizeof(float), ((CompiledMeshData*)standardQuad)->vectorData.data(), GL_STATIC_DRAW);
		// Matrix Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, MBO);
		glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4x4), &transform, GL_STATIC_DRAW);
		// Element Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->indexData.size() * sizeof(int), ((CompiledMeshData*)standardQuad)->indexData.data(), GL_STATIC_DRAW);

		// Bind color texture of the main render pass
		for (int i = 0; i < m_MainRenderPasses[getCurrentWindowID()]->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_MainRenderPasses[getCurrentWindowID()]->textures[i]);
		}

		// Bind post processing shader
		m_CurrentPPShader.bind();

		// Render the quad with the texture
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDrawElements(GL_TRIANGLES, ((CompiledMeshData*)standardQuad)->indexData.size(), GL_UNSIGNED_INT, 0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		Renderer::finalize();
	}

	void Renderer3D::renderMesh(Mesh mesh, glm::mat4x4& transform)
	{
#ifdef LOST_DEBUG_MODE
		debugLogIf((mesh == nullptr), "Mesh was null, this may be because it was destroyed or was never initialized in the firstplace", LOST_LOG_WARNING);

		if (mesh != nullptr)
		{
			glBindVertexArray(VAOs[getCurrentWindowID()]);
			glDrawElements(GL_TRIANGLES, ((CompiledMeshData*)mesh)->indexData.size(), GL_UNSIGNED_INT, 0);
		}
#else
		glBindVertexArray(VAOs[getCurrentWindowID()]);
		glDrawElements(GL_TRIANGLES, ((CompiledMeshData*)mesh)->indexData.size(), GL_UNSIGNED_INT, 0);
#endif
	}
#pragma endregion

	// Base Engine Functions
	Mesh standardQuad = nullptr;

	void _initRenderer(unsigned int rendererMode)
	{
		if (rendererMode == LOST_RENDER_2D)
			_renderer = new Renderer2D();
		else
			_renderer = new Renderer3D();

		_generateBufferObjects();

		lost::MeshData data;
		data.verticies = {
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f }
		};
		data.indexArray = { 0, 2, 1, 2, 0, 3 };
		data.materialSlotIndicies = { 0 };

		standardQuad = lost::makeMesh(data, "LOST_standardQuad");
	}

	void _destroyRenderer()
	{
		delete _renderer;
	}

	void _generateBufferObjects()
	{
		_renderer->generateBufferObjects();
	}

	void _generateNewVAO()
	{
		_renderer->generateNewVAO();
	}

	void _regenerateVAO(unsigned int id)
	{
		_renderer->regenerateVAO(id);
	}

	void _destroyWindowVAO(unsigned int contextIndex)
	{
		_renderer->destroyVAO(contextIndex);
	}

	void _startRender()
	{
		_renderer->startRender();
	}

	void _finalizeRender()
	{
		_renderer->finalize();
	}

	void _resizeFrameBuffers(int windowID, int width, int height)
	{
		_renderer->resizeFrameBuffers(windowID, width, height);
	}

	void _setUsingImGui(bool state)
	{
		_renderer->_setUsingImGui(state);
	}

	bool _getUsingImGui()
	{
		return _renderer->_getUsingImGui();
	}

	void setRenderMode(unsigned int renderMode)
	{
	}

	void fillWindow(Color color)
	{
		_renderer->fillWindow(color);
	}

	void renderMesh(Mesh mesh, std::vector<Material> materials, glm::mat4x4& transform)
	{
		glm::mat4x4 outTransform;

		outTransform = _getCurrentCamera()->getPV() * transform;

		debugLogIf((mesh == nullptr), "Mesh was null, this may be because it was destroyed or was never initialized in the first place", LOST_LOG_WARNING);

		if (mesh != nullptr)
			_renderer->addMeshToQueue(mesh, materials, outTransform, transform);

	}

	void renderMesh(Mesh mesh, std::vector<Material> materials, Vec3 pos, Vec3 rotation, Vec3 scale)
	{

		if (((CompiledMeshData*)mesh)->materialSlotIndicies.size() != materials.size())
		{
			debugLog("Tried to render mesh with incorrect amount of material inputs", LOST_LOG_WARNING);
			return;
		}

		glm::mat4x4 transform = glm::mat4x4(
			scale.x, 0.0f,    0.0f,    0.0f,
			0.0f,    scale.y, 0.0f,    0.0f,
			0.0f,    0.0f,    scale.z, 0.0f,
			0.0f,    0.0f,    0.0f,    1.0f
		);

		transform = glm::eulerAngleXYZ(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)) * transform;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;

		renderMesh(mesh, materials, transform);

	}

	void renderRect(Material mat, Bounds2D bounds)
	{
		Window currentWindow = getCurrentWindow();

		glm::mat4x4 transform = glm::mat4x4(
			  2.0f / getWidth(currentWindow) * bounds.w,         0.0f,                                              0.0f,    0.0f,
			  0.0f,                                             -2.0f / getHeight(currentWindow) * bounds.h,        0.0f,    0.0f,
			  0.0f,                                              0.0f,                                              0.0001f, 0.0f, // [!] Todo: remove depth testing for screenspace, epsilon will not be needed here afterwards
			 -1.0f + bounds.x / getWidth(currentWindow) * 2.0f,  1.0f - bounds.y / getHeight(currentWindow) * 2.0f, 0.1f,    1.0f
		);

		std::vector<Material> materialList = { mat };
		
		_renderer->addMeshToQueue(lost::standardQuad, materialList, transform, transform, LOST_DEPTH_TEST_ALWAYS, false);
	}

	void renderRect3D(Material mat, Vec3 position, Vec2 size, Vec3 rotation)
	{
		glm::mat4x4 transform = glm::mat4x4(
			size.w, 0.0f,   0.0f, 0.0f,
			0.0f,   size.h, 0.0f, 0.0f,
			0.0f,   0.0f,   1.0f, 0.0f,
			0.0f,   0.0f,   0.0f, 1.0f
		);
		transform = glm::eulerAngleXYZ(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)) * transform;
		transform[3][0] += position.x;
		transform[3][1] += position.y;
		transform[3][2] += position.z;

		glm::mat4x4 mpvTransform = _getCurrentCamera()->getPV() * transform;

		std::vector<Material> materialList = { mat };

		_renderer->addMeshToQueue(lost::standardQuad, materialList, mpvTransform, transform);
	}

	void renderQuad(Material mat, Bounds2D bounds, Bounds2D texbounds)
	{
		Window currentWindow = getCurrentWindow();

		bounds.x /= getWidth(currentWindow);
		bounds.y /= getHeight(currentWindow);
		bounds.w /= getWidth(currentWindow);
		bounds.h /= getHeight(currentWindow);

		CompiledMeshData mesh = {};
		//   x --- y --- z               u ----------------------- v               r --- g --- b --- a   nx -- ny -- nz
		mesh.vectorData = {
			bounds.x,            1.0f - bounds.y,            0.0f, texbounds.x,               texbounds.y,               1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			bounds.x + bounds.w, 1.0f - bounds.y,            0.0f, texbounds.x + texbounds.w, texbounds.y,               1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			bounds.x + bounds.w, 1.0f - bounds.y - bounds.h, 0.0f, texbounds.x + texbounds.w, texbounds.y + texbounds.h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			bounds.x,            1.0f - bounds.y - bounds.h, 0.0f, texbounds.x,               texbounds.y + texbounds.h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f
		};
		mesh.indexData = { 0, 2, 1, 2, 0, 3 };
		mesh.materialSlotIndicies = { 0 };

		glm::mat4x4 transform = glm::mat4x4(
			 2.0f,  0.0f, 0.0f, 0.0f,
			 0.0f,  2.0f, 0.0f, 0.0f,
			 0.0f,  0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f
		);
		// glm::translate(glm::scale(glm::identity<glm::mat4x4>(), glm::vec3(bounds.x, bounds.y, 0.0f)), glm::vec3(bounds.w, bounds.h, 1.0f));

		std::vector<Material> materialList = { mat };

		_renderer->addRawToQueue(mesh, materialList, transform, transform, LOST_DEPTH_TEST_ALWAYS, false);
	}

	void renderQuad3D(Material mat, Vec3 position, Vec2 size, Vec3 rotation, Bounds2D texBounds)
	{
		CompiledMeshData mesh = {};
		//   x ----- y ---- z               u ----------------------- v               r --- g --- b --- a   nx -- ny -- nz
		mesh.vectorData = {
			0.0f,   0.0f,   0.0f, texBounds.x,               texBounds.y,               1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			size.x, 0.0f,   0.0f, texBounds.x + texBounds.w, texBounds.y,               1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			size.x, size.y, 0.0f, texBounds.x + texBounds.w, texBounds.y + texBounds.h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
			0.0f,   size.y, 0.0f, texBounds.x,               texBounds.y + texBounds.h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f
		};
		mesh.indexData = { 0, 2, 1, 2, 0, 3 };
		mesh.materialSlotIndicies = { 0 };

		glm::mat4x4 transform = glm::eulerAngleXYZ(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
		transform[3][0] += position.x;
		transform[3][1] += position.y;
		transform[3][2] += position.z;

		glm::mat4x4 mpvTransform = _getCurrentCamera()->getPV() * transform;

		std::vector<Material> materialList = { mat };

		_renderer->addRawToQueue(mesh, materialList, mpvTransform, transform);
	}

	void renderTexture(Texture texture, Bounds2D bounds, Bounds2D texBounds)
	{
		if (texBounds.w == -1) 
			texBounds.w = 1.0f; 
		else 
			texBounds.w /= texture->getWidth();

		if (texBounds.h == -1) 
			texBounds.h = 1.0f; 
		else 
			texBounds.w /= texture->getHeight();

		if (bounds.w == -1.0f) bounds.w = texture->getWidth();
		if (bounds.h == -1.0f) bounds.h = texture->getHeight();

		renderQuad(texture->getMaterial(), bounds, texBounds);
	}

	void renderTexture(Texture texture, float x, float y, float w, float h)
	{
		if (w == -1.0f) w = texture->getWidth();
		if (h == -1.0f) h = texture->getHeight();

		renderQuad(texture->getMaterial(), { x, y, w, h }, { 0.0f, 0.0f, 1.0f, 1.0f });
	}

	void renderInstanceQueue()
	{
		_renderer->renderInstanceQueue();
	}

	void setPostProcessingShader(PPShader shader)
	{
		_renderer->setPostProcessingShader(shader);
	}
}