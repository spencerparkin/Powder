#include "ContainerSizeExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ContainerSizeExpressionHandler::ContainerSizeExpressionHandler()
	{
	}

	/*virtual*/ ContainerSizeExpressionHandler::~ContainerSizeExpressionHandler()
	{
	}

	/*virtual*/ void ContainerSizeExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 2)
			throw new CompileTimeException("Expected \"container-size-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

		// Load the container value onto the eval stack top.  Note that we won't know until run-time if we're actually getting a container value.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);

		// Now compute its size whilst also popping it off.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::MathOp::SIZE;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}