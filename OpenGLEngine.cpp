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

	int currentColor = 0;

	double time = 0.0;
	bool lowQuality = true;

	// Loops until no window open
	while (lost::windowOpen())
	{
		lost::beginFrame();

		time += lost::getDeltaTime();

		lost::setFillColor(100, 100, 100);
		
		int squareSize = 50;
		for (int x = 0; x < lost::getWidth(); x += squareSize)
		{
			for (int y = 0; y < lost::getHeight(); y += squareSize)
			{
				lost::renderRect(x, y, squareSize, squareSize);
			}
		}

		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}