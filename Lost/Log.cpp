#include "Log.h"
#include <cassert>
#include <Windows.h>
#include <iostream>

namespace lost
{

	bool _logHasContext = false;
	std::string _logContext = "";

	const char* _logLevelNames[7] =
	{
		"",
		" Success ",
		" Info ",
		" Warning ",
		" Warning ",
		" Error ",
		" FATAL "
	};

#ifdef LOST_DEBUG_MODE
	const char* _terminalSequnceCodes[7] =
	{
		"",
		"\x1B[38;2;50;255;75m",  // Success: Green
		"\x1B[38;2;100;150;255m",//    Info: Blue
		"\x1B[38;2;255;255;50m", // Warning: Yellow
		"\x1B[38;2;255;255;50m", // Warning: Yellow
		"\x1B[38;2;255;100;50m", //   Error: Light Red
		"\x1B[38;2;255;0;0m"     //   Fatal: Deep Red
	};
#else
	const char* _terminalSequnceCodes[7] =
	{
		"",
		"",
		"",
		"",
		"",
		"",
		""
	};
#endif 

	void setLogContext(std::string context)
	{
		_logHasContext = true;
		_logContext = context;
	}

	void clearLogContext()
	{
		_logHasContext = false;
	}

	void log(std::string text, int level)
	{
		std::string errorMsg;

		if (level >= LOST_LOG_WARNING) // Warning or greater
		{

			if (_logHasContext)
				errorMsg = std::string("=[") + _logLevelNames[level] + "]==================================\n\n" + text + "\nContext: " + _logContext + "\n\n=[" + _logLevelNames[level] + "]==================================";
			else
				errorMsg = std::string("=[") + _logLevelNames[level] + "]==================================\n\n" + text + "\n\n=[" + _logLevelNames[level] + "]==================================";

			std::cout << "\n" << _terminalSequnceCodes[level] << errorMsg << "\x1B[0m\n" << std::endl;

			if (level >= LOST_LOG_ERROR)
			{
				std::wstring wStrErrorText = std::wstring(errorMsg.begin(), errorMsg.end());
				MessageBox(NULL, wStrErrorText.c_str(), L"Error!", MB_ICONERROR | MB_OK);
			}

			if (level == LOST_LOG_FATAL)
				assert(false);

		}
		else // Info or Success
		{
			if (level != LOST_LOG_NONE)
				errorMsg = std::string("[") + _logLevelNames[level] + "] " + text;
			else
				errorMsg = text;

			std::cout << _terminalSequnceCodes[level] << errorMsg << "\x1B[0m" << std::endl;
		}
	}

	void log(std::string text, int level, int line, const char* file)
	{
		if (level < LOST_LOG_WARNING)
			log(text, level);
		else
		{
			log(text + "\n\nLine: " + std::to_string(line) + "\nFile: " + file, level);
		}
	}

}