#include <iostream>

#include "Lost/lost.h"
#include "Lost/lostImGui.h"

int main()
{
	lost::init(LOST_RENDER_3D);

	lost::Window A = lost::createWindow(500, 500);
	lost::setupImGui();

	while (lost::windowOpen())
	{
		lost::beginFrame(A);

		lost::imGuiDisplayProgramInfo();
		lost::endFrame();
	}

	lost::exit();
}