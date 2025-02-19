#pragma once

#include "State.h"

#include "Log.h"
#include "FileIO.h"
#include "GL/LostGL.h"
#include "Input/Input.h"
#include "DeltaTime.h"

namespace lost
{
	void init(unsigned int rendererMode = LOST_RENDER_2D);
	void exit();
}