#pragma once

#include <cstdint>
#include "Defines.h"
#include <list>
#include <string>
#include "HashMap.hpp"
#include "LinkedList.hpp"

namespace Powder
{
	class Instruction;

	class POWDER_API Assembler
	{
	public:
		Assembler();
		virtual ~Assembler();

		uint8_t* AssembleExecutable(const LinkedList<Instruction*>& instructionList, uint64_t& programBufferSize);

		void ResolveJumpDeltas(const LinkedList<Instruction*>& instructionList);
	};

	struct POWDER_API AssemblyData
	{
		AssemblyData()
		{
			this->programBufferLocation = 0L;
		}

		uint64_t programBufferLocation;

		struct Entry
		{
			Entry()
			{
				this->instruction = nullptr;
				this->number = 0.0;
				this->jumpDelta = 0L;
			}

			Instruction* instruction;
			double number;
			std::string string;
			uint8_t code;
			int64_t jumpDelta;
		};

		HashMap<Entry> configMap;
	};
}