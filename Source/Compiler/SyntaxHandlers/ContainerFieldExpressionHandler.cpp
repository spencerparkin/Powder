#include "ContainerFieldExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	ContainerFieldExpressionHandler::ContainerFieldExpressionHandler()
	{
	}

	/*virtual*/ ContainerFieldExpressionHandler::~ContainerFieldExpressionHandler()
	{
	}

	/*virtual*/ bool ContainerFieldExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 2)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"container-field-expression\" in AST to have exactly 2 children.");
			return false;
		}

		// Load the container value onto the eval stack top first.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0), error))
		{
			error.Add(std::string(syntaxNode->GetChild(0)->fileLocation) + "Could not generate instructions to load container value onto the eval stack.");
			return false;
		}

		// Load the field value onto the eval stack top second.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(1), error))
		{
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Could not generate instructions to load the field value onto the eval stack.");
			return false;
		}

		// Now compute the container value look-up operation.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::MathOp::GET_FIELD;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		return true;
	}
}