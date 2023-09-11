#pragma once

#include <cstdint>
#include "Defines.h"
#include <list>
#include <string>
#include "Lexer.h"
#include "HashMap.hpp"
#include "LinkedList.hpp"
#include "Error.h"

namespace Powder
{
	class Instruction;
	class Executable;

	class POWDER_API Assembler
	{
	public:
		Assembler();
		virtual ~Assembler();

		Executable* AssembleExecutable(const LinkedList<Instruction*>& instructionList, bool generateDebugInfo, Error& error);

	private:

		bool ResolveJumps(const LinkedList<Instruction*>& instructionList, Error& error);
	};

	struct POWDER_API AssemblyData
	{
		AssemblyData()
		{
			this->programBufferLocation = 0L;
			this->fileLocation.line = -1;
			this->fileLocation.column = -1;
			this->debuggerHelp = new std::string;
		}

		virtual ~AssemblyData()
		{
			delete this->debuggerHelp;
		}

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
				this->offset = 0;
				this->jumpDelta = 0L;
				this->ptr = nullptr;
			}

			Instruction* instruction;
			double number;
			std::string string;
			uint8_t code;
			int32_t offset;
			int64_t jumpDelta;
			void* ptr;
		};

		uint64_t programBufferLocation;
		HashMap<Entry> configMap;
		ParseParty::Lexer::FileLocation fileLocation;
		std::string* debuggerHelp;
	};
}