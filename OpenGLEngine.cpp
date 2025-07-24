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

	lost::Sound sound = lost::loadSound("data/Cascade.wav");
	lost::SoundStream soundStream = lost::loadSoundStream("data/Compass.wav");

	// Loops until no window open
	while (lost::windowOpen())
	{
		lost::beginFrame();

		lost::setFillColor(255, 255, 255);
		lost::renderRect(50, 50, 100, 100);


		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}