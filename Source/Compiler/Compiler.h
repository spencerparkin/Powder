#pragma once

#include "Defines.h"
#include "VirtualMachine.h"
#include "Executable.h"
#include "Lexer.h"
#include "Parser.h"
#include "Error.h"
#include <cstdint>

namespace Powder
{
	class POWDER_API Compiler : public VirtualMachine::CompilerInterface
	{
	public:
		Compiler();
		virtual ~Compiler();

		virtual Executable* CompileCode(const char* programSourceCode, Error& error) override;

		bool generateDebugInfo;

	private:

		bool PerformReductions(ParseParty::Parser::SyntaxNode* parentNode);
		bool PerformSugarExpansions(ParseParty::Parser::SyntaxNode* parentNode);
	};

	extern Compiler theDefaultCompiler;
}