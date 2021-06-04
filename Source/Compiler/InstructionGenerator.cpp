#include "InstructionGenerator.h"
#include "Assembler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "LoadInstruction.h"
#include "StoreInstruction.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "MathInstruction.h"
#include "SysCallInstruction.h"
#include "Exceptions.hpp"

namespace Powder
{
	InstructionGenerator::InstructionGenerator()
	{
	}

	/*virtual*/ InstructionGenerator::~InstructionGenerator()
	{
	}

	void InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode)
	{
		if (*syntaxNode->name == "statement-list")
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
			{
				if (*node->value->name != "statement")
					throw new CompileTimeException("Expected \"statement\" under \"statement-list\" in AST", &node->value->fileLocation);

				// We simply execute the code for each statement in order.
				this->GenerateInstructionList(instructionList, node->value);
			}
		}
		else if (*syntaxNode->name == "statement")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"statement\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "if-statement")
		{
			if (syntaxNode->childList.GetCount() != 3 && syntaxNode->childList.GetCount() != 5)
				throw new CompileTimeException("Expected \"if-statement\" in AST to have exactly 3 or 5 children.", &syntaxNode->fileLocation);

			AssemblyData::Entry entry;

			// Execute conditional instructions.  What remains on the evaluation stack top gets consumed by the branch instruction.
			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			
			// The branch instruction falls through if the condition passes, and jumps if the condition fails.
			BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>();
			instructionList.AddTail(branchInstruction);
			
			// Lay down condition-pass instructions.
			LinkedList<Instruction*> passInstructionList;
			this->GenerateInstructionList(passInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);
			instructionList.Append(passInstructionList);

			// Else clause?
			if (syntaxNode->childList.GetCount() != 5)
			{
				// No.  Setup jump-hint on the branch instruction to jump to instruction just after the last condition-pass instruction.
				entry.jumpDelta = passInstructionList.GetCount();
				entry.string = "branch";
				branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
			}
			else
			{
				// Yes.  Before laying down the condition-fail instructions, we want an unconditional jump that goes over them if the condition passed.
				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				instructionList.AddTail(jumpInstruction);

				// Okay, now lay down the condition-fail instructions.
				LinkedList<Instruction*> failInstructionList;
				this->GenerateInstructionList(failInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
				instructionList.Append(failInstructionList);
				entry.jumpDelta = failInstructionList.GetCount();
				entry.string = "jump";
				jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);

				// We have enough now to resolve the conditional jump instruction.
				entry.instruction = failInstructionList.GetHead()->value;
				branchInstruction->assemblyData->configMap.Insert("branch", entry);
			}
		}
		else if (*syntaxNode->name == "expression")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"expression\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "binary-expression")
		{
			if(syntaxNode->childList.GetCount() != 3)
				throw new CompileTimeException("Expected \"binary-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

			// We treat assignment as a special case here, because it is not supported by the math instruction.
			// Rather, it is supported by the store instruction.  The math instruction only operates on concrete values.
			// Yes, we could introduce variable values as another type of value floating around in the VM, and have the
			// math instruction store variables when given an assignment operation to perform, or load concrete values
			// when it encounters a variable value, but I think that starts to violate a clear separation of concerns,
			// and furthermore, it makes one instruction (in this case, the math instruction), more complicated than it needs to be.
			const Parser::SyntaxNode* operationNode = syntaxNode->childList.GetHead()->GetNext()->value;
			if (*operationNode->name == "=")
			{
				const Parser::SyntaxNode* storeLocationNode = operationNode->childList.GetHead()->value;
				if (*storeLocationNode->name != "identifier")
					throw new CompileTimeException(FormatString("Expected left operand of \"binary-expression\" in AST to be an identifier (not \"%s\") when the operation is assignment.", storeLocationNode->name->c_str()), &storeLocationNode->fileLocation);

				// Lay down the instructions that will generate the value to be stored on top of the evaluation stack.
				this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

				// Now issue a store instruction.  Note that this also pops the value off the stack, which is
				// symmetrically consistent with its counter-part, the load instruction.
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
				AssemblyData::Entry entry;
				entry.string = *storeLocationNode->name;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);

				// TODO: We might try to handle the case: a = b = 1 here.  Maybe add a config byte to the store
				//       instruction to tell it to not pop the value?  That would probably be the right way.
				//       Here we would look up the AST parent line to see if we find another assignment, and
				//       if we do, forgo the pop in the store.
			}
			else
			{
				// We first lay down the instruction that generate the left operand on top of the evaluation stack.
				this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);

				// Then we lay down the instructions that will generate the right operand on top of the evaluation stack.
				this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

				// At this point, we should have our left and right operands as the two top values of the evaluation stack.
				// So here we simply issue the appropriate math instruction to pop both those off, combine them, and then push the result.
				AssemblyData::Entry entry;
				if (*operationNode->name == "+")
					entry.code = MathInstruction::MathOp::ADD;
				else if (*operationNode->name == "-")
					entry.code = MathInstruction::MathOp::SUBTRACT;
				else if (*operationNode->name == "*")
					entry.code = MathInstruction::MathOp::MULTIPLY;
				else if (*operationNode->name == "/")
					entry.code = MathInstruction::MathOp::DIVIDE;
				else
					throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"binary-expression\" in AST.", operationNode->name->c_str()), &operationNode->fileLocation);

				MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
				mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
				instructionList.AddTail(mathInstruction);
			}
		}
		else if (*syntaxNode->name == "unary-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"unary-expression\" in AST to have exactly 1 child.", &syntaxNode->fileLocation);

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "left-unary-expression" || *syntaxNode->name == "right-unary-expression")
		{
			const Parser::SyntaxNode* operationNode = nullptr;
			const Parser::SyntaxNode* operandNode = nullptr;

			if (*syntaxNode->name == "left-unary-expression")
			{
				operationNode = syntaxNode->childList.GetHead()->value;
				operandNode = syntaxNode->childList.GetHead()->GetNext()->value;
			}
			else if (*syntaxNode->name == "right-unary-expression")
			{
				operationNode = syntaxNode->childList.GetHead()->GetNext()->value;
				operandNode = syntaxNode->childList.GetHead()->value;
			}

			AssemblyData::Entry entry;
			if (*operationNode->name == "-")
				entry.code = MathInstruction::MathOp::NEGATE;
			else if (*operationNode->name == "!" && *syntaxNode->name == "left-unary-expression")
				entry.code = MathInstruction::MathOp::NOT;
			else if (*operationNode->name == "!" && *syntaxNode->name == "right-unary-expression")
				entry.code = MathInstruction::MathOp::FACTORIAL;
			else
				throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"unary-expression\" in AST.", operationNode->name->c_str()), &operationNode->fileLocation);

			this->GenerateInstructionList(instructionList, operandNode);

			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);

		}
		else if (*syntaxNode->name == "literal")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"literal\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			if (syntaxNode->childList.GetHead()->value->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"literal\" in AST to have exactly one grandchild.", &syntaxNode->fileLocation);

			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
			instructionList.AddTail(pushInstruction);

			const Parser::SyntaxNode* literalTypeNode = syntaxNode->childList.GetHead()->value;
			const Parser::SyntaxNode* literalDataNode = literalTypeNode->childList.GetHead()->value;

			AssemblyData::Entry typeEntry;
			AssemblyData::Entry dataEntry;

			if (*literalTypeNode->name == "string-literal")
			{
				typeEntry.code = PushInstruction::DataType::STRING;
				dataEntry.string = *literalDataNode->name;
			}
			else if (*literalTypeNode->name == "number-literal")
			{
				typeEntry.code = PushInstruction::DataType::NUMBER;
				dataEntry.number = ::strtol(literalDataNode->name->c_str(), nullptr, 10);
			}
			else if (*literalTypeNode->name == "list-literal")
			{
				typeEntry.code = PushInstruction::DataType::EMPTY_LIST;
			}
			else
			{
				throw new CompileTimeException(FormatString("Did not recognize \"%s\" data-type under \"literal\" in AST.", literalTypeNode->name->c_str()), &literalTypeNode->fileLocation);
			}

			pushInstruction->assemblyData->configMap.Insert("type", typeEntry);
			pushInstruction->assemblyData->configMap.Insert("data", dataEntry);

			if (*literalTypeNode->name == "list-literal")
			{
				// In this case, next come the instructions that populate the list.
				this->GenerateInstructionList(instructionList, literalTypeNode);
			}
		}
		else if (*syntaxNode->name == "identifier")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"identifier\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			// Note that we assume we're generating code here in the context of an expression, but we're not
			// going to check that, because there's no obvious way I can think of at the moment without
			// introducing more fluff back into the AST.  Much of the fluff was removed before we were handed the AST.
			// Of course, identifiers can appear in other contexts, such as a function call for function definition.
			// Note that we also generate this code in the context of a system call, which is looking for values on the eval stack.
			const Parser::SyntaxNode* identifierNode = syntaxNode->childList.GetHead()->value;
			LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>();
			AssemblyData::Entry entry;
			entry.string = *identifierNode->name;
			loadInstruction->assemblyData->configMap.Insert("name", entry);
			instructionList.AddTail(loadInstruction);
		}
		else if (*syntaxNode->name == "list-literal")
		{
			// TODO: Here we need to generate the instructions that populate the list literal.
		}
		else if (*syntaxNode->name == "function-call")
		{
			const Parser::SyntaxNode* identifierNode = syntaxNode->FindChild("identifier", 1);
			if (!identifierNode)
				throw new CompileTimeException("Expected \"function-call\" in AST to have a child with name \"identifier\".", &syntaxNode->fileLocation);

			const Parser::SyntaxNode* argListNode = syntaxNode->FindChild("argument-list", 1);

			std::string funcName = *identifierNode->name;
			SysCallInstruction::SysCall sysCall = SysCallInstruction::TranslateAsSysCall(funcName);
			if (sysCall != SysCallInstruction::SysCall::UNKNOWN)
			{
				if (argListNode)
				{
					for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
					{
						const Parser::SyntaxNode* argNode = node->value;
						if (*argNode->name != "argument")
							throw new CompileTimeException("Expected \"argument-list\" in AST to have only \"argument\" children.", &argNode->fileLocation);

						if (!argNode->childList.GetCount() == 1)
							throw new CompileTimeException("Expected \"argument\" in AST to have exactly one child.", &argNode->fileLocation);

						// We need to check that the code we're about to run will just push a value onto the eval stack, which is where sys-calls take their arguments from.
						if (*argNode->childList.GetHead()->value->name != "expression" && *argNode->childList.GetHead()->value->name != "function-call")
							throw new CompileTimeException("Expected \"argument\" of system call to be an \"expression\" or \"function-call\".", &argNode->fileLocation);

						this->GenerateInstructionList(instructionList, argNode->childList.GetHead()->value);
					}
				}

				SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>();
				AssemblyData::Entry entry;
				entry.code = sysCall;
				sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
				instructionList.AddTail(sysCallInstruction);
			}
			else
			{
				// TODO: This is where we call a user-defined function.
			}
		}
	}
}