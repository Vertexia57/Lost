#pragma once
#include "Mesh/Mesh.h"
#include "glm/glm.hpp"
#include <vector>
#include "RenderPass.h"
#include "Structs.h" 

// [!] TODO: Make addRawMeshToQueue() use batching, by making a queue of verticies that is pushed once there is a material change

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

	// Sets the clear color of the given render buffer, this effects all windows
	void setPassClearColor(unsigned int passID, Color color);

	// Depending on the current render mode of the renderer, renders the mesh given to the screen
	void renderMesh(Mesh mesh, std::vector<Material> materials, glm::mat4x4& transform);
	// Depending on the current render mode of the renderer, renders the mesh given to the screen using the position and scale given
	void renderMesh(Mesh mesh, std::vector<Material> materials, Vec3 pos, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Vec3 scale = { 1.0f, 1.0f, 1.0f });

	// Renders a quad to the screen not caring for perspective or view
	void renderRect(Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f }, Material mat = nullptr);
	// Renders a quad to the screen not caring for perspective or view, allows for rotation
	void renderRectPro(Bounds2D bounds, Vec2 origin, float angle, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f }, Material mat = nullptr);
	// Renders a quad in 3D space, taking in a 3D position, size and rotation and texture bounds (Cannot use batching)
	void renderRect3D(Vec3 position, Vec2 size, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Bounds2D texBounds = { 0.0f, 0.0f, 1.0f, 1.0f }, Material mat = nullptr);

	// [!] TODO: renderRectPro3D()

	// Renders a circle to the screen, detail is the amount of segments the polygon will have
	void renderCircle(Vec2 position, float radius, Material mat = nullptr, int detail = 30);
	// Renders a circle in 3D space, detail is the amount of segments the polygon will have
	void renderCircle3D(Vec3 position, float radius, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Material mat = nullptr, int detail = 30);

	// Renders a ellipse to the screen, detail is the amount of segments the polygon will have
	void renderEllipse(Vec2 position, Vec2 extents, Material mat = nullptr, int detail = 30);
	// Renders a ellipse to the screen, detail is the amount of segments the polygon will have
	void renderEllipsePro(Vec2 position, Vec2 extents, Vec2 origin, float angle, Material mat = nullptr, int detail = 30);
	// Renders a ellipse in 3D space, detail is the amount of segments the polygon will have
	void renderEllipse3D(Vec3 position, Vec2 extents, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, Material mat = nullptr, int detail = 30);

	// Renders the texture to the screen using the default shader, bounds is the area of the screen it renders to and texBounds is the area on the texture it will use in pixels, by default rendering the whole texture
	void renderTexture(Texture texture, Bounds2D bounds, Bounds2D texBounds = { 0.0f, 0.0f, -1.0f, -1.0f });
	// Renders the texture to the screen in the area given
	void renderTexture(Texture texture, float x, float y, float w = -1, float h = -1);

	// Renders the queue of meshes stored in the instance queue
	void renderInstanceQueue();

	// Sets the current post processing shader
	// Temporary
	void setPostProcessingShader(PPShader shader);

	// Starts the creation of a mesh which will be rendered once endMesh() is ran
	
	/// <summary>
	/// Begins creating a mesh. Works in tandum with endMesh(), addVertex() and setMeshTransform()
	/// </summary>
	/// <param name="meshMode">Mesh mode uses the LOST_MESH_xxx enum, this dictates what mode the mesh will be rendered in</param>
	/// <param name="screenspace">Dictates whether the mesh uses screenspace coordinates</param>
	void beginMesh(unsigned int meshMode = LOST_MESH_TRIANGLES, bool screenspace = false);
	// Finishes the creation of a mesh and renders it, using the materials provided
	void endMesh(std::vector<Material>& materials);
	// Finishes the creation of a mesh and renders it, using the material provided
	void endMesh(Material material);
	// Finishes the creation of a mesh and renders it, using a default white material
	void endMesh();

	/// <summary>
	/// Adds a vertex to the mesh being created, must be ran after beginMesh()
	/// </summary>
	/// <param name="position">The 3D local position of the mesh</param>
	/// <param name="vertexColor">The color of the vertex, from 0 - 255</param>
	/// <param name="textureCoord">The texture coordinate or UV of the vertex</param>
	void addVertex(Vec3 position, Color vertexColor = { 255, 255, 255, 255 }, Vec2 textureCoord = { 0.0f, 0.0f });
	// Adds a vertex to the mesh being created, must be ran after beginMesh()
	void addVertex(Vertex& vertex);

	// Sets the *WORLD* transform of the mesh being rendered, must be ran after beginMesh()
	void setMeshTransform(glm::mat4x4& transform);
	// Sets the *WORLD* transform of the mesh being rendered, must be ran after beginMesh()
	// "screenspace" when true will use a screenspace projection
	void setMeshTransform(Vec3 position, Vec3 scale = { 1.0f, 1.0f, 1.0f }, Vec3 rotation = { 0.0f, 0.0f, 0.0f }, bool screenspace = false);

	// Sets the current backface culling override, LOST_CULL_AUTO is default, which doesn't override material cull settings
	void setCullMode(unsigned int cullMode);

	// Returns the OpenGL texture id of the render pass selected, by default getting the render texture of the window that's active
	unsigned int getRenderTexture(unsigned int pass, unsigned int windowID = -1);
	// Returns the OpenGL depth texture id, by default getting the render texture of the window that's active
	unsigned int getDepthTexture(unsigned int windowID = -1);

	class Renderer
	{
	protected:
		struct RawMeshBuffer
		{
			std::vector<Material> materials;
			glm::mat4x4 mvpTransform;
			glm::mat4x4 modelTransform;
			unsigned int depthTestFuncOverride;
			bool depthWrite;

			bool operator==(const RawMeshBuffer& rhs) const
			{
				// Check if the material array is the same
				if (materials.size() != rhs.materials.size())
					return false;

				for (int i = 0; i < materials.size(); i++)
				{
					if (materials.at(i) != rhs.materials.at(i))
						return false;
				}

				// Compare transforms, we will only compare the MVP transform
				// [?] Comparing the model transform may be important too, as the MVP may be the same but the model different
				if (mvpTransform != rhs.mvpTransform)
					return false;

				// Compare depth settings
				return (depthTestFuncOverride == rhs.depthTestFuncOverride || depthWrite == rhs.depthWrite);
			}
		};
	public:
		Renderer();
		virtual ~Renderer();

		// Sets the render mode 
		void setRenderMode(unsigned int renderMode);

		// Sets the cull mode
		void setCullMode(unsigned int cullMode);

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

		virtual void initRenderInstanceQueue();
		// Renders the queue of meshes in the render queue
		virtual void renderInstanceQueue();

		// Renders a mesh using instant renderering
		virtual void renderMesh(Mesh mesh, glm::mat4x4& transform);

		virtual void startRender();
		virtual void finalize();

		// Returns the render texture used by the pass at the index given
		unsigned int getRenderTexture(unsigned int windowID, unsigned int pass);
		// Returns the OpenGL depth texture id, by default getting the render texture of the window that's active
		unsigned int getDepthTexture(unsigned int windowID = -1);

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

		void setPassClearColor(unsigned int passID, Color color);

		// Mesh Building

		// The mesh used when running beingMesh and endMesh;
		CompiledMeshData _TempMeshBuild = {};
		bool _BuildingMesh = false;
		bool _TempMeshUsesWorldTransform = false;
		glm::mat4x4 _TempMeshModelTransform;
	protected:
#ifndef IMGUI_DISABLE
		bool m_UsingImGui = false;
#endif

		unsigned int m_RenderMode = LOST_RENDER_MODE_AUTO_QUEUE;
		unsigned int m_CullMode = LOST_CULL_AUTO;
		bool m_CullingEnabled = true; // Checks if culling is enabled, this is to reduce calls

		CompiledMeshData* m_RawMeshInstance = nullptr; // Is set to the last raw mesh rendered, reset on queue render
		RawMeshBuffer m_RawMeshBuffer; // Stores the settings of the last raw mesh rendered

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

			unsigned int cullMode = LOST_CULL_AUTO;
			unsigned int ZSortMode = LOST_ZSORT_NORMAL;
			unsigned int depthMode = LOST_DEPTH_TEST_LESS;
			unsigned int renderMode = LOST_MESH_TRIANGLES;
			bool depthWrite = true;

			bool depthComp(MeshRenderData& other) const
			{
				return other.depthVal > depthVal;
			}
		};

		static bool meshSortFunc(const MeshRenderData& lhs, const MeshRenderData& rhs)
		{
			// Check if ZSorting is disabled
			if (lhs.ZSortMode == LOST_ZSORT_NONE || rhs.ZSortMode == LOST_ZSORT_NONE)
				return false;
			// Check if ZSorting is done by depth
			if (lhs.ZSortMode == LOST_ZSORT_DEPTH || rhs.ZSortMode == LOST_ZSORT_DEPTH)
				return (lhs.depthVal < rhs.depthVal);

			// Normal ZSorting

			// [!] TODO: Remove this and replace with LOST_ZSORT_NONE
			if (lhs.depthMode == LOST_DEPTH_TEST_ALWAYS || rhs.depthMode == LOST_DEPTH_TEST_ALWAYS)
			{
				// No batching when using LOST_DEPTH_TEST_ALWAYS
				// This is to preserve order when rendering
				return false;
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

			// Ordered from most commonly to least commonly used
			if (lhs.cullMode != rhs.cullMode)
				return lhs.cullMode < rhs.cullMode;
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