#include <iostream>

#include "Lost/lost.h"
#include "Lost/lostImGui.h"

int main()
{
	//lost::addOutputBuffer("normal", {0.5f, 0.5f, 0.5f, 1.0f});
	lost::init(LOST_RENDER_3D);

	lost::Window A = lost::createWindow(500, 500);
	lost::setWindowCloseCallback(A, &lost::closeAllWindows);

	lost::setupImGui();

	lost::Shader shader = lost::loadShader(nullptr, "data/fragment.frag", "phongShader");

	lost::Vec3 lightColor = { 1.0f, 0.0f, 0.0f };
	lost::setUniform(shader, (void*)lightColor.v, "lightColor", 1, 0);
	lightColor = { 0.0f, 1.0f, 0.0f };
	lost::setUniform(shader, (void*)lightColor.v, "lightColor", 1, 1);
	lightColor = { 0.0f, 0.0f, 1.0f };
	lost::setUniform(shader, (void*)lightColor.v, "lightColor", 1, 2);
	float lightIntensity = 1.0f;
	lost::setUniform(shader, (void*)&lightIntensity, "lightIntensity", 1, 0);
	lost::setUniform(shader, (void*)&lightIntensity, "lightIntensity", 1, 1);
	lost::setUniform(shader, (void*)&lightIntensity, "lightIntensity", 1, 2);

	lost::Mesh mesh = lost::loadMesh("data/cubeUV.obj");
	lost::Material weird = lost::makeMaterial(
		{
			lost::loadTexture("data/testNormal.png"),
			lost::getDefaultWhiteTexture(),
			lost::loadTexture("data/testNormal.png"),
		}, "weird", shader
	);
	lost::Material cube = lost::makeMaterial(
		{
			lost::getDefaultWhiteTexture(),
			lost::getDefaultWhiteTexture(),
			lost::getDefaultNormalTexture()
		}, "cube", shader
	);

	// Nardo Material List
	/*
	lost::Mesh mesh = lost::loadMesh("data/Nardo.obj");
	std::vector<lost::Material> nardoMaterialList =
	{
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/body.png"),
				lost::_getDefaultBlackTexture(),
				lost::loadTexture("data/Textures/bodyNorm.png"),
			}, "body", shader
		),
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/misc.png"),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "misc", shader
		),
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/eyes.png"),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "eyes", shader
		),
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/fluff.png"),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "fluff", shader
		),
		lost::makeMaterial(
			{
				lost::_getDefaultWhiteTexture(),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "specular", shader
		),
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/mawBase.png"),
				lost::loadTexture("data/Textures/mawRoughness.png"),
				lost::loadTexture("data/Textures/mawNormal.png"),
			}, "maw", shader
		),
		lost::makeMaterial(
			{
				lost::_getDefaultWhiteTexture(),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "saliva", shader
		),
		lost::makeMaterial(
			{
				lost::loadTexture("data/Textures/hair.png"),
				lost::_getDefaultBlackTexture(),
				lost::_getDefaultNormalTexture()
			}, "hair", shader
		),
	};
	*/

	lost::Vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };
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

		lost::beginFrame(A);

		uptime += lost::getDeltaTime() / 1000.0f;

		if (lost::getKeyTapped(LOST_KEY_C) && lost::getKeyDown(LOST_KEY_LEFT_SHIFT))
		{
			cameraActive = !cameraActive;
			lost::setMousePosition(lost::getWidth() / 2.0f, lost::getHeight() / 2.0f);
		}

		if (lost::getKeyTapped(LOST_KEY_R))
			cameraPosition = { 0.0f, 0.0f, 0.0f };

		float mouseOffsetX = lost::getMouseX() - int(lost::getWidth() / 2.0f);
		float mouseOffsetY = lost::getMouseY() - int(lost::getHeight() / 2.0f);

		mouseOffsetX = (abs(mouseOffsetX) >= 0.1f) ? mouseOffsetX : 0.0f;
		mouseOffsetY = (abs(mouseOffsetY) >= 0.1f) ? mouseOffsetY : 0.0f;

		if (cameraActive)
		{
			lost::setMousePosition(int(lost::getWidth() / 2.0f), int(lost::getHeight() / 2.0f));
			camYaw += mouseOffsetX / 300.0f;
			camPit += mouseOffsetY / 300.0f;
			camPit = fmax(fmin(camPit, 1.5f), -1.5f);
		}

		if (lost::getKeyTapped(LOST_KEY_ESCAPE))
			lost::closeAllWindows();

		lost::Vec3 forwardVector = lost::Vec3(sin(camYaw) * -cos(camPit), cos(camYaw) * -cos(camPit), -sin(camPit));
		lost::Vec3 rightVector = lost::Vec3(-cos(camYaw), sin(camYaw), 0.0f);

		if (lost::getKeyDown('S'))
			cameraPosition -= forwardVector * cameraSpeed * lost::getDeltaTime();
		if (lost::getKeyDown('D'))
			cameraPosition += rightVector * cameraSpeed * lost::getDeltaTime();
		if (lost::getKeyDown('W'))
			cameraPosition += forwardVector * cameraSpeed * lost::getDeltaTime();
		if (lost::getKeyDown('A'))
			cameraPosition -= rightVector * cameraSpeed * lost::getDeltaTime();
		if (lost::getKeyDown('E'))
			cameraPosition.z += cameraSpeed * lost::getDeltaTime();
		if (lost::getKeyDown('Q'))
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

		lost::renderMesh(mesh, { weird }, { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 0.0f });

		//lost::renderEllipsePro({ 100.0f, 150.0f }, { 100.0f, 50.0f }, uptime * 10.0f, weird);

		//lost::renderCircle3D({ 2.0f, 0.0f, 0.0f }, 1.0f, { 10.0f * uptime, (float)PI * uptime * 10.0f, 7.0f * uptime }, nullptr);
		//lost::renderEllipse3D({ -2.0f, 0.0f, 0.0f }, { 2.0f, 1.0f }, { 10.0f * uptime, (float)PI * uptime * 10.0f, 7.0f * uptime });

		lost::setFillColor(255, 0, 0, 255);
		lost::renderEllipsePro({ 500.0f, 500.0f }, { 500.0f, 200.0f }, { 0.0f, 0.0f }, uptime);
		lost::setFillColor(0, 255, 0, 255);
		lost::renderEllipsePro({ 500.0f, 500.0f }, { 300.0f, 200.0f }, { 0.0f, 200.0f }, uptime);
		lost::setFillColor(0, 0, 255, 255);
		lost::renderEllipsePro({ 500.0f, 500.0f }, { 300.0f, 200.0f }, { 0.0f, -200.0f }, uptime);

		lost::setFillColor(255, 125, 125, 255);
		lost::renderRectPro({ 500.0f, 500.0f, 200.0f, 200.0f }, { 100.0f, 0.0f }, uptime);
		lost::setFillColor(125, 125, 255, 255);
		lost::renderRectPro({ 500.0f, 500.0f, 200.0f, 200.0f }, { 100.0f, 200.0f }, uptime);
		lost::setFillColor(255, 255, 255, 255);
		lost::renderRectPro({ 500.0f, 500.0f, 200.0f, 200.0f }, { 100.0f, 100.0f }, uptime);

		// ImGUI
		lost::imGuiDisplayProgramInfo();

		lost::endFrame();
	}

	lost::exit();
}