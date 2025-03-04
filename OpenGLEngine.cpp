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

	glm::vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };
	float camYaw = 0.0f;
	float camPit = 0.0f;
	float cameraSpeed = 0.1f;
	bool cameraActive = true;

	lost::Mesh nardoMesh = lost::loadMesh("data/Nardo.obj");
	lost::Material matTest = lost::makeMaterial({ lost::loadTexture("data/emy.png") }, "emy", shader);
	std::vector<lost::Material> nardoMaterials = lost::loadMaterialsFromOBJMTL("data/Nardo.obj");

	while (lost::windowOpen())
	{

		// [--------------]
		//     Window 1  
		// [--------------]

		lost::beginFrame();
		lost::fillWindow({ 0, 0, 0, 255 });

		if (lost::getKeyTapped(LOST_KEY_LEFT_CONTROL))
		{
			cameraActive = !cameraActive;
			lost::setMousePosition(lost::getWidth() / 2.0f, lost::getHeight() / 2.0f);
		}

		float mouseOffsetX = lost::getMouseX() - lost::getWidth() / 2.0f;
		float mouseOffsetY = lost::getMouseY() - lost::getHeight() / 2.0f;

		mouseOffsetX = (abs(mouseOffsetX) > 1.0f) ? mouseOffsetX : 0.0f;
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

		lost::setCullMode(LOST_CULL_NONE);
		lost::beginMesh();
		lost::addVertex(lost::Vec3{ 0.0f, 0.0f, 0.0f });
		lost::addVertex(lost::Vec3{ 1.0f, 0.0f, 0.0f });
		lost::addVertex(lost::Vec3{ 1.0f, 1.0f, 0.0f });
		lost::setFillColor(255, 0, 0);
		lost::addVertex(lost::Vec3{ 1.0f, 0.0f, 1.0f });
		lost::addVertex(lost::Vec3{ 0.0f, 0.0f, 0.0f });
		lost::addVertex(lost::Vec3{ 1.0f, 0.0f, 0.0f });
		lost::setFillColor(255, 255, 255);
		lost::endMesh();
		lost::setCullMode(LOST_CULL_AUTO);

		lost::renderMesh(nardoMesh, nardoMaterials, { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 00.0f });

		//lost::setFillColor(255, 0, 255, 255);
		//lost::renderRect(nardoMaterials[0], { 100.0f, 100.0f, 300.0f, 300.0f });


		// ImGUI
		lost::imGuiDisplayProgramInfo();

		lost::endFrame();
	}

	lost::exit();
}