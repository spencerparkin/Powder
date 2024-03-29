#include "AssignmentExpressionHandler.h"
#include "StoreInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	AssignmentExpressionHandler::AssignmentExpressionHandler()
	{
	}

	/*virtual*/ AssignmentExpressionHandler::~AssignmentExpressionHandler()
	{
	}

	/*virtual*/ bool AssignmentExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 3)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"assignment-expression\" in AST to have exactly 3 children.");
			return false;
		}

		// We treat assignment to identifiers (or other things) different than a generic binary expression, because it is not
		// supported by the math instruction.  Rather, it is supported by the store instruction.  The math instruction only
		// operates on concrete values.  Yes, we could introduce symbolic variable values as another type of value floating
		// around in the VM, and have the math instruction store variables when given an assignment operation to perform, or
		// load concrete values when it encounters a symbolic variable value, but I think that starts to violate a clear separation
		// of concerns, and furthermore, it makes one instruction (in this case, the math instruction), more complicated than it needs to be.
		const ParseParty::Parser::SyntaxNode* operationNode = syntaxNode->GetChild(1);
		if (*operationNode->text != "=")
		{
			error.Add(std::string(operationNode->fileLocation) + "Expected \"assignment-expression\" to have equality child node in AST.");
			return false;
		}
		
		const ParseParty::Parser::SyntaxNode* storeLocationNode = syntaxNode->GetChild(0);
		if (*storeLocationNode->text != "@identifier" && *storeLocationNode->text != "container-field-expression")
		{
			error.Add(std::string(storeLocationNode->fileLocation) + std::format("Expected left operand of \"assignment-expression\" in AST to be a storable location (not \"{}\".)", storeLocationNode->text->c_str()));
			return false;
		}

		if (*storeLocationNode->text == "@identifier")
		{
			const ParseParty::Parser::SyntaxNode* storeLocationNameNode = storeLocationNode->GetChild(0);

			// Lay down the instructions that will generate the value to be stored on top of the evaluation stack.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2), error))
			{
				error.Add(std::string(syntaxNode->fileLocation) + "Failed to generate instructions for the value to be stored on the top of the eval stack.");
				return false;
			}

			// Now issue a store instruction.  Note that this also pops the value off the stack, which is
			// symmetrically consistent with its counter-part, the load instruction.
			StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.string = *storeLocationNameNode->text;
			storeInstruction->assemblyData->configMap.Insert("name", entry);
			instructionList.AddTail(storeInstruction);

			// TODO: For the case a = b = 1, look at our parent syntax nodes to see if we should issue
			//       a load instruction here for the next store instruction.
		}
		else if (*storeLocationNode->text == "container-field-expression")
		{
			// First goes the container value.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->GetChild(0), error))
			{
				error.Add(std::string(storeLocationNode->fileLocation) + "Failed to generate instructions for container value.");
				return false;
			}

			// Second goes the field value.  (Not to be confused with the value that will be stored at the field value in the container value.)
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->GetChild(1), error))
			{
				error.Add(std::string(storeLocationNode->fileLocation) + "Failed to generate instructions for the field value.");
				return false;
			}

			// Third goes the value to be store in the container value at the field value.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(2), error))
			{
				error.Add(std::string(syntaxNode->fileLocation) + "Failed to generate instructions for value to be sotred in the container.");
				return false;
			}

			// Finally, issue a math instruction to insert a value into the container value at the field value.
			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = MathInstruction::MathOp::SET_FIELD;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);

			// Lastly, check out context.  If nothing wants the value we leave on the stack-stop, pop it.
			if (this->PopNeededForExpression(syntaxNode))
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
				instructionList.AddTail(popInstruction);
			}
		}

		return true;
	}
}