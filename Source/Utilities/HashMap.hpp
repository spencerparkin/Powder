#pragma once

#include "Defines.h"
#include <cinttypes>
#include <functional>
#include <string.h>

namespace Powder
{
	template<typename T>
	class POWDER_API HashMap
	{
		struct TableEntry;

	public:

		HashMap()
		{
			this->numEntries = 0;
			this->tableSize = 1024;
			this->table = new TableEntry*[this->tableSize];
			for (uint32_t i = 0; i < this->tableSize; i++)
				this->table[i] = nullptr;
		}

		virtual ~HashMap()
		{
			this->Clear();

			delete[] this->table;
		}

		virtual uint32_t CalcHash(const char* key)
		{
			// https://en.wikipedia.org/wiki/Jenkins_hash_function
			uint32_t hash = 0;
			int i = 0;
			while (key[i] != '\0')
			{
				hash += key[i++];
				hash += hash << 10;
				hash ^= hash >> 6;
			}
			hash += hash << 3;
			hash ^= hash >> 11;
			hash += hash << 15;
			return hash;
		}

		uint32_t NumEntries() const
		{
			return this->numEntries;
		}

		void DeleteAndClear()
		{
			this->ForAllEntries([](const char* key, T data) -> bool {
				delete data;
				return true;
			});

			this->Clear();
		}

		void Clear()
		{
			for (uint32_t i = 0; i < this->tableSize; i++)
			{
				TableEntry* entry = this->table[i];
				while (entry)
				{
					TableEntry* nextEntry = entry->nextEntry;
					delete entry;
					entry = nextEntry;
				}

				this->table[i] = nullptr;
			}

			this->numEntries = 0;
		}

		void Insert(const char* key, T data)
		{
			TableEntry* entry = this->FindEntry(key, true);
			entry->data = data;
			this->numEntries++;
		}

		T Lookup(const char* key)
		{
			TableEntry* entry = this->FindEntry(key, false);
			return entry ? entry->data : nullptr;
		}

		const T lookup(const char* key) const
		{
			TableEntry* entry = const_cast<HashMap<T>*>(this)->FindEntry(key, false);
			return entry ? entry->data : nullptr;
		}

		T* LookupPtr(const char* key)
		{
			TableEntry* entry = this->FindEntry(key, false);
			return entry ? &entry->data : nullptr;
		}

		const T* LookupPtr(const char* key) const
		{
			TableEntry* entry = this->FindEntry(key, false);
			return entry ? &entry->data : nullptr;
		}

		void Remove(const char* key)
		{
			uint32_t i = 0;
			TableEntry* doomedEntry = this->FindEntry(key, false, &i);
			if (doomedEntry)
			{
				TableEntry* entry = this->table[i];
				if (entry == doomedEntry)
					this->table[i] = entry->nextEntry;
				else
				{
					while (entry->nextEntry != doomedEntry)
						entry = entry->nextEntry;
					entry->nextEntry = doomedEntry->nextEntry;
				}

				delete doomedEntry;
				this->numEntries--;
			}
		}

		void ForAllEntries(std::function<bool(const char* key, T& data)> callback)
		{
			for (uint32_t i = 0; i < this->tableSize; i++)
			{
				TableEntry* entry = this->table[i];
				while (entry)
				{
					if (!callback(entry->key, entry->data))
						return;

					entry = entry->nextEntry;
				}
			}
		}

		struct POWDER_API iterator
		{
			HashMap<T>* hashMap;
			uint32_t i;
			TableEntry* entry;

			iterator()
			{
				this->hashMap = nullptr;
				this->i = -1;
				this->entry = nullptr;
			}

			void operator++()
			{
				if (this->entry)
					this->entry = this->entry->nextEntry;
				while (this->entry == nullptr && this->i < this->hashMap->tableSize - 1)
					this->entry = this->hashMap->table[++this->i];
			}

			T& operator*()
			{
				return this->entry->data;
			}

			bool operator==(const iterator& iter) const
			{
				return this->hashMap == iter.hashMap && this->i == iter.i && this->entry == iter.entry;
			}
		};

		// By using the begin() and end() methods, this class becomes compatible
		// with C++'s for-loop special syntax: for(auto item : container), I think.

		iterator begin()
		{
			iterator iter;
			iter.hashMap = this;
			iter.i = 0;
			iter.entry = nullptr;
			++iter;
			return iter;
		}

		iterator end()
		{
			iterator iter;
			iter.hashMap = this;
			iter.i = (int32_t)this->tableSize - 1;
			iter.entry = nullptr;
			return iter;
		}

	private:

		TableEntry* FindEntry(const char* key, bool canCreateIfNotFound, uint32_t* tableOffset = nullptr)
		{
			uint32_t hash = this->CalcHash(key);
			uint32_t i = hash % this->tableSize;
			if (tableOffset)
				*tableOffset = i;

			TableEntry* entry = this->table[i];
			while (entry)
			{
				if (0 == ::strcmp(entry->key, key))
					break;
				entry = entry->nextEntry;
			}
			
			if (!entry && canCreateIfNotFound)
			{
				entry = new TableEntry;
				::strcpy_s(entry->key, sizeof(entry->key), key);
				entry->nextEntry = this->table[i];
				this->table[i] = entry;
			}

			return entry;
		}

		struct TableEntry
		{
			T data;
			char key[128];
			TableEntry* nextEntry;
		};

		TableEntry** table;
		uint32_t tableSize;
		uint32_t numEntries;
	};
}