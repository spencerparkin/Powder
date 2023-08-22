#include "AssignmentExpressionHandler.h"
#include "StoreInstruction.h"
#include "MathInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"

namespace Powder
{
	AssignmentExpressionHandler::AssignmentExpressionHandler()
	{
	}

	/*virtual*/ AssignmentExpressionHandler::~AssignmentExpressionHandler()
	{
	}

	/*virtual*/ void AssignmentExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 3)
			throw new CompileTimeException("Expected \"assignment-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

		// We treat assignment to identifiers (or other things) different than a generic binary expression, because it is not
		// supported by the math instruction.  Rather, it is supported by the store instruction.  The math instruction only
		// operates on concrete values.  Yes, we could introduce symbolic variable values as another type of value floating
		// around in the VM, and have the math instruction store variables when given an assignment operation to perform, or
		// load concrete values when it encounters a symbolic variable value, but I think that starts to violate a clear separation
		// of concerns, and furthermore, it makes one instruction (in this case, the math instruction), more complicated than it needs to be.
		const ParseParty::Parser::SyntaxNode* operationNode = syntaxNode->childList.GetHead()->GetNext()->value;
		if (*operationNode->name != "=")
			throw new CompileTimeException("Expected \"assignment-expression\" to have quality child node in AST.", &operationNode->fileLocation);
		
		const ParseParty::Parser::SyntaxNode* storeLocationNode = syntaxNode->childList.GetHead()->value;
		if (*storeLocationNode->name != "identifier" && *storeLocationNode->name != "container-field-expression")
			throw new CompileTimeException(FormatString("Expected left operand of \"assignment-expression\" in AST to be a storable location (not \"%s\".)", storeLocationNode->name->c_str()), &storeLocationNode->fileLocation);

		if (*storeLocationNode->name == "identifier")
		{
			const ParseParty::Parser::SyntaxNode* storeLocationNameNode = storeLocationNode->childList.GetHead()->value;

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
			if (this->PopNeededForExpression(syntaxNode))
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
				instructionList.AddTail(popInstruction);
			}
		}
	}
}