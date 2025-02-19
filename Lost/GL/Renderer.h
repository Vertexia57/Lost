#pragma once
#include "Mesh/Mesh.h"
#include "glm/glm.hpp"
#include <vector>
#include "RenderPass.h"
#include "Structs.h" 

enum RenderMode2D
{
	// Renders every mesh to the frame buffer instantly, no queue
	// This removes the efficiency of instancing identical meshes and should only be set if necessary
	LOST_RENDER_MODE_INSTANT,

	// This is the DEFAULT setting Lost has the renderer set to on start up
	// Queues up identical meshes into an instance buffer and renders them to the frame buffer when:
	//  - lost::EndFrame() is ran
	//  - A non identical mesh is rendered
	//  - lost::renderInstanceQueue() is ran, manually activating the queue
	//  - A shader's uniform has been changed (This is the only difference from LOST_RENDER_MODE_QUEUE)
	LOST_RENDER_MODE_AUTO_QUEUE,

	// Queues up identical meshes into an instance buffer and renders them to the frame buffer when:
	//  - lost::EndFrame() is ran
	//  - A non identical mesh is rendered
	//  - lost::renderInstanceQueue() is ran, manually activating the queue
	LOST_RENDER_MODE_QUEUE
};

enum RendererMode
{
	LOST_RENDER_2D,
	LOST_RENDER_3D
};

namespace lost
{
	
	extern Mesh standardQuad;

	// Initializes the renderer for rendering
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _initRenderer(unsigned int rendererMode);

	// Destroys the renderer, removing it from the heap
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _destroyRenderer();
	
	// Generates the buffer objects used by the renderer, gets ran after the invisible context is created
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _generateBufferObjects();

	// Generates a new VAO for the renderer, this binds to the latest context created
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _generateNewVAO();

	// Regenerates a VAO for the renderer, this binds to the context with the same id as the VAO
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _regenerateVAO(unsigned int id);

	// Destroys a VAO on the renderer, usually done when a window is closed
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _destroyWindowVAO(unsigned int contextIndex);

	// Runs the "startRender" function within the renderer
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _startRender();

	// Runs the "finalize" function within the renderer, doing all rendering tasks that were unfinished or needed to wait
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _finalizeRender();

	// Resizes the framebuffers in the renderer to fit the new window size
	// NOTE: This is only used inside of the Lost engine, do not run it (unless you know what you're doing)
	void _resizeFrameBuffers(int windowID, int width, int height);

#ifndef IMGUI_DISABLE
	// Used within the renderer to know if it should run the ImGui functions
	// Only defined when ImGui is enabled
	void _setUsingImGui(bool state);

	// Used within the renderer to know if it should run the ImGui functions
	// Only defined when ImGui is enabled
	bool _getUsingImGui();
#endif

	// Changes the render mode of the renderer, follows the RenderMode enum
	void setRenderMode(unsigned int renderMode);

	// [---------------------]
	//   Rendering Functions
	// [---------------------]

	// Fills the current window with the color given
	void fillWindow(Color color);

	// Depending on the current render mode of the renderer, renders the mesh given to the screen
	void renderMesh(Mesh mesh, std::vector<Material> materials, glm::mat4x4& transform);
	// Depending on the current render mode of the renderer, renders the mesh given to the screen using the position and scale given
	void renderMesh(Mesh mesh, std::vector<Material> materials, Vec3 pos, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Vec3 scale = { 1.0f, 1.0f, 1.0f });

	// Renders a rect to the screen, this can use batching unlike renderQuad
	void renderRect(Material mat, Bounds2D bounds);
	// Renders a rect in 3D space, taking in a 3D position, size and rotation, can use batching unlike renderQuad3D
	void renderRect3D(Material mat, Vec3 position, Vec2 size, Vec3 rotation = { 0.0f, 0.0f, 0.0f });
	// Renders a quad to the screen not caring for perspective or view, (Cannot use batching)
	void renderQuad(Material mat, Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f });
	// Renders a quad in 3D space, taking in a 3D position, size and rotation and texture bounds (Cannot use batching)
	void renderQuad3D(Material mat, Vec3 position, Vec2 size, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f });

	// Renders the texture to the screen using the default shader, bounds is the area of the screen it renders to and texBounds is the area on the texture it will use in pixels, by default rendering the whole texture
	void renderTexture(Texture texture, Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, -1.0f, -1.0f });
	// Renders the texture to the screen in the area given
	void renderTexture(Texture texture, float x, float y, float w = -1, float h = -1);

	// Renders the queue of meshes stored in the instance queue
	void renderInstanceQueue();

	// Sets the current post processing shader
	// Temporary
	void setPostProcessingShader(PPShader shader);

	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		// Sets the render mode 
		void setRenderMode(unsigned int renderMode);

		// Generates the buffer objects used by the renderer, gets ran after the invisible context is created
		void generateBufferObjects();

		// Resizes the framebuffers in the renderer to fit the new window size
		void resizeFrameBuffers(int windowID, int width, int height);

		// Sets the current post processing shader
		void setPostProcessingShader(PPShader shader);

		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);
		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);

		// Renders the queue of meshes in the render queue
		virtual void renderInstanceQueue();

		// Renders a mesh using instant renderering
		virtual void renderMesh(Mesh mesh, glm::mat4x4& transform);

		virtual void startRender();
		virtual void finalize();

		// Generates a new VAO for the renderer, this binds to the latest context created
		void generateNewVAO();
		// Regenerates a VAO for the renderer, this binds to the context with the same id as the VAO
		void regenerateVAO(unsigned int id);
		// Destroys a VAO on the renderer, usually done when a window is closed
		void destroyVAO(unsigned int contextIndex);

		// Binds the buffer descriptions to whatever VAO is currently bound
		// NOTE: Is contextual depending on what state OpenGL is in
		void bindBufferDescriptions();

