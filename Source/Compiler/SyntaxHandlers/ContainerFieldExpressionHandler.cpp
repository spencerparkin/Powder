#include "ContainerFieldExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"

namespace Powder
{
	ContainerFieldExpressionHandler::ContainerFieldExpressionHandler()
	{
	}

	/*virtual*/ ContainerFieldExpressionHandler::~ContainerFieldExpressionHandler()
	{
	}

	/*virtual*/ void ContainerFieldExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->GetChildCount() != 2)
			throw new CompileTimeException("Expected \"container-field-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

		// Load the container value onto the eval stack top first.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0));

		// Load the field value onto the eval stack top second.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1));

		// Now compute the container value look-up operation.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::MathOp::GET_FIELD;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}