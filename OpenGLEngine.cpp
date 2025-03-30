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

		//lost::renderMesh(mesh, { weird }, { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 0.0f });

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

		//lost::renderLine(100, 100, 800, 200);

		//lost::renderLineStrip({ { 50, 100 }, { 100, 100 }, { 100, 50 }, {50, 50} });

		// ImGUI
		lost::imGuiDisplayProgramInfo();

		lost::endFrame();
	}

	lost::exit();
}