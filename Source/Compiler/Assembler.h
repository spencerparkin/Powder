#pragma once

#include <cstdint>
#include "Defines.h"
#include <list>
#include <string>
#include "HashMap.hpp"
#include "LinkedList.hpp"
#include "Exceptions.hpp"

namespace Powder
{
	class Instruction;
	class Executable;

	class POWDER_API Assembler
	{
	public:
		Assembler();
		virtual ~Assembler();

		Executable* AssembleExecutable(const LinkedList<Instruction*>& instructionList, bool generateDebugInfo);

	private:

		void ResolveJumps(const LinkedList<Instruction*>& instructionList);
	};

	struct POWDER_API AssemblyData
	{
		AssemblyData()
		{
			this->programBufferLocation = 0L;
			this->fileLocation.lineNumber = -1;
			this->fileLocation.columnNumber = -1;
			this->debuggerHelp = new std::string;
		}

		virtual ~AssemblyData()
		{
			delete this->debuggerHelp;
		}

		uint64_t programBufferLocation;

		struct Entry
		{
			Entry()
			{
				this->Reset();
			}

			void Reset()
			{
				this->instruction = nullptr;
				this->number = 0.0;
				this->string = "";
				this->code = 0;
				this->jumpDelta = 0L;
			}

			Instruction* instruction;
			double number;
			std::string string;
			uint8_t code;
			int64_t jumpDelta;
		};

		HashMap<Entry> configMap;
		FileLocation fileLocation;
		std::string* debuggerHelp;
	};
}