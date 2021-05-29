#pragma once

namespace Powder
{
	class Assembler
	{
	public:
		Assembler();
		virtual ~Assembler();

		// TODO: We should be able to take a sequence (array) of instruction instances
		//       and produce from that the final binary executable buffer.  These instances
		//       should be given some assemble-time data they can use to render themselves
		//       out appropriately.  Two passes are made.  In the first pass, all instructions
		//       get located in the buffer.  In the second pass, links between instructions,
		//       if any, are resolved.
	};
}