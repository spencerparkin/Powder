#include "ContainerSizeExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ContainerSizeExpressionHandler::ContainerSizeExpressionHandler()
	{
	}

	/*virtual*/ ContainerSizeExpressionHandler::~ContainerSizeExpressionHandler()
	{
	}

	/*virtual*/ bool ContainerSizeExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 2)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"container-size-expression\" in AST to have exactly 2 children.");
			return false;
		}

		// Load the container value onto the eval stack top.  Note that we won't know until run-time if we're actually getting a container value.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1), error))
		{
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Failed to generate instructions to load container value onto the eval stack.");
			return false;
		}

		// Now compute its size whilst also popping it off.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::MathOp::SIZE;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		return true;
	}
}