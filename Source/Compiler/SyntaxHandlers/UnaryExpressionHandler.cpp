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

		if (*syntaxNode->text == "left-unary-expression")
		{
			operationNode = syntaxNode->GetChild(0);
			operandNode = syntaxNode->GetChild(1);
			entry.code = MathInstruction::TranslateUnaryLeftOperatorToken(*operationNode->text);
		}
		else if (*syntaxNode->text == "right-unary-expression")
		{
			operationNode = syntaxNode->GetChild(1);
			operandNode = syntaxNode->GetChild(0);
			entry.code = MathInstruction::TranslateUnaryRightOperatorToken(*operationNode->text);
		}

		if (entry.code == MathInstruction::MathOp::UNKNOWN)
			throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"unary-expression\" in AST.", operationNode->text->c_str()), &operationNode->fileLocation);

		instructionGenerator->GenerateInstructionListRecursively(instructionList, operandNode);

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}