#include "DeleteFieldExpressionHandler.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	DeleteFieldExpressionHandler::DeleteFieldExpressionHandler()
	{
	}

	/*virtual*/ DeleteFieldExpressionHandler::~DeleteFieldExpressionHandler()
	{
	}

	/*virtual*/ bool DeleteFieldExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 2)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"delete-field-expression\" in AST to have exactly 2 children.");
			return false;
		}

		if (*syntaxNode->GetChild(1)->text != "container-field-expression")
		{
			error.Add(std::string(syntaxNode->GetChild(1)->fileLocation) + "Expected \"container-field-expression\" to be second child of \"delete-field-expression\" in AST.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* containerFieldNode = syntaxNode->GetChild(1);

		// Push the container value.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, containerFieldNode->GetChild(0), error))
		{
			error.Add(std::string(containerFieldNode->fileLocation) + "Failed to generate instructions to push the container value.");
			return false;
		}

		// Push the field value to delete.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, containerFieldNode->GetChild(1), error))
		{
			error.Add(std::string(containerFieldNode->fileLocation) + "Failed to generate instructions to push field value to be deleted.");
			return false;
		}

		// And now issue the del instruction.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::MathOp::DEL_FIELD;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		// Our result is the value in the container at the deleted field value.  But if no one wants it, pop it.
		if (this->PopNeededForExpression(syntaxNode))
		{
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}

		return true;
	}
}