#include <iostream>

#include "Lost/lost.h"
#include "Lost/lostImGui.h"

int main()
{
	lost::init(LOST_RENDER_3D);

	lost::Window A = lost::createWindow(500, 500);
	lost::setupImGui();

	lost::Font font = lost::loadFont("data/PixeloidSans.ttf", 32);

	while (lost::windowOpen())
	{
		lost::beginFrame(A);

		lost::setFillColor(0, 255, 0);
		lost::renderText("I've come to make an announcement;\nShadow The Hedgehog's a bitch ass motherfucker,\nhe pissed on my fucking wife.\nThats right, he took his hedgehog quilly dick out and he pissed on my fucking wife,\nand he said his dick was \"This big\" and I said that's disgusting,\nso I'm making a callout post on my twitter dot com,\nShadow the Hedgehog, you've got a small dick, it's the size of this walnut except WAY smaller,\nand guess what? Here's what my dong looks like: PFFFT, THAT'S RIGHT, BABY. ALL POINTS, NO QUILLS, NO PILLOWS.\nLook at that, it looks like two balls and a bong. He fucked my wife so guess what? I'm gonna fuck the Earth.\nTHAT'S RIGHT THIS IS WHAT YOU GET, MY SUPER LASER PISS! Except I'm not gonna piss on the earth.\nI'm gonna go higher. I'M PISSING ON THE MOON! HOW DO YOU LIKE THAT, OBAMA?\nI PISSED ON THE MOON YOU IDIOT! YOU HAVE 23 HOURS BEFORE THE PISS DROPLETS HIT THE FUCKING EARTH\nNOW GET OUT OF MY SIGHT BEFORE I PISS ON YOU TOO.", font, { lost::getWidth() / 2.0f, lost::getHeight() / 2.0f }, 0.5f, LOST_TEXT_ALIGN_MIDDLE, LOST_TEXT_ALIGN_MIDDLE);

		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}