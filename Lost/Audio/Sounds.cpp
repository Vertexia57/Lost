#include "Sounds.h"
#include "../Log.h"

#include "Audio.h"

#include <thread>
#include <mutex>

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
				m_SoundInfo.sampleRate = waveData.sampleRate;
				m_SoundInfo.sampleCount = waveData.dataChunkSize / (waveData.bitsPerSample / 8) / waveData.channelCount;

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

	_SoundStream::_SoundStream(unsigned int bufferSize)
		: m_BufferSize(bufferSize)
		, a_UsingBBuffer(false)
		, m_File(nullptr)
		, m_Functional(false)
		, m_SoundInfo{ 0, 0, 0, 0, 0, 0 }
		, a_Playing{ false }
		, m_Active(false)
		, a_LoopCount(0)
		, a_Volume{ 1.0f }
		, a_Panning{ 0.0f }
	{
		a_Buffer = nullptr;
		a_CurrentByte = 0;
	}

	_SoundStream::~_SoundStream()
	{
		_destroy();
	}

	void _SoundStream::_initializeWithFile(const char* fileLocation)
	{
		_RIFFWAVEHeaderData waveData = _loadWaveFile(fileLocation);
		if (waveData.loadedFile) // This is nullptr if it failed
		{
			m_Functional = true;
			m_SoundInfo.channelCount = waveData.channelCount;
			m_SoundInfo.sampleRate = waveData.sampleRate;
			m_SoundInfo.sampleCount = waveData.dataChunkSize / (waveData.bitsPerSample / 8) / waveData.channelCount;

			m_SoundInfo.byteCount = waveData.dataChunkSize;
			m_SoundInfo.sampleSize = waveData.blockAlign;
			m_SoundInfo.bitsPerSample = waveData.bitsPerSample;
			m_SoundInfo.format = m_SoundInfo.bitsPerSample >> 3;

			m_File = waveData.loadedFile;

			m_ByteSize = m_BufferSize * m_SoundInfo.channelCount * (m_SoundInfo.bitsPerSample / 8);
			a_Buffer = new char[m_ByteSize * 2]; // * 2 because we are dual buffering, this just helps with "hot-memory"

			// Bytes per channel's samples
			unsigned int soundFormat = m_SoundInfo.format;
			// The bit selected is the amount of bytes per channel sample
			unsigned int audioFormat = _getAudioHandlerFormat();

			a_FormatFactor = soundFormat - (log2(audioFormat) + 1);

			// Fill the buffer with the starting information
			fread_s(a_Buffer, m_ByteSize, sizeof(char), m_ByteSize, m_File);
			a_UsingBBuffer = true;
		}
	}

	void _SoundStream::_destroy()
	{
		if (a_Buffer)
		{
			delete[] a_Buffer;
			a_Buffer = nullptr;
		}
		m_Functional = false;
		fclose(m_File);
	}

	unsigned int _SoundStream::_getCurrentByte() const
	{
		return a_CurrentByte;
	}

	unsigned int _SoundStream::_getDataByteSize() const
	{
		return m_SoundInfo.byteCount;
	}

	unsigned int _SoundStream::_getDataBlockSize() const
	{
		return m_ByteSize;
	}

	unsigned int _SoundStream::_getBytesLeftToPlay() const
	{
		if (a_LoopCount > 0)
			return UINT_MAX;
		return m_SoundInfo.byteCount - a_CurrentByte;
	}

	unsigned int _SoundStream::_getFormatFactor() const
	{
		return a_FormatFactor;
	}

	unsigned int _SoundStream::_getLoopCount() const
	{
		return a_LoopCount;
	}

	const char* _SoundStream::_getNextDataBlock()
	{
		// Check if the other buffer is currently being filled

		if (a_FillingBufferMutex.try_lock())
		{
			// If it isn't be immediately unlock it and start a _fillBuffer thread
			// which then locks it until it's filled the other buffer

			// Increase current byte by buffer size
			if (m_SoundInfo.byteCount > a_CurrentByte + m_ByteSize)
			{
				a_CurrentByte += m_ByteSize;
			}
			else
			{
				if (a_LoopCount > 0)
				{
					a_CurrentByte += m_ByteSize;
					a_CurrentByte -= m_SoundInfo.byteCount;
					if (a_LoopCount != UINT_MAX)
						a_LoopCount--;
				}
				else
					a_CurrentByte = m_SoundInfo.byteCount;
			}

			// Swap buffers and start a fill buffer thread
			a_UsingBBuffer = !a_UsingBBuffer;

			a_FillingBufferMutex.unlock();
			a_FillingBufferThread = std::thread(&_SoundStream::_fillBuffer, this);
			a_FillingBufferThread.detach();
		}

		// If it is we reuse the old data, this does cause audio glitching

		// If using B buffer return B buffer, otherwise return A buffer
		return a_UsingBBuffer ? a_Buffer + m_ByteSize : a_Buffer;
	}

	void _SoundStream::_prepareStartPlay(float volume, float panning, unsigned int loopCount)
	{
		fseek(m_File, 44, SEEK_SET);
		fread_s(a_Buffer, m_ByteSize, sizeof(char), m_ByteSize, m_File);
		a_CurrentByte = 0;
		a_UsingBBuffer = true;
		a_LoopCount = loopCount;
		a_Volume.write(volume);
		a_Panning.write(panning);
	}

	void _SoundStream::_fillBuffer()
	{
		a_FillingBufferMutex.lock();

		// This just gets the start byte of the buffer to start writing in (The opposite of the get function)
		char* writeStartByte = a_UsingBBuffer ? a_Buffer : a_Buffer + m_ByteSize;

		// Loop processing
		bool reachEndOfData = m_SoundInfo.byteCount < a_CurrentByte + m_ByteSize;
		unsigned int readCount = reachEndOfData ? m_SoundInfo.byteCount - a_CurrentByte : m_ByteSize;

		fread_s(writeStartByte, readCount, sizeof(char), readCount, m_File);

		// Loop back to the start of the data in the file if looping
		if (reachEndOfData && a_LoopCount > 0)
		{
			fseek(m_File, 44, SEEK_SET);
			fread_s(writeStartByte + readCount, m_ByteSize - readCount, sizeof(char), m_ByteSize - readCount, m_File);
		}

		a_FillingBufferMutex.unlock();
	}

}