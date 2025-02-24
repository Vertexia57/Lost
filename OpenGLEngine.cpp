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

	lost::Shader shader = lost::loadShader("data/vertex.vert", "data/fragment.frag", "phongLighting");

	lost::Font font = lost::loadFont("data/PixeloidSans.ttf", 64.0f, "pixelFont");
	lost::Texture tex = lost::loadTexture("data/emy.png");

	lost::Mesh mayu = lost::loadMesh("data/mayu.obj", "mayu");
	lost::Texture bodytex = lost::loadTexture("data/body.png", "body");
	lost::Texture headtex = lost::loadTexture("data/head.png", "head");

	lost::Material bodyMat = lost::makeMaterial({ bodytex }, "body", shader);
	lost::Material headMat = lost::makeMaterial({ headtex }, "head", shader);

	glm::vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };

	float camYaw = 0.0f;
	float camPit = 0.0f;
	float cameraSpeed = 0.1f;
	bool cameraActive = true;

	while (lost::windowOpen())
	{

		// [--------------]
		//     Window 1  
		// [--------------]

		lost::beginFrame();
		lost::fillWindow({ 0, 0, 0, 255 });

		float mouseOffsetX = lost::getMouseX() - lost::getWidth() / 2.0f;
		float mouseOffsetY = lost::getMouseY() - lost::getHeight() / 2.0f;

		mouseOffsetX = (abs(mouseOffsetX) > 1.0f) ? mouseOffsetX : 0.0f;
		mouseOffsetY = (abs(mouseOffsetY) > 1.0f) ? mouseOffsetY : 0.0f;

		if (lost::getKeyTapped(LOST_KEY_LEFT_CONTROL))
			cameraActive = !cameraActive;

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

		lost::renderMesh(mayu, { headMat, bodyMat }, { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 0.0f });

		lost::renderQuad3D(headMat, { 5.0f, -5.0f, 0.0f }, { 10.0f, 10.0f }, { 0.0f, 180.0f, 0.0f });

		// ImGUI
		lost::imGuiDisplayProgramInfo();

		lost::endFrame();
	}

	lost::exit();
}