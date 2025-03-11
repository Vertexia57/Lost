#include <iostream>

#include "Lost/lost.h"
#include "Lost/lostImGui.h"

int main()
{
	lost::addOutputBuffer("normal", {0.5, 0.5, 1.0f, 1.0f});
	lost::init(LOST_RENDER_3D);

	lost::createWindow(500, 500);
	lost::useVSync(true);

	lost::setupImGui();

	lost::Shader shader = lost::loadShader(nullptr, "data/fragment.frag", "phongShader");

	lost::Mesh mesh = lost::loadMesh("data/cubeUV.obj");
	lost::Material mat = lost::makeMaterial({ lost::loadTexture("data/BrickAlbedo.png"), lost::_getDefaultWhiteTexture(), lost::loadTexture("data/BrickNormal.png")}, "WhiteMat", shader);

	glm::vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };
	float camYaw = 0.0f;
	float camPit = 0.0f;
	float cameraSpeed = 0.1f;
	bool cameraActive = true;

	float uptime = 0.0f;

	while (lost::windowOpen())
	{

		// [--------------]
		//     Window 1  
		// [--------------]

		lost::beginFrame();

		uptime += lost::getDeltaTime() / 1000.0f;

		if (lost::getKeyTapped(LOST_KEY_C))
		{
			cameraActive = !cameraActive;
			lost::setMousePosition(lost::getWidth() / 2.0f, lost::getHeight() / 2.0f);
		}

		if (lost::getKeyTapped(LOST_KEY_R))
			cameraPosition = { 0.0f, 0.0f, 0.0f };

		float mouseOffsetX = lost::getMouseX() - lost::getWidth() / 2.0f;
		float mouseOffsetY = lost::getMouseY() - lost::getHeight() / 2.0f;

		mouseOffsetX = (abs(mouseOffsetX) > 0.1f) ? mouseOffsetX : 0.0f;
		mouseOffsetY = (abs(mouseOffsetY) > 1.0f) ? mouseOffsetY : 0.0f;

		if (cameraActive)
		{
			lost::setMousePosition(lost::getWidth() / 2.0f, lost::getHeight() / 2.0f);
			camYaw += mouseOffsetX / 300.0f;
			camPit += mouseOffsetY / 300.0f;
			camPit = fmax(fmin(camPit, 1.5f), -1.5f);
		}

		if (lost::getKeyTapped(LOST_KEY_ESCAPE))
			lost::closeAllWindows();

		glm::vec3 forwardVector = glm::vec3(sin(camYaw) * -cos(camPit), cos(camYaw) * -cos(camPit), -sin(camPit));
		glm::vec3 rightVector = glm::vec3(-cos(camYaw), sin(camYaw), 0.0f);

		if (lost::getKeyDown('S'))
			cameraPosition -= forwardVector * cameraSpeed;
		if (lost::getKeyDown('D'))
			cameraPosition += rightVector * cameraSpeed;
		if (lost::getKeyDown('W'))
			cameraPosition += forwardVector * cameraSpeed;
		if (lost::getKeyDown('A'))
			cameraPosition -= rightVector * cameraSpeed;
		if (lost::getKeyDown(LOST_KEY_SPACE))
			cameraPosition.z += cameraSpeed;
		if (lost::getKeyDown(LOST_KEY_LEFT_SHIFT))
			cameraPosition.z -= cameraSpeed;

		cameraSpeed *= pow(2, lost::getMouseScroll());

		lost::setCameraPosition(cameraPosition);
		lost::cameraLookAtRelative(forwardVector);

		lost::setUniform(shader, &cameraPosition.x, "cameraPosition");

		lost::Vec3 lightPos = { 2 * sin(uptime), 2 * cos(uptime), 0.0f };
		lost::setUniform(shader, (void*)lightPos.v, "lightData", 1, 0);
		lightPos = { 0.0f, 2 * sin(uptime), 2 * cos(uptime) };
		lost::setUniform(shader, (void*)lightPos.v, "lightData", 1, 1);
		lightPos = { 2 * cos(uptime), 2 * sin(uptime), 2 * sin(uptime) };
		lost::setUniform(shader, (void*)lightPos.v, "lightData", 1, 2);

		lost::renderMesh(mesh, { mat }, { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 0.0f });

		// ImGUI
		lost::imGuiDisplayProgramInfo();

		lost::endFrame();
	}

	lost::exit();
}