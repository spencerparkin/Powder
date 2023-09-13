#include "BinaryExpressionHandler.h"
#include "StoreInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	BinaryExpressionHandler::BinaryExpressionHandler()
	{
	}

	/*virtual*/ BinaryExpressionHandler::~BinaryExpressionHandler()
	{
	}

	/*virtual*/ bool BinaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 3)
		{
			error.Add(std::string(syntaxNode->fileLocation) + std::format("Expected \"binary-expression\" in AST to have exactly 3 children."));
			return false;
		}

		// We first lay down the instruction(s) that generate the left operand on top of the evaluation stack.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0), error))
		{
			error.Add(std::string(syntaxNode->GetChild(0)->fileLocation) + "Failed to generate instructions for left operand.");
			return false;
		}

		// Then we lay down the instruction(s) that will generate the right operand on top of the evaluation stack.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2), error))
		{
			error.Add(std::string(syntaxNode->GetChild(2)->fileLocation) + "Failed to generate instructions for right operand.");
			return false;
		}

		// At this point, we should have our left and right operands as the two top values of the evaluation stack, in proper order.
		// So here we simply issue the appropriate math instruction to pop both those off, combine them in the operation at hand, and then push the result.
		AssemblyData::Entry entry;
		const ParseParty::Parser::SyntaxNode* operationNode = syntaxNode->GetChild(1);
		entry.code = MathInstruction::TranslateBinaryOperatorInfixToken(*operationNode->text);
		if (entry.code == MathInstruction::MathOp::UNKNOWN)
		{
			error.Add(std::string(operationNode->fileLocation) + std::format("Failed to recognize math operation \"{}\" for \"binary-expression\" in AST.", operationNode->text->c_str()));
			return false;
		}

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
		return true;
	}
}