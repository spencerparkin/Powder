#include "UnaryExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"

namespace Powder
{
	UnaryExpressionHandler::UnaryExpressionHandler()
	{
	}

	/*virtual*/ UnaryExpressionHandler::~UnaryExpressionHandler()
	{
	}

	/*virtual*/ void UnaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		const ParseParty::Parser::SyntaxNode* operationNode = nullptr;
		const ParseParty::Parser::SyntaxNode* operandNode = nullptr;

		AssemblyData::Entry entry;

		if (*syntaxNode->name == "left-unary-expression")
		{
			operationNode = syntaxNode->childList.GetHead()->value;
			operandNode = syntaxNode->childList.GetHead()->GetNext()->value;
			entry.code = MathInstruction::TranslateUnaryLeftOperatorToken(*operationNode->name);
		}
		else if (*syntaxNode->name == "right-unary-expression")
		{
			operationNode = syntaxNode->childList.GetHead()->GetNext()->value;
			operandNode = syntaxNode->childList.GetHead()->value;
			entry.code = MathInstruction::TranslateUnaryRightOperatorToken(*operationNode->name);
		}

		if (entry.code == MathInstruction::MathOp::UNKNOWN)
			throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"unary-expression\" in AST.", operationNode->name->c_str()), &operationNode->fileLocation);

		instructionGenerator->GenerateInstructionListRecursively(instructionList, operandNode);

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}