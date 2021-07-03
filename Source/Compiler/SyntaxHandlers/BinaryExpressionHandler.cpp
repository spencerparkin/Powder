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

	/*virtual*/ void BinaryExpressionHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"binary-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		// We treat assignment to identifiers as a special case here, because it is not supported by the math instruction.
		// Rather, it is supported by the store instruction.  The math instruction only operates on concrete values.
		// Yes, we could introduce variable values as another type of value floating around in the VM, and have the
		// math instruction store variables when given an assignment operation to perform, or load concrete values
		// when it encounters a variable value, but I think that starts to violate a clear separation of concerns,
		// and furthermore, it makes one instruction (in this case, the math instruction), more complicated than it needs to be.
		const Parser::SyntaxNode* operationNode = syntaxNode->childList.GetHead()->GetNext()->value;
		if (*operationNode->name == "=")
		{
			const Parser::SyntaxNode* storeLocationNode = syntaxNode->childList.GetHead()->value;
			if (*storeLocationNode->name != "identifier" && *storeLocationNode->name != "container-field-expression")
				throw new CompileTimeException(FormatString("Expected left operand of \"binary-expression\" in AST to be an storable location (not \"%s\") when the operation is assignment.", storeLocationNode->name->c_str()), &storeLocationNode->fileLocation);

			if (*storeLocationNode->name == "identifier")
			{
				const Parser::SyntaxNode* storeLocationNameNode = storeLocationNode->childList.GetHead()->value;

				// Lay down the instructions that will generate the value to be stored on top of the evaluation stack.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

				// Now issue a store instruction.  Note that this also pops the value off the stack, which is
				// symmetrically consistent with its counter-part, the load instruction.
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.string = *storeLocationNameNode->name;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);

				// TODO: For the case a = b = 1, look at our parent syntax nodes to see if we should issue
				//       a load instruction here for the next store instruction.
			}
			else if (*storeLocationNode->name == "container-field-expression")
			{
				// First goes the container value.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->childList.GetHead()->value);

				// Second goes the field value.  (Not to be confused with the value that will be stored at the field value in the container value.)
				instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->childList.GetHead()->GetNext()->value);

				// Third goes the value to be store in the container value at the field value.
				instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

				// Finally, issue a math instruction to insert a value into the container value at the field value.
				MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = MathInstruction::MathOp::SET_FIELD;
				mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
				instructionList.AddTail(mathInstruction);

				// Lastly, check out context.  If nothing wants the value we leave on the stack-stop, pop it.
				if (syntaxNode->parentNode && *syntaxNode->parentNode->name == "statement-list")
				{
					PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
					instructionList.AddTail(popInstruction);
				}
			}
		}
		else
		{
			// We first lay down the instruction that generate the left operand on top of the evaluation stack.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);

			// Then we lay down the instructions that will generate the right operand on top of the evaluation stack.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

			// At this point, we should have our left and right operands as the two top values of the evaluation stack.
			// So here we simply issue the appropriate math instruction to pop both those off, combine them, and then push the result.
			AssemblyData::Entry entry;
			entry.code = MathInstruction::TranslateBinaryOperatorInfixToken(*operationNode->name);
			if (entry.code == MathInstruction::MathOp::UNKNOWN)
				throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"binary-expression\" in AST.", operationNode->name->c_str()), &operationNode->fileLocation);

			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);
		}
	}
}