#include <iostream>

#include "Lost/lost.h"
#include "Lost/lostImGui.h"

// Test Notes:
//  > fill color wasn't automatically white?
//  > allow for lost::setFillColor to take lost::Color

int main()
{
	// Initializes Lost
	lost::init(LOST_RENDER_3D);

	// Creates a 500x500 pixel window
	lost::createWindow(500, 500);
	// Initializes ImGui
	lost::setupImGui();

	lost::Sound sound = lost::loadSound("data/sound2.wav");
	lost::SoundStream sound2 = lost::loadSoundStream("data/Compass.wav");

	lost::RenderTexture rt = lost::RenderTexture(500, 500);
	lost::RenderTexture rt2 = lost::RenderTexture(300, 300);

	// Loops until no window open
	while (lost::windowOpen())
	{
		lost::beginFrame();

		lost::setFillColor(100, 100, 100);
		int squareSize = 50;
		for (int x = 0; x < lost::getWidth(); x += squareSize)
		{
			for (int y = 0; y < lost::getHeight(); y += squareSize)
			{
				lost::renderRect(x+1, y+1, squareSize -2, squareSize - 2);
			}
		}

		lost::setFillColor(255, 255, 255);
		rt.bind();

		rt.clear();
		lost::renderRect(lost::getMouseX() - 100, lost::getMouseY() - 100, 300, 300);

		rt2.bind();

		rt2.clear();
		lost::setFillColor(255, 0, 0);
		lost::renderRect(lost::getMouseX() - 200, lost::getMouseY() - 200, 300, 300);

		rt2.unbind();
		lost::setFillColor(255, 255, 255);
		lost::renderTexture(rt2.getTexture(0), { 100, 100, 300, 300 });

		rt.unbind();
		lost::setFillColor(255, 255, 255);
		lost::renderTexture(rt.getTexture(0), { 100, 100, 1000, 500 });

		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}