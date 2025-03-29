#pragma once
#include <string>
#include <vector>

#include "State.h"

#ifdef LOST_DEBUG_MODE
	// The size of the log queue
	#define LOST_LOG_QUEUE_SIZE 500
	// Automatically removed if LOST_RELEASE_MODE is defined, currently LOST_DEBUG_MODE is defined
	#define debugLog(text, level) lost::log(text, level, __LINE__, __FILE__)
	// Automatically removed if LOST_RELEASE_MODE is defined, currently LOST_DEBUG_MODE is defined
	#define debugLogIf(condition, text, level) if (condition) lost::log(text, level, __LINE__, __FILE__)
#else
	#define LOST_LOG_QUEUE_SIZE 0 // The size of the log queue
	#define debugLog(text, level) {}// LOST_RELEASE_MODE is currently defined, this line does nothing right now.
	#define debugLogIf(condition, text, level) {}// LOST_RELEASE_MODE is currently defined, this line does nothing right now.
#endif

enum LogLevel
{
	LOST_LOG_NONE,
	LOST_LOG_SUCCESS,
	LOST_LOG_INFO,
	LOST_LOG_WARNING_NO_NOTE,
	LOST_LOG_WARNING,
	LOST_LOG_ERROR,
	LOST_LOG_FATAL
};

namespace lost
{
	extern const char* _logLevelNames[7];

	struct _Log
	{
		std::string logText;
		unsigned int level;

		// Optional
		int line = -1;
		const char* file = nullptr;

		// [?] TODO: Time?
	};

	const std::vector<_Log>& _getLogList();

	// Should not be accessed by the user
	extern bool _logHasContext;
	extern std::string _logContext;

	// Add context for the log function, helps with error messages, can be cleared with clearLogContext()
	extern void setLogContext(std::string context); 
	// Clears the log context, does nothing unless setLogContext() has been ran
	extern void clearLogContext();

	// Log function, uses LOST_LOG_INFO by default, follows the LogLevel enum
	extern void log(std::string text, int level);
	// Log function, uses LOST_LOG_INFO by default, follows the LogLevel enum, includes line and file
	extern void log(std::string text, int level, int line, const char* file);
	
}