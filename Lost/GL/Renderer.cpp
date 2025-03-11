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

	void Renderer::setCullMode(unsigned int cullMode)
	{
		m_CullMode = cullMode;
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

		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

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

	unsigned int Renderer::getRenderTexture(unsigned int windowID, unsigned int pass)
	{
		return m_MainRenderPasses[windowID]->textures[pass];
	}

	unsigned int Renderer::getDepthTexture(unsigned int windowID)
	{
		return m_MainRenderPasses[windowID]->depthStencilTexture;
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// TexCoord
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1); 

		// Color
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// Normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		// Tangent
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(4);

		// Bind matrix buffer mat4x4's are stored as 4 * vec4's
		glBindBuffer(GL_ARRAY_BUFFER, MBO);
		glVertexAttribPointer(5,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, 0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(6,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(4  * sizeof(float)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(7,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(8  * sizeof(float)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(8,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(8);

		glVertexAttribPointer(9,  4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(16 * sizeof(float)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(20 * sizeof(float)));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(24 * sizeof(float)));
		glEnableVertexAttribArray(11);
		glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4) * 2, (void*)(28 * sizeof(float)));
		glEnableVertexAttribArray(12);

		glVertexAttribDivisor(5, 1); // Make it so this input only updates once per INSTANCE rather than per vertex
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);

		glVertexAttribDivisor(9,  1);
		glVertexAttribDivisor(10, 1);
		glVertexAttribDivisor(11, 1);
		glVertexAttribDivisor(12, 1);

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

	void Renderer::setPassClearColor(unsigned int passID, Color color)
	{
		const std::vector<Window>& windows = lost::getWindows();
		for (int i = 0; i < windows.size(); i++)
		{
			RenderBufferData& renderBuffer = m_MainRenderPasses[i]->storedBuffers[passID];
			renderBuffer.defaultColor = { color.r, color.g, color.b, color.a };
		}
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
			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)m_CurrentMeshID)->vertexData.size() * sizeof(float), ((CompiledMeshData*)m_CurrentMeshID)->vertexData.data(), GL_STATIC_DRAW);
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
		glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->vertexData.size() * sizeof(float), ((CompiledMeshData*)standardQuad)->vertexData.data(), GL_STATIC_DRAW);
		// Matrix Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, MBO);
		glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4x4), &transform, GL_STATIC_DRAW);
		// Element Buffer Data
		glBindBuffer(GL_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->indexData.size() * sizeof(int), ((CompiledMeshData*)standardQuad)->indexData.data(), GL_STATIC_DRAW);

		//glBindTexture(GL_TEXTURE_2D, m_MainRenderPasses[getCurrentWindowID()]->textures[0]);
		_getDefaultWhiteTexture()->bind(0);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDrawElementsInstanced(GL_TRIANGLES, ((CompiledMeshData*)standardQuad)->indexData.size(), GL_UNSIGNED_INT, 0, 1);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

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
			renderData.depthWrite = depthWrite && materials[i]->getDepthWrite();
			renderData.cullMode = (m_CullMode == LOST_CULL_AUTO ? materials[i]->getFaceCullMode() : m_CullMode);
			renderData.renderMode = ((CompiledMeshData*)mesh)->meshRenderMode;

			debugLogIf(renderData.cullMode == LOST_CULL_AUTO, "Material had cull mode LOST_CULL_AUTO. Which shouldn't be used by materials, and only the renderer", LOST_LOG_WARNING);

			m_MainRenderData.push_back(renderData);
		}

		// Active during LOST_RENDER_MODE_QUEUE and LOST_RENDER_MODE_AUTO_QUEUE
		// Fulfils the rule of "A non identical mesh is rendered
	}

	void Renderer3D::addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride, bool depthWrite)
	{
		CompiledMeshData* newMesh = new CompiledMeshData{
			meshData.vertexData,
			meshData.materialSlotIndicies,
			meshData.indexData,
			meshData.meshRenderMode
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
			unsigned int currentCullMode      = m_MainRenderData[0].cullMode;
			
			// Setup first meshes material and vertex data
			currentMaterial->bindShader();
			currentMaterial->bindTextures();

			// Set the depth test function to the first one
			glDepthMask(currentDepthWrite);
			glDepthFunc(currentDepthTestFunc);

			// Set the cull mode of the renderer
			if (currentCullMode != LOST_CULL_NONE) 
			{
				// Set the current current cull mode
				glCullFace(currentCullMode);
				m_CullingEnabled = true;

				// Culling is automatically enabled at the end of every frame, we don't need to enable it
			}
			else
			{
				glDisable(GL_CULL_FACE);
				m_CullingEnabled = false;
			} 

			// Vertex Array Object
			glBindVertexArray(VAOs[getCurrentWindowID()]);
			// Vertex Buffer Data
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vertexData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vertexData.data(), GL_STATIC_DRAW);

			glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vertexData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vertexData.data(), GL_STATIC_DRAW); // [!] TODO: WTF?

			// Create transform list, reserving the maximum size that could occur
			std::vector<glm::mat4x4> transforms;
			transforms.reserve(m_MainRenderData.size() * 2);

			for (int i = 0; i < m_MainRenderData.size(); i++)
			{
				MeshRenderData& renderData = m_MainRenderData[i];

				if (renderData.mesh != currentMesh || renderData.material != currentMaterial || currentIndexOffset != renderData.startIndex || currentDepthTestFunc != renderData.depthMode || currentDepthWrite != renderData.depthWrite || currentCullMode != renderData.cullMode)
				{
					// Render all matching meshes with instancing

					// Matrix Buffer Data
					glBindBuffer(GL_ARRAY_BUFFER, MBO);
					glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4x4), transforms.data(), GL_STATIC_DRAW);
					// Element Buffer Data
					glBindBuffer(GL_ARRAY_BUFFER, EBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, currentIndexCount * sizeof(int), ((CompiledMeshData*)currentMesh)->indexData.data() + currentIndexOffset, GL_STATIC_DRAW);

					glDrawElementsInstanced(((CompiledMeshData*)currentMesh)->meshRenderMode, currentIndexCount, GL_UNSIGNED_INT, 0, transforms.size() / 2);

					// Recreate transform list, reserving the maximum size that could occur
					transforms.clear();
					transforms.reserve(2 * (m_MainRenderData.size() - i));

					// Set the index offset for the new mesh
					currentIndexOffset = renderData.startIndex;
					currentIndexCount = renderData.indicies;

					// Set the new depth mode settings
					if (renderData.depthMode != currentDepthTestFunc)
					{
						currentDepthTestFunc = renderData.depthMode;
						glDepthFunc(currentDepthTestFunc);
					}

					// Set the new depth write settings
					if (renderData.depthWrite != currentDepthWrite)
					{
						currentDepthWrite = renderData.depthWrite;
						glDepthMask(currentDepthWrite);
					}

					// Set the new cull mode
					if (renderData.cullMode != currentCullMode)
					{
						// Update the culling value
						currentCullMode = renderData.cullMode;

						// Set the cull mode of the renderer
						if (currentCullMode != LOST_CULL_NONE)
						{
							// Check if culling is already enabled
							if (!m_CullingEnabled)
							{
								glEnable(GL_CULL_FACE);
								m_CullingEnabled = true;
							}

							// Set the current current cull mode
							glCullFace(currentCullMode);
						}
						else if (m_CullingEnabled) // Check if it is enabled
						{
							glDisable(GL_CULL_FACE);
							m_CullingEnabled = false;
						}
					}

					// Update mesh vertex data if necessary
					if (renderData.mesh != currentMesh)
					{
						currentMesh = renderData.mesh;

						// Vertex Buffer Data
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)currentMesh)->vertexData.size() * sizeof(float), ((CompiledMeshData*)currentMesh)->vertexData.data(), GL_STATIC_DRAW);

					}

					// Update mesh material if necessary
					if (renderData.material != currentMaterial)
					{
						if (renderData.material->getShader() != currentMaterial->getShader())
							renderData.material->bindShader();

						currentMaterial = renderData.material;
						currentMaterial->bindTextures();

						if (currentMaterial->hasMaterialUniforms())
							currentMaterial->bindMaterialUniforms();
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
			
			glDrawElementsInstanced(((CompiledMeshData*)m_MainRenderData[m_MainRenderData.size() - 1].mesh)->meshRenderMode, m_MainRenderData[m_MainRenderData.size() - 1].indicies, GL_UNSIGNED_INT, 0, transforms.size());

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
		glBufferData(GL_ARRAY_BUFFER, ((CompiledMeshData*)standardQuad)->vertexData.size() * sizeof(float), ((CompiledMeshData*)standardQuad)->vertexData.data(), GL_STATIC_DRAW);
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

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_MainRenderPasses[getCurrentWindowID()]->depthStencilTexture);

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
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
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

	void setPassClearColor(unsigned int passID, Color color)
	{
		_renderer->setPassClearColor(passID, color);
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

		// [!] TODO: Make this a function? since it scales it might not need to be
		glm::mat4x4 transform = glm::mat4x4(
			  2.0f / getWidth(currentWindow) * bounds.w,         0.0f,                                              0.0f,    0.0f,
			  0.0f,                                             -2.0f / getHeight(currentWindow) * bounds.h,        0.0f,    0.0f,
			  0.0f,                                              0.0f,                                              0.0001f, 0.0f, // [!] Todo: remove depth testing for screenspace, epsilon will not be needed here afterwards
			 -1.0f + bounds.x / getWidth(currentWindow) * 2.0f,  1.0f - bounds.y / getHeight(currentWindow) * 2.0f, 0.1f,    1.0f
		);

		std::vector<Material> materialList = { mat };

		const Color& color = getNormalizedColor();
		CompiledMeshData mesh = {};
		mesh.vertexData = {
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f
		};
		mesh.indexData = { 0, 2, 1, 2, 0, 3 };
		mesh.materialSlotIndicies = { 0 };

		_renderer->addRawToQueue(mesh, materialList, transform, transform, LOST_DEPTH_TEST_ALWAYS, false);
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

		std::vector<Material>materialList = { mat };

		const Color& color = getNormalizedColor();

		CompiledMeshData mesh = {};

		mesh.vertexData = {
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, color.r, color.g, color.b, color.a, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f
		};

		mesh.indexData = { 0, 2, 1, 2, 0, 3 };
		mesh.materialSlotIndicies = { 0 };

		_renderer->addRawToQueue(mesh, materialList, mpvTransform, transform);
	}

	void renderQuad(Material mat, Bounds2D bounds, Bounds2D texbounds)
	{
		Window currentWindow = getCurrentWindow();

		bounds.x /= getWidth(currentWindow);
		bounds.y /= getHeight(currentWindow);
		bounds.w /= getWidth(currentWindow);
		bounds.h /= getHeight(currentWindow);

		// Get current render color from state
		const Color& color = getNormalizedColor();

		CompiledMeshData mesh = {};
		mesh.vertexData = {
			bounds.x,            1.0f - bounds.y,            0.0f, texbounds.x,               texbounds.y,               color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			bounds.x + bounds.w, 1.0f - bounds.y,            0.0f, texbounds.x + texbounds.w, texbounds.y,               color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			bounds.x + bounds.w, 1.0f - bounds.y - bounds.h, 0.0f, texbounds.x + texbounds.w, texbounds.y + texbounds.h, color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			bounds.x,            1.0f - bounds.y - bounds.h, 0.0f, texbounds.x,               texbounds.y + texbounds.h, color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f
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
		// Get current render color from state
		const Color& color = getNormalizedColor();

		CompiledMeshData mesh = {};
		//   x ----- y ---- z               u ----------------------- v               r ------- g ------ b ----- a   nx -- ny -- nz
		mesh.vertexData = {
			0.0f,   0.0f,   0.0f, texBounds.x,               texBounds.y,               color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			size.x, 0.0f,   0.0f, texBounds.x + texBounds.w, texBounds.y,               color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			size.x, size.y, 0.0f, texBounds.x + texBounds.w, texBounds.y + texBounds.h, color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f,   size.y, 0.0f, texBounds.x,               texBounds.y + texBounds.h, color.r, color.g, color.b, color.a, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f
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

	void beginMesh(unsigned int meshMode, bool screenspace)
	{
		// Clear the old data that was used in the last mesh
		_renderer->_TempMeshBuild = {};
		_renderer->_TempMeshBuild.meshRenderMode = meshMode;
		_renderer->_TempMeshBuild.materialSlotIndicies = { 0 };
		_renderer->_TempMeshUsesWorldTransform = !screenspace;
		_renderer->_TempMeshModelTransform = glm::identity<glm::mat4x4>();

		// Check if was already building mesh
		if (_renderer->_BuildingMesh)
		{
			debugLog("Tried to run \"beginMesh()\" while already creating a mesh.", LOST_LOG_WARNING);
			return;
		}

		// Begin building the mesh
		_renderer->_BuildingMesh = true;
	}

	void endMesh(std::vector<Material>& materials)
	{
		glm::mat4x4 mpvTransform;
		// Get the MPV of the model
		if (_renderer->_TempMeshUsesWorldTransform)
			mpvTransform = _getCurrentCamera()->getPV() * _renderer->_TempMeshModelTransform;
		else
		{
			Window currentWindow = getCurrentWindow();

			// [!] TODO: Make this a function
			mpvTransform = glm::mat4x4(
				 2.0f / getWidth(currentWindow), 0.0f,							  0.0f,		0.0f,
				 0.0f,							-2.0f / getHeight(currentWindow), 0.0f,		0.0f,
				 0.0f,							 0.0f,							  0.0001f,	0.0f,
				-1.0f,							 1.0f,							  0.2f,		1.0f
			) * _renderer->_TempMeshModelTransform;
		}

		// Add the mesh data to the render queue
		_renderer->addRawToQueue(_renderer->_TempMeshBuild, materials, mpvTransform, _renderer->_TempMeshModelTransform, _renderer->_TempMeshUsesWorldTransform ? LOST_DEPTH_TEST_AUTO : LOST_DEPTH_TEST_ALWAYS, _renderer->_TempMeshUsesWorldTransform);
		_renderer->_BuildingMesh = false;
	}

	void endMesh(Material material)
	{
		// Create a material list which contains the material given
		std::vector<Material> materials = { material };

		glm::mat4x4 mpvTransform;
		// Get the MPV of the model
		if (_renderer->_TempMeshUsesWorldTransform)
			mpvTransform = _getCurrentCamera()->getPV() * _renderer->_TempMeshModelTransform;
		else
		{
			Window currentWindow = getCurrentWindow();

			// Screenspace transform
			// [!] TODO: Make this a function
			mpvTransform = glm::mat4x4(
				 2.0f / getWidth(currentWindow), 0.0f,							  0.0f,		0.0f,
				 0.0f,							-2.0f / getHeight(currentWindow), 0.0f,		0.0f,
				 0.0f,							 0.0f,							  0.0001f,	0.0f,
				-1.0f,							 1.0f,							  0.2f,		1.0f
			) * _renderer->_TempMeshModelTransform;
		}

		// Add the mesh data to the render queue
		_renderer->addRawToQueue(_renderer->_TempMeshBuild, materials, mpvTransform, _renderer->_TempMeshModelTransform, _renderer->_TempMeshUsesWorldTransform ? LOST_DEPTH_TEST_AUTO : LOST_DEPTH_TEST_ALWAYS, _renderer->_TempMeshUsesWorldTransform);
		_renderer->_BuildingMesh = false;
	}

	void endMesh()
	{
		endMesh(lost::_getDefaultWhiteMaterial());
	}

	void addVertex(Vec3 position, Color vertexColor, Vec2 textureCoord)
	{
		// Get current render color from state
		vertexColor.normalize();
		vertexColor *= getNormalizedColor();

		// Create a vector of the vertex data
		// [!] TODO: Convert this into a function just incase we change the format
		std::vector<float> vertexData = {
			position.x, position.y, position.z,
			textureCoord.x, textureCoord.y,
			vertexColor.r, vertexColor.g, vertexColor.b, vertexColor.a,
			0.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 0.0f, 0.0f // Normal, Tangent
		};
		// Append the data to the end of the list
		_renderer->_TempMeshBuild.vertexData.insert(_renderer->_TempMeshBuild.vertexData.end(), vertexData.begin(), vertexData.end());
		_renderer->_TempMeshBuild.indexData.push_back(_renderer->_TempMeshBuild.indexData.size());
	}

	void addVertex(Vertex& vertex)
	{
		// Get current render color from state
		const Color& currentColor = getNormalizedColor();

		// Create a vector of the vertex data
		// [!] TODO: Convert this into a function just incase we change the format
		std::vector<float> vertexData = {
			vertex.position.x, vertex.position.y, vertex.position.z,
			vertex.textureCoord.x, vertex.textureCoord.y,
			vertex.vertexColor.r * currentColor.r, vertex.vertexColor.g * currentColor.g, vertex.vertexColor.b * currentColor.b, vertex.vertexColor.a * currentColor.a,
			0.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 0.0f, 0.0f // Normal, Tangent
		};

		// Append the data to the end of the list
		_renderer->_TempMeshBuild.vertexData.insert(_renderer->_TempMeshBuild.vertexData.end(), vertexData.begin(), vertexData.end());
		_renderer->_TempMeshBuild.indexData.push_back(_renderer->_TempMeshBuild.indexData.size());
	}

	void setMeshTransform(glm::mat4x4& transform)
	{
		// Set the world transform of the mesh
		_renderer->_TempMeshUsesWorldTransform = true;
		_renderer->_TempMeshModelTransform = transform;
	}

	void setMeshTransform(Vec3 position, Vec3 scale, Vec3 rotation, bool screenspace)
	{
		// Create a transform
		_renderer->_TempMeshUsesWorldTransform = !screenspace;

		glm::mat4x4 transform = glm::mat4x4(
			scale.x, 0.0f,    0.0f,    0.0f,
			0.0f,    scale.y, 0.0f,    0.0f,
			0.0f,    0.0f,    scale.z, 0.0f,
			0.0f,    0.0f,    0.0f,    1.0f
		);
		transform = glm::eulerAngleXYZ(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)) * transform;
		transform[3][0] += position.x;
		transform[3][1] += position.y;
		transform[3][2] += position.z;

		// Set the world transform of the mesh
		_renderer->_TempMeshModelTransform = transform;

	}

	void setCullMode(unsigned int cullMode)
	{
		_renderer->setCullMode(cullMode);
	}

	unsigned int getRenderTexture(unsigned int pass, unsigned int windowID)
	{
		if (windowID == -1)
			windowID = getCurrentWindowID();

		if (windowID >= getWindows().size())
		{
			debugLog("Tried to get a render texture of a non-existant window", LOST_LOG_ERROR);
			return 0;
		}

		return _renderer->getRenderTexture(windowID, pass);
	}

	unsigned int getDepthTexture(unsigned int windowID)
	{
		if (windowID == -1)
			windowID = getCurrentWindowID();

		return _renderer->getDepthTexture(windowID);
	}
}