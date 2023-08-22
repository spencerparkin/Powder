#include "BinaryExpressionHandler.h"
#include "StoreInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"

namespace Powder
{
	BinaryExpressionHandler::BinaryExpressionHandler()
	{
	}

	/*virtual*/ BinaryExpressionHandler::~BinaryExpressionHandler()
	{
	}

	/*virtual*/ void BinaryExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"binary-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		// We first lay down the instruction(s) that generate the left operand on top of the evaluation stack.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0));

		// Then we lay down the instruction(s) that will generate the right operand on top of the evaluation stack.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2));

		// At this point, we should have our left and right operands as the two top values of the evaluation stack, in proper order.
		// So here we simply issue the appropriate math instruction to pop both those off, combine them in the operation at hand, and then push the result.
		AssemblyData::Entry entry;
		const ParseParty::Parser::SyntaxNode* operationNode = syntaxNode->GetChild(1);
		entry.code = MathInstruction::TranslateBinaryOperatorInfixToken(*operationNode->text);
		if (entry.code == MathInstruction::MathOp::UNKNOWN)
			throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"binary-expression\" in AST.", operationNode->text->c_str()), &operationNode->fileLocation);

		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);
	}
}