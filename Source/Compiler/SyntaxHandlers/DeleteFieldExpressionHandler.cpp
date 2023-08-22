#include "DeleteFieldExpressionHandler.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"

namespace Powder
{
	DeleteFieldExpressionHandler::DeleteFieldExpressionHandler()
	{
	}

	/*virtual*/ DeleteFieldExpressionHandler::~DeleteFieldExpressionHandler()
	{
	}

	/*virtual*/ void DeleteFieldExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->GetChildCount() != 2)
			throw new CompileTimeException("Expected \"delete-field-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

		if (*syntaxNode->GetChild(1)->text != "container-field-expression")
			throw new CompileTimeException("Expected \"container-field-expression\" to be second child of \"delete-field-expression\" in AST.", &syntaxNode->GetChild(1)->fileLocation);

		const ParseParty::Parser::SyntaxNode* containerFieldNode = syntaxNode->GetChild(1);

		// Push the container value.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, containerFieldNode->GetChild(0));

		// Push the field value to delete.
		instructionGenerator->GenerateInstructionListRecursively(instructionList, containerFieldNode->GetChild(1));

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
	}
}