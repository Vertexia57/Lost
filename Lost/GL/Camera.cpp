#include "Camera.h"
#include "LostGL.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

#define PI 3.141592653

namespace lost
{

	_Camera::_Camera()
	{
	}

	_Camera::~_Camera()
	{
	}

	void _Camera::update()
	{
	}

	void _Camera::_updatePerspective(int width, int height)
	{
		if (height == 0.0f) height = 1.0f;

		m_CurrentMatrix.m_Perspective = glm::perspective(m_CurrentMatrix.m_Fov, (float)width / (float)height, m_CurrentMatrix.m_Near, m_CurrentMatrix.m_Far);
		m_ScreenspaceProj = glm::mat4x4(
			  2.0f / width,  0.0f,          0.0f,   0.0f,
			  0.0f,          -2.0f / height, 0.0f,   0.0f,
			  0.0f,          0.0f,          0.0001f,0.0f, // [!] Todo: remove depth testing for screenspace, epsilon will not be needed here afterwards
			 -1.0f,          1.0f,          0.1f,   1.0f
		);
	}

	void _Camera::setPerspective(float verticalFOV)
	{
		setPerspective(verticalFOV, (float)getWidth() / (float)getHeight(), 0.1f, 1000.0f);
	}

	void _Camera::setPerspective(float verticalFOV, float aspectRatio, float clipNear, float clipFar)
	{
		m_CurrentMatrix.m_Fov = verticalFOV / 180.0f * PI;
		m_CurrentMatrix.m_Near = clipNear;
		m_CurrentMatrix.m_Far = clipFar;

		m_CurrentMatrix.m_Perspective = glm::perspective(m_CurrentMatrix.m_Fov, aspectRatio, clipNear, clipFar);
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::setTransform(const glm::vec3& translate, const glm::vec3& rotation, const glm::vec3& scale)
	{
		m_CurrentMatrix.m_Translate = translate;
		m_CurrentMatrix.m_Rotation = rotation;
		m_CurrentMatrix.m_Scale = scale;
		m_CurrentMatrix.m_DirtyView = true;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::setTranslation(const glm::vec3& translate)
	{
		m_CurrentMatrix.m_Translate = translate;
		m_CurrentMatrix.m_DirtyView = true;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::setRotation(const glm::vec3& rotation)
	{
		m_CurrentMatrix.m_Rotation = rotation;
		m_CurrentMatrix.m_DirtyView = true;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::setScale(const glm::vec3& scale)
	{
		m_CurrentMatrix.m_Scale = scale;
		m_CurrentMatrix.m_DirtyView = true;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::lookAt(const glm::vec3& location)
	{
		m_CurrentMatrix.m_View = glm::lookAt(m_CurrentMatrix.m_Translate, location, { 0.0f, 0.0f, 1.0f });
		m_CurrentMatrix.m_View = glm::scale(m_CurrentMatrix.m_View, m_CurrentMatrix.m_Scale);
		m_CurrentMatrix.m_DirtyView = false;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::lookAtRelative(const glm::vec3& location)
	{
		m_CurrentMatrix.m_View = glm::lookAt(m_CurrentMatrix.m_Translate, location + m_CurrentMatrix.m_Translate, {0.0f, 0.0f, 1.0f});
		m_CurrentMatrix.m_View = glm::scale(m_CurrentMatrix.m_View, m_CurrentMatrix.m_Scale);
		m_CurrentMatrix.m_DirtyView = false;
		m_CurrentMatrix.m_DirtyPV = true;
	}

	void _Camera::pushMatrix()
	{
		m_MatrixStack.push(m_CurrentMatrix);
	}

	void _Camera::popMatrix()
	{
		m_CurrentMatrix = m_MatrixStack.top();
		m_MatrixStack.pop();
	}

	void _Camera::useScreenspace(bool state)
	{
		m_UsingScreenspace = state;
	}

	const glm::mat4x4& _Camera::getView()
	{
		if (m_CurrentMatrix.m_DirtyView)
		{
			m_CurrentMatrix.m_View = glm::identity<glm::mat4x4>();
			m_CurrentMatrix.m_View = glm::eulerAngleXYZ(m_CurrentMatrix.m_Rotation.x, m_CurrentMatrix.m_Rotation.y, m_CurrentMatrix.m_Rotation.z) * m_CurrentMatrix.m_View;
			m_CurrentMatrix.m_View = glm::scale(m_CurrentMatrix.m_View, m_CurrentMatrix.m_Scale);
			m_CurrentMatrix.m_View[3] += glm::vec4(m_CurrentMatrix.m_Translate, 0.0f);
			m_CurrentMatrix.m_View = glm::inverse(m_CurrentMatrix.m_View);
			
			m_CurrentMatrix.m_DirtyView = false;
			m_CurrentMatrix.m_DirtyPV = true;
		}

		if (!m_UsingScreenspace)
			return m_CurrentMatrix.m_View;
		return glm::identity<glm::mat4x4>();
	}

	const glm::mat4x4& _Camera::getProjection()
	{
		return m_CurrentMatrix.m_Perspective;
	}

	const glm::mat4x4& _Camera::getPV()
	{
		if (m_UsingScreenspace)
			return m_ScreenspaceProj;

		if (!m_CurrentMatrix.m_DirtyPV)
			return m_CurrentMatrix.m_PV;

		m_CurrentMatrix.m_PV = m_CurrentMatrix.m_Perspective * getView();
		m_CurrentMatrix.m_DirtyPV = false;
		return m_CurrentMatrix.m_PV;
	}

	void pushMatrix()
	{
		_getCurrentCamera()->pushMatrix();
	}

	void popMatrix()
	{
		_getCurrentCamera()->popMatrix();
	}

	void setCameraTransform(const glm::vec3& translate, const glm::vec3& rotation, const glm::vec3& scale)
	{
		_getCurrentCamera()->setTransform(translate, rotation, scale);
	}

	void cameraUseScreenSpace()
	{
		_getCurrentCamera()->useScreenspace(true);
	}

	void cameraUseProject()
	{
		_getCurrentCamera()->useScreenspace(false);
	}

	void setCameraFOV(float degrees)
	{
		_getCurrentCamera()->setPerspective(degrees);
	}

	void setCameraPosition(const glm::vec3& translate)
	{
		_getCurrentCamera()->setTranslation(translate);
	}

	void setCameraRotation(const glm::vec3& rotation)
	{
		_getCurrentCamera()->setRotation(rotation);
	}

	void setCameraScale(const glm::vec3& scale)
	{
		_getCurrentCamera()->setScale(scale);
	}

	void cameraLookAt(const glm::vec3& location)
	{
		_getCurrentCamera()->lookAt(location);
	}

	void cameraLookAtRelative(const glm::vec3& location)
	{
		_getCurrentCamera()->lookAtRelative(location);
	}

}