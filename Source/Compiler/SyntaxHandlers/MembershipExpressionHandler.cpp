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
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"membership-expression\" node in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = MathInstruction::CONTAINS;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}