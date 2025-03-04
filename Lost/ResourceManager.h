#pragma once
#include <vector>
#include <map>
#include <string>

namespace lost
{

	template <typename T>
	struct DataCount
	{
		T data;
		unsigned int count = 0;
	};

	template <typename T>
	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		void addValue(T value, const char* id);

		T getValue(const char* id) const;
		const char* getIDByValue(T value) const;

		int getValueCount() const;

		// Destroys a value within the resource manager by the ID of the value given
		// If required use destroyValueByValue() to remove the value from the manager by the value instead
		void destroyValue(const char* id);
		// Destroys a value within the resource manager
		void destroyValueByValue(T value);

		void forceDestroyValue(const char* id);
		void forceDestroyValueByValue(T value);

		inline bool hasValue(const char* id) const { return m_Data.count(id); };

		inline const std::map<std::string, DataCount<T>>& getDataMap() const { return m_Data; };
	private:
		std::map<std::string, DataCount<T>> m_Data;
	};

	template<typename T>
	inline ResourceManager<T>::ResourceManager()
	{
	}

	template<typename T>
	inline ResourceManager<T>::~ResourceManager()
	{
		for (typename std::map<std::string, DataCount<T>>::iterator it = m_Data.begin(); it != m_Data.end(); it++)
			delete it->second.data;
		m_Data.clear();
	}

	template<typename T>
	inline void ResourceManager<T>::addValue(T value, const char* id)
	{
		unsigned int count = m_Data.count(id) == 0 ? 1 : m_Data[id].count + 1;
		m_Data[id] = { value, count };
	}

	template<typename T>
	inline T ResourceManager<T>::getValue(const char* id) const
	{
		return m_Data.at(id).data;
	}

	template<typename T>
	inline const char* ResourceManager<T>::getIDByValue(T value) const
	{
		for (typename std::map<std::string, DataCount<T>>::const_iterator it = m_Data.begin(); it != m_Data.end(); it++)
		{
			if ((it->second).data == value)
				return it->first.c_str();
		}
		return nullptr;
	}

	template<typename T>
	inline int ResourceManager<T>::getValueCount() const
	{
		return m_Data.size();
	}

	template<typename T>
	inline void ResourceManager<T>::destroyValue(const char* id)
	{
		if (m_Data[id].count == 1)
		{
			delete m_Data[id].data;
			m_Data.erase(id);
			return;
		}
		m_Data[id].count--;
	}

	template<typename T>
	inline void ResourceManager<T>::destroyValueByValue(T value)
	{
		for (typename std::map<std::string, DataCount<T>>::iterator it = m_Data.begin(); it != m_Data.end(); it++)
		{
			if ((it->second).data == value)
			{
				if (it->second.count == 1)
				{
					m_Data.erase(it);
					delete value;
					break;
				}
				it->second.count--;
			}
		}
	}

	template<typename T>
	inline void ResourceManager<T>::forceDestroyValue(const char* id)
	{
		delete m_Data[id].data;
		m_Data.erase(id);
		return;
	}

	template<typename T>
	inline void ResourceManager<T>::forceDestroyValueByValue(T value)
	{
		for (typename std::map<std::string, DataCount<T>>::iterator it = m_Data.begin(); it != m_Data.end(); it++)
		{
			if ((it->second).data == value)
			{
				m_Data.erase(it);
				delete value;
				break;
			}
		}
	}

}