#ifndef IMGUI_DISABLE
		// Used within the renderer to know if it should run the ImGui functions
		// Only defined when ImGui is enabled
		inline void _setUsingImGui(bool state) { m_UsingImGui = state; };

		// Used within the renderer to know if it should run the ImGui functions
		// Only defined when ImGui is enabled
		inline bool _getUsingImGui()   const   { return  m_UsingImGui; };
#endif

		void fillWindow(Color color);
	protected:
#ifndef IMGUI_DISABLE
		bool m_UsingImGui = false;
#endif

		unsigned int m_RenderMode = LOST_RENDER_MODE_AUTO_QUEUE;

		std::vector<CompiledMeshData*> m_RawMeshes;

		std::vector<RenderPass> m_MainRenderPasses;

		unsigned int m_EmptyVAO = 0; // The invisible windows VAO
		std::vector<unsigned int> VAOs; // One per window
		unsigned int VBO; // Vertex Buffer Object
		unsigned int MBO; // Matrix Buffer Object
		unsigned int EBO; // Element Buffer Object

		PPShader m_CurrentPPShader;
	};

	class Renderer2D : public Renderer
	{
	public:
		Renderer2D();
		virtual ~Renderer2D();

		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);
		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);

		// Renders the queue of meshes in the render queue
		virtual void renderInstanceQueue();

		virtual void finalize();

		// Renders a mesh using instant renderering
		virtual void renderMesh(Mesh mesh, glm::mat4x4& transform);
	private:
		Mesh m_CurrentMeshID = nullptr;
		Material m_CurrentMaterial = nullptr;
		unsigned int m_CurrentIndexOffset = -1;
		std::vector<glm::mat4x4> m_TransformArray;
	};

	class Renderer3D : public Renderer
	{
	protected:
		
		struct MeshRenderData
		{
			Mesh mesh;

			Material material;

			unsigned int startIndex;
			unsigned int indicies;

			float depthVal;

			glm::mat4x4 mvpTransform;
			glm::mat4x4 modelTransform;

			unsigned int depthMode = LOST_DEPTH_TEST_LESS;
			bool depthWrite = true;

			bool depthComp(MeshRenderData& other) const
			{
				return other.depthVal > depthVal;
			}
		};

		static bool meshSortFunc(const MeshRenderData& lhs, const MeshRenderData& rhs)
		{
			if (lhs.material->getDepthTestFunc() == LOST_DEPTH_TEST_ALWAYS || rhs.material->getDepthTestFunc() == LOST_DEPTH_TEST_ALWAYS)
			{
				// No batching when using LOST_DEPTH_TEST_ALWAYS
				// This is to preserve order when rendering
				if (lhs.material->getDepthTestFunc() == LOST_DEPTH_TEST_ALWAYS)
					return false;
				return true;
			}

			// Normal, has batching

			if (lhs.material->getQueueLevel() != rhs.material->getQueueLevel())
				return lhs.material->getQueueLevel() < rhs.material->getQueueLevel();
			if (lhs.material->getShader() != rhs.material->getShader())
				return lhs.material->getShader() < rhs.material->getShader();
			if (lhs.material != rhs.material)
				return lhs.material < rhs.material;
			if (lhs.mesh != rhs.mesh)
				return lhs.mesh < rhs.mesh;
			if (lhs.startIndex != rhs.startIndex)
				return lhs.startIndex < rhs.startIndex;
			if (lhs.depthMode != rhs.depthMode)
				return lhs.depthMode < rhs.depthMode;
			if (lhs.depthWrite != rhs.depthWrite)
				return lhs.depthWrite < rhs.depthWrite;

			return (lhs.depthVal < rhs.depthVal);
		}
	public:
		Renderer3D();
		virtual ~Renderer3D();

		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addMeshToQueue(Mesh mesh, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);
		// Adds a mesh to the image queue and renders it if the conditions are met
		virtual void addRawToQueue(CompiledMeshData& meshData, std::vector<Material>& materials, const glm::mat4x4& mvpTransform, const glm::mat4x4& modelTransform, unsigned int depthTestFuncOverride = LOST_DEPTH_TEST_AUTO, bool depthWrite = true);

		// Renders the queue of meshes in the render queue
		virtual void renderInstanceQueue();

		// Finalizes the render
		virtual void finalize();

		// Renders a mesh using instant renderering
		virtual void renderMesh(Mesh mesh, glm::mat4x4& transform);
	private:
		std::vector<MeshRenderData> m_MainRenderData;
	};

}