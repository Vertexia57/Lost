#pragma once
#include "glm/glm.hpp"
#include <stack>
#include "Vector.h"

namespace lost
{
	
	class _Camera
	{
	public:
		_Camera();
		~_Camera();

		void update();

		void _updatePerspective(int width, int height);
		void setPerspective(float verticalFOV);
		void setPerspective(float verticalFOV, float aspectRatio, float clipNear, float clipFar);

		void setTransform(const glm::vec3& translate, const glm::vec3& rotation, const glm::vec3& scale = {1.0f, 1.0f, 1.0f});
		void setTranslation(const glm::vec3& translate);
		void setRotation(const glm::vec3& rotation);
		void setScale(const glm::vec3& scale);
		void lookAt(const glm::vec3& location);
		void lookAtRelative(const glm::vec3& location);

		void pushMatrix();
		void popMatrix();

		void useScreenspace(bool state);

		const glm::mat4x4& getView();
		const glm::mat4x4& getProjection();
		const glm::mat4x4& getPV();

	private:

		struct ViewData
		{
			float m_Fov = 45.0f;
			float m_Near = 0.1f;
			float m_Far = 1000.0f;

			glm::mat4x4 m_View;
			glm::vec3 m_Translate = { 0.0f, 0.0f, 0.0f };
			glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
			glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };
			bool m_DirtyView = true;

			glm::mat4x4 m_Perspective;

			glm::mat4x4 m_PV;
			bool m_DirtyPV = true;
		};

		ViewData m_CurrentMatrix = {};

		bool m_UsingScreenspace = false;
		glm::mat4x4 m_ScreenspaceProj;

		std::stack<ViewData> m_MatrixStack;
	};

	// Pushes the current camera matrix to a stack which can be popped off later
	void pushMatrix();
	// Pops the matrix off the top of the stack and uses that as the new camera matrix
	void popMatrix();

	// Sets every value of the camera at once, is much more efficient than running all 3 seperately
	void setCameraTransform(const lost::Vec3& translate, const lost::Vec3& rotation, const lost::Vec3& scale = { 1.0f, 1.0f, 1.0f });
	
	// Sets the current camera's projection to screenspace, making rendering 2D (with depth testing)
	void cameraUseScreenSpace();
	// Sets the current camera's projection to projected, making rendering 3D
	void cameraUseProject();

	// Sets the vertical FOV of the camera, in degrees
	void setCameraFOV(float degrees);

	// Sets the position of the camera
	void setCameraPosition(const lost::Vec3& position);
	// Sets the rotation of the camera
	void setCameraRotation(const lost::Vec3& rotation);
	// Sets the scale of the camera (A bigger camera shrinks the world)
	void setCameraScale(const lost::Vec3& scale);
	// Sets the rotation of the camera to look at a certain point, taking the current position into account
	void cameraLookAt(const lost::Vec3& location);
	// Sets the rotation of the camera to look at a certain point, using local space
	void cameraLookAtRelative(const lost::Vec3& location);
}