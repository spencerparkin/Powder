#include "UnaryExpressionHandler.h"
#include "MathInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	UnaryExpressionHandler::UnaryExpressionHandler()
	{
	}

	/*virtual*/ UnaryExpressionHandler::~UnaryExpressionHandler()
	{
	}

	/*virtual*/ bool UnaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
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
		{
			error.Add(std::string(operationNode->fileLocation) + std::format("Failed to recognize math operation \"{}\" for \"unary-expression\" in AST.", operationNode->text->c_str()));
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, operandNode, error))
		{
			error.Add(std::string(operandNode->fileLocation) + "Failed to generate instructions for unary operand.");
			return false;
		}

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		return true;
	}
}