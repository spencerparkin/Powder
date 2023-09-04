#include "MembershipExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"

namespace Powder
{
	MembershipExpressionHandler::MembershipExpressionHandler()
	{
	}

	/*virtual*/ MembershipExpressionHandler::~MembershipExpressionHandler()
	{
	}

	/*virtual*/ void MembershipExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->GetChildCount() != 3)
			throw new CompileTimeException("Expected \"membership-expression\" node in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0));
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2));

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::CONTAINS;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}