#include "IdentifierExpressionHandler.h"
#include "LoadInstruction.h"
#include "Assembler.h"

namespace Powder
{
	IdentifierExpressionHandler::IdentifierExpressionHandler()
	{
	}

	/*virtual*/ IdentifierExpressionHandler::~IdentifierExpressionHandler()
	{
	}

	/*virtual*/ void IdentifierExpressionHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 1)
			throw new CompileTimeException("Expected \"identifier\" in AST to have exactly one child.", &syntaxNode->fileLocation);

		// Note that we assume we're generating code here in the context of an expression, but we're not
		// going to check that, because there's no obvious way I can think of at the moment without
		// introducing more fluff back into the AST.  Much of the fluff was removed before we were handed the AST.
		// Of course, identifiers can appear in other contexts, such as a function call or function definition.
		// Note that we also generate this code in the context of a system call, which is looking for values on the eval stack.
		const Parser::SyntaxNode* identifierNode = syntaxNode->childList.GetHead()->value;
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.string = *identifierNode->name;
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(loadInstruction);
	}
}