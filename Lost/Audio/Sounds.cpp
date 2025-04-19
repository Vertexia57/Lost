#include "Sounds.h"
#include "../Log.h"

namespace lost
{

#pragma region .wav file reading

	static const size_t _RiffWaveHeaderByteSize = 44;
	struct _RIFFWAVEHeaderData
	{
		// RIFF WAVE HEADER
		char		   riffText[4];   // "RIFF"
		unsigned int   chunkSize;     // The data of format + data + 20
		char		   waveText[4];   // "WAVE" Can be different in other RIFF files, but we require WAVE

		// FMT HEADER
		char		   fmtText[4];    // "fmt "
		unsigned int   fmtChunkSize;
		unsigned short audioFormat;   // We will throw an error if it's not equal to 1 as that means the audio is compressed
		unsigned short channelCount;
		unsigned int   sampleRate;
		unsigned int   byteRate;      // channelCount * sampleRate * bitsPerSample / 8
		unsigned short blockAlign;    // channelCount * bitsPerSample / 8 
		unsigned short bitsPerSample; // bits per sample, indicates quality

		// There are extra bytes in some files that go at the end of the fmt but they are only there if the audioFormat
		// is not PCM (1) which in our case is a requirement.

		// DATA HEADER
		char		   dataText[4];   // "data"
		unsigned int   dataChunkSize; // sampleCount * channleCount * bitsPerSample / 8

		FILE* loadedFile = nullptr;   // We will use this 
	};

	// Reads the header of a RIFF WAVE PCM file
	// The file read pointer is located at the start of the data
	_RIFFWAVEHeaderData _loadWaveFile(const char* fileLocation)
	{
		FILE* openFile;
		fopen_s(&openFile, fileLocation, "rb");

		if (openFile == nullptr)
		{
			debugLog(std::string("Wave file \"") + fileLocation + "\" failed to load, file missing or in use by another program", LOST_LOG_ERROR);
			return {};
		}

		_RIFFWAVEHeaderData outData = {};
		fread(&outData, sizeof(char), _RiffWaveHeaderByteSize, openFile);
		// _RIFFWAVEHeaderData is in the exact same format as a PCM wave header, so all we need to do is do the sanity checks

		// Sanity Checks
		if (strcmp(outData.riffText, "RIFF") != 1 ||
			strcmp(outData.waveText, "WAVE") != 1 ||
			strcmp(outData.fmtText,  "fmt ") != 1 ||
			strcmp(outData.dataText, "data") != 1)
		{
			debugLog(std::string("Wave file \"") + fileLocation + "\" failed to load, invalid format\nMust be PCM/raw .wav file (RIFF WAVE PCM)", LOST_LOG_ERROR);
			fclose(openFile); // Close file
			return {};
		}

		// Check if the format is PCM
		if (outData.audioFormat != 1)
		{
			debugLog(std::string("Wave file \"") + fileLocation + "\" failed to load, invalid format\nMust be PCM/raw .wav file (RIFF WAVE PCM)", LOST_LOG_ERROR);
			fclose(openFile); // Close file
			return {};
		}

		// Passed all checks!
		outData.loadedFile = openFile;
		return outData;
	}

#pragma endregion

	//int playRaw(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
	//	double streamTime, RtAudioStreamStatus status, void* data)
	//{
	//	_PlaybackData* playbackData = (_PlaybackData*)data;

	//	unsigned int bytesLeft    = playbackData->currentByte - playbackData->dataCount;
	//	unsigned int bytesToWrite = bytesLeft < nBufferFrames ? bytesLeft : nBufferFrames;
	//	//           ^ Calculates how many bytes it should write to the output buffer
	//	//             Equivalent of min(bytesLeft, nBufferFrames)

	//	// Write the data from the sound into the outputBuffer
	//	memcpy_s(outputBuffer, nBufferFrames, playbackData->data + playbackData->currentByte, bytesToWrite);
	//	playbackData->currentByte += bytesToWrite;

	//	// End of sound cleanup
	//	if (bytesToWrite != nBufferFrames)
	//	{
	//		// 0 out the left over bytes at the end of the buffer
	//		unsigned int leftoverBytes = nBufferFrames - bytesToWrite;
	//		memset((char*)outputBuffer + bytesToWrite, 0, leftoverBytes);
	//		return 1; // Exit playback
	//	}

	//	return 0;
	//}

	_Sound::_Sound()
		: m_SoundInfo{ 0, 0, 0, 0, 0, 0 }
		, m_Functional(false)
	{
	}

	_Sound::~_Sound()
	{
		_destroy();
	}

	void _Sound::_initializeWithFile(const char* fileLocation)
	{
		_RIFFWAVEHeaderData waveData = _loadWaveFile(fileLocation);
		if (waveData.loadedFile) // This is nullptr if it failed
		{
			unsigned int dataSize = waveData.dataChunkSize;
			m_Data = new char[dataSize];

			// Count is the amount of chars fread SUCCESSFULLY read
			unsigned int count = fread(m_Data, sizeof(char), dataSize, waveData.loadedFile);

			if (count != dataSize)
			{
				debugLog("Malformed .wav file \"" + std::string(fileLocation) + "\", was told to read more bytes than were in file", LOST_LOG_ERROR);
				_destroy();
			}
			else
			{
				m_Functional = true;
				m_SoundInfo.channelCount = waveData.channelCount;
				debugLogIf(m_SoundInfo.channelCount != 2, ".wav file: \"" + std::string(fileLocation) + "\" wasn't stereo", LOST_LOG_WARNING);
				m_SoundInfo.sampleRate = waveData.sampleRate;
				m_SoundInfo.sampleCount = waveData.dataChunkSize / (waveData.bitsPerSample / 8);

				m_SoundInfo.byteCount = waveData.dataChunkSize;
				m_SoundInfo.sampleSize = waveData.blockAlign;
				m_SoundInfo.bitsPerSample = waveData.bitsPerSample;
				m_SoundInfo.format = m_SoundInfo.bitsPerSample >> 3;
			}

			fclose(waveData.loadedFile);
		}
	}

	void _Sound::_destroy()
	{
		if (m_Data)
		{
			delete[] m_Data;
			m_Data = nullptr;
		}
		m_Functional = false;
	}

	_SoundStream::_SoundStream()
	{
	}

	_SoundStream::~_SoundStream()
	{
	}

}