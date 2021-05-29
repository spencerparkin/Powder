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
			std::hash<const char*> hash;
			return hash(key);
		}

		uint32_t NumEntries() const
		{
			return this->numEntries;
		}

		void DeleteAndClear()
		{
			this->ForAllEntries([](T data) -> bool {
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
		}

		T Lookup(const char* key)
		{
			TableEntry* entry = this->FindEntry(key, false);
			return entry ? entry->data : nullptr;
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
			}
		}

		void ForAllEntries(std::function<bool(T data)> callback)
		{
			for (uint32_t i = 0; i < this->tableSize; i++)
			{
				TableEntry* entry = this->table[i];
				while (entry)
				{
					if (!callback(entry->data))
						return;

					entry = entry->nextEntry;
				}
			}
		}

	private:

		struct TableEntry;

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