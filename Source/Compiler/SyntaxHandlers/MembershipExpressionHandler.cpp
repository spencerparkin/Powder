#include "MembershipExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	MembershipExpressionHandler::MembershipExpressionHandler()
	{
	}

	/*virtual*/ MembershipExpressionHandler::~MembershipExpressionHandler()
	{
	}

	/*virtual*/ bool MembershipExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 3)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"membership-expression\" node in AST to have exactly 3 children.");
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0), error))
		{
			error.Add(std::string(syntaxNode->GetChild(0)->fileLocation) + "Failed to generate instructions for left operand in membership expression.");
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2), error))
		{
			error.Add(std::string(syntaxNode->GetChild(2)->fileLocation) + "Failed to generate instructions for right operand in membership expression.");
			return false;
		}

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::CONTAINS;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		return true;
	}
}