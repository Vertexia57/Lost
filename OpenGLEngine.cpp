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

	lost::SoundStream sound = lost::loadSoundStream("data/sound.wav");
	lost::playSoundStream(sound);
	lost::Sound sound2 = lost::loadSound("data/sound2.wav");

	int currentColor = 0;

	double time = 0.0;
	bool lowQuality = true;

	// Loops until no window open
	while (lost::windowOpen())
	{
		lost::beginFrame();

		lost::setFillColor(100, 100, 100);
		
		// Loop over the x until we hit the right side of the window
		for (int x = 0; x < lost::getWidth(); x += 50)
		{
			lost::renderLine(x, 0, x, lost::getHeight());
		}

		// Loop over the y until we hit the bottom of the window
		for (int y = 0; y < lost::getHeight(); y += 50)
		{
			lost::renderLine(0, y, lost::getWidth(), y);
		}

		lost::Color colorList[5] = {
			{ 255, 255, 255 },
			{ 255, 0,   0   },
			{ 0,   255, 0   },
			{ 0,   0,   255 },
			{ 100, 100, 100 }
		};

		if (lost::getMouseTapped(LOST_MOUSE_LEFT))
		{
			currentColor++;
			if (currentColor >= 5)
				currentColor = 0;
		}

		if (lost::getMouseTapped(LOST_MOUSE_MIDDLE))
			lost::playSoundStream(sound);
		if (lost::getMouseTapped(LOST_MOUSE_LEFT))
			lost::playSound(sound2);

		lost::setFillColor(colorList[currentColor % 5].r, colorList[currentColor % 5].g, colorList[currentColor % 5].b);
		lost::renderCircle(lost::getMousePosition(), 40);

		lost::setFillColor(colorList[(currentColor + 1) % 5].r, colorList[(currentColor + 1) % 5].g, colorList[(currentColor + 1) % 5].b);
		lost::renderCircle(lost::getMousePosition(), 32);

		lost::setFillColor(colorList[(currentColor + 2) % 5].r, colorList[(currentColor + 2) % 5].g, colorList[(currentColor + 2) % 5].b);
		lost::renderCircle(lost::getMousePosition(), 24);

		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}