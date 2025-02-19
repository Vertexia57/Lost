#include "Lost.h"

namespace lost
{

	void init(unsigned int rendererMode)
	{
		debugLog("Lost is currently in DEBUG mode, define LOST_RELEASE_MODE globally in project settings or compiler settings to remove extra debug features", LOST_LOG_INFO);
		lost::_initGL(rendererMode);

#ifdef LOST_DEBUG_MODE
		const lost::LostState& lostState = getLostState();
		debugLog("\n==[ Lost Initialization Settings ]==\n", LOST_LOG_NONE);
		debugLog(std::string(" - Renderer: ") + (lostState.rendererMode == LOST_RENDER_3D ? "3D" : "2D"), LOST_LOG_NONE);
		debugLog(" - Created Output Buffers: ", LOST_LOG_NONE);
		for (int i = 0; i < lostState.currentBuffers.size(); i++)
			debugLog("    - " + lostState.currentBuffers[i].name + ", location: " + std::to_string(i) + ", clear color: (" + std::to_string(lostState.currentBuffers[i].defaultColor.x) + ", " + std::to_string(lostState.currentBuffers[i].defaultColor.y) + ", " + std::to_string(lostState.currentBuffers[i].defaultColor.z) + ", " + std::to_string(lostState.currentBuffers[i].defaultColor.w) + ")", LOST_LOG_NONE);
		debugLog("\n==[ Lost Initialization Settings ]==\n", LOST_LOG_NONE);
#endif
	}

	void exit()
	{
		lost::_exitGL();
	}

}
