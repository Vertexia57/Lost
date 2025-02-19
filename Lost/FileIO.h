#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "Log.h"

namespace lost
{

	// Loads the text file at fileDir and returns the data that file contains
	static std::string loadfile(const char* fileDir)
	{
		std::string out;

		std::ifstream file;
		std::stringstream outbuffer;
		std::string line;
		file.open(fileDir);

		if (!file.is_open()) // File failed to open
		{
			log(std::string("File at \"") + fileDir + "\" failed to load.\nFile may be open by another program, missing or inaccessible.", LOST_LOG_ERROR);
			return out; // Exit the function, don't want to process anything else
		}
		else // Only iterate over file if file is open
		{
			while (std::getline(file, line)) // Loop over file data and add it to output
				outbuffer << line << "\n";
			out = outbuffer.str();
			out.erase(out.end() - 1); // Remove extra \n that was added from before
		}

		file.close(); // Close file 
		debugLog(std::string("Successfully loaded text file at \"") + fileDir + "\"", LOST_LOG_SUCCESS);
		return out;
	}

}