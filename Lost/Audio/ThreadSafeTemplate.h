#pragma once

#include <mutex>

namespace lost
{

	// A thread safe template which does not halt the read function, only the write
	// Eg. the main thread needs to write and read to the audio data and can halt
	// but the audio thread cannot halt. 
	// This allows for that
	template <typename T>
	class _HaltWrite
	{
	public:
		_HaltWrite<T>(const T& in);

		void write(const T& in);

		// Must be locked!!!
		inline T& getWriteRef()  { return m_Data[m_WriteIndex]; };
		// Must be locked!!!
		inline void forceDirty() { m_Dirty = true; };

		const T& read();

		inline std::mutex& getMutex() { return m_Mutex; };

	private:
		bool m_Dirty;

		T m_Data[2];
		unsigned char m_WriteIndex;
		unsigned char m_ReadIndex;

		std::mutex m_Mutex;
	};

	template <typename T>
	class _HaltRead
	{
	public:
		_HaltRead<T>(const T& in);

		void write(const T& in);
		const T& read();

		inline std::mutex& getMutex() { return m_Mutex; };

	private:
		T m_Data[2];
		unsigned char m_WriteIndex;
		unsigned char m_ReadIndex;

		std::mutex m_Mutex;
	};

	template<typename T>
	inline _HaltWrite<T>::_HaltWrite(const T& in)
		: m_WriteIndex(0)
		, m_ReadIndex(1)
		, m_Dirty(false)
	{
		m_Data[0] = in;
		m_Data[1] = in;
	}

	template<typename T>
	inline void _HaltWrite<T>::write(const T& in)
	{
		m_Mutex.lock();
		m_Data[m_WriteIndex] = in;
		m_Dirty = true;
		m_Mutex.unlock();
	}

	template<typename T>
	inline const T& _HaltWrite<T>::read()
	{
		T* ret;

		if (m_Mutex.try_lock())
		{
			if (m_Dirty)
			{
				m_Data[m_ReadIndex] = m_Data[m_WriteIndex];
				m_Dirty = false;
			}
			ret = &m_Data[m_ReadIndex];
			m_Mutex.unlock();
		}
		else
		{
			ret = &m_Data[m_ReadIndex];
		}

		return *ret;
	}

	template<typename T>
	inline _HaltRead<T>::_HaltRead(const T& in)
		: m_WriteIndex(0)
		, m_ReadIndex(1)
	{
		m_Data[0] = in;
		m_Data[1] = in;
	}

	template<typename T>
	inline void _HaltRead<T>::write(const T& in)
	{
		if (m_Mutex.try_lock())
		{
			m_Data[m_WriteIndex] = in;
			m_Mutex.unlock();
		}
		else
		{
			m_WriteIndex = 1 - m_WriteIndex; // Race condition
			m_Data[m_WriteIndex] = in;
		}
	}

	template<typename T>
	inline const T& _HaltRead<T>::read()
	{
		m_Mutex.lock();
		T& ret = m_Data[m_ReadIndex];
		m_ReadIndex = m_WriteIndex; // Race condition
		m_Mutex.unlock();
		return ret;
	}
}