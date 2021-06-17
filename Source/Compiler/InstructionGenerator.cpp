#include "InstructionGenerator.h"
#include "Assembler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "ListInstruction.h"
#include "LoadInstruction.h"
#include "StoreInstruction.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "MapInstruction.h"
#include "MathInstruction.h"
#include "SysCallInstruction.h"
#include "ScopeInstruction.h"
#include "ForkInstruction.h"
#include "YieldInstruction.h"
#include "Exceptions.hpp"
#include "HashMap.hpp"

namespace Powder
{
	InstructionGenerator::InstructionGenerator()
	{
	}

	/*virtual*/ InstructionGenerator::~InstructionGenerator()
	{
	}

	void InstructionGenerator::GenerateFunctionArgumentInstructions(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* argListNode)
	{
		if (argListNode)
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
			{
				const Parser::SyntaxNode* argNode = node->value;
				if (*argNode->name != "identifier")
					throw new CompileTimeException("Expected all children of \"identifier-list\" in AST to be \"identifier\".", &argNode->fileLocation);

				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>();
				AssemblyData::Entry entry;
				entry.code = ListInstruction::Action::POP_LEFT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(listInstruction);

				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
				entry.Reset();
				entry.string = *argNode->childList.GetHead()->value->name;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);
			}
		}

		// Don't leak the argument list.
		PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
		instructionList.AddTail(popInstruction);
	}

	void InstructionGenerator::GenerateFunctionDeclarationInstructions(LinkedList<Instruction*>& instructionList, const std::string& funcName, Instruction* firstFunctionInstruction)
	{
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
		AssemblyData::Entry entry;
		entry.code = PushInstruction::DataType::ADDRESS;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.Reset();
		entry.instruction = firstFunctionInstruction;
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);

		StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
		entry.Reset();
		entry.string = funcName;
		storeInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(storeInstruction);
	}

	void InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* rootSyntaxNode)
	{
		this->GenerateInstructionListRecursively(instructionList, rootSyntaxNode);

		// Issue a halt instruction here so that we don't crash into the first subroutine of the program, if any.
		// Of course, this isn't a problem if there aren't any subroutines.  However, we also want this instruction
		// here in the case that we need to resolve a jump that goes beyond the last program construct.
		SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>();
		AssemblyData::Entry entry;
		entry.code = SysCallInstruction::SysCall::EXIT;
		sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
		instructionList.AddTail(sysCallInstruction);

		HashMap<int> functionNameMap;

		if (this->functionDefinitionList.GetCount() > 0)
		{
			for (LinkedList<const Parser::SyntaxNode*>::Node* node = this->functionDefinitionList.GetHead(); node; node = node->GetNext())
			{
				const Parser::SyntaxNode* functionDefNode = node->value;
				const Parser::SyntaxNode* functionStatementListNode = functionDefNode->FindChild("statement-list", 2);
				if (!functionStatementListNode)
					throw new CompileTimeException("Expected \"function-definition\" to have \"statement-list\" in AST.", &functionDefNode->fileLocation);

				const Parser::SyntaxNode* identifierNode = functionDefNode->FindChild("identifier", 1);
				if (!identifierNode)
					throw new CompileTimeException("Expected \"function-definition\" in AST to have \"identifier\" child.", &functionDefNode->fileLocation);

				const Parser::SyntaxNode* argListNode = functionDefNode->FindChild("identifier-list", 1);

				std::string funcName = *identifierNode->childList.GetHead()->value->name;
				if (functionNameMap.LookupPtr(funcName.c_str()) != nullptr)
					throw new CompileTimeException(FormatString("Function \"%s\" defined more than once.", funcName.c_str()), &functionDefNode->fileLocation);
				else
					functionNameMap.Insert(funcName.c_str(), 0);

				// The instructions of a function consist of the code for off-loading the arguments, then the function body.
				LinkedList<Instruction*> functionInstructionList;
				this->GenerateFunctionArgumentInstructions(functionInstructionList, argListNode);
				this->GenerateInstructionListRecursively(functionInstructionList, functionStatementListNode);

				// The end of the function might already have a return, but if it doesn't, this doesn't hurt.
				// We ensure that all functions return a value no matter what, because the calling code will always
				// expect it, even if it is going to discard it.
				PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				entry.Reset();
				entry.code = PushInstruction::DataType::UNDEFINED;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				functionInstructionList.AddTail(pushInstruction);
				this->GenerateFunctionReturnInstructions(functionInstructionList);

				// Lay down the instructions for the function at the end of the executable.
				// This way, we don't have to jump over the function bodies, which would be dumb.
				instructionList.Append(functionInstructionList);

				// Lastly, declare the function at the top of the executable so that it can be
				// called from anywhere, even before the definition shows up in the source file.
				LinkedList<Instruction*> functionDeclarationInstructionList;
				this->GenerateFunctionDeclarationInstructions(functionDeclarationInstructionList, funcName, functionInstructionList.GetHead()->value);
				instructionList.Prepend(functionDeclarationInstructionList);
			}
		}
	}

	// Normally I would cringe at a subroutine being this hideously long,
	// because it can causes you to not be able to see the forest through
	// the trees (i.e., you lose sight of context and the structure of the
	// code), but I don't believe this is the case here, and that this is
	// one of the rare exceptions to the rule that a subroutine should be
	// about no more than a page-full.
	void InstructionGenerator::GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode)
	{
		if (*syntaxNode->name == "statement-list")
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
			{
				// We simply execute the code for each statement in order.
				this->GenerateInstructionListRecursively(instructionList, node->value);
			}
		}
		else if (*syntaxNode->name == "container-size-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"container-size-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

			// Load the container value onto the eval stack top.  Note that we won't know until run-time if we're actually getting a container value.
			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);

			// Now compute its size while also popping it off.
			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			AssemblyData::Entry entry;
			entry.code = MathInstruction::MathOp::SIZE;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);
		}
		else if (*syntaxNode->name == "container-field-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"container-field-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

			// Load the container value onto the eval stack top first.
			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);

			// Load the field value onto the eval stack top second.
			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);

			// Now compute the container value look-up operation.
			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			AssemblyData::Entry entry;
			entry.code = MathInstruction::MathOp::GET_FIELD;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);
		}
		else if (*syntaxNode->name == "delete-field-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"delete-field-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

			if (*syntaxNode->childList.GetHead()->GetNext()->value->name != "container-field-expression")
				throw new CompileTimeException("Expected \"container-field-expression\" to be second child of \"delete-field-expression\" in AST.", &syntaxNode->childList.GetHead()->GetNext()->value->fileLocation);

			const Parser::SyntaxNode* containerFieldNode = syntaxNode->childList.GetHead()->GetNext()->value;

			// Push the container value.
			this->GenerateInstructionListRecursively(instructionList, containerFieldNode->childList.GetHead()->value);

			// Push the field value to delete.
			this->GenerateInstructionListRecursively(instructionList, containerFieldNode->childList.GetHead()->GetNext()->value);

			// And now issue the del instruction.
			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			AssemblyData::Entry entry;
			entry.code = MathInstruction::MathOp::DEL_FIELD;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);

			// Our result is the value in the container at the deleted field value.  But if no one wants it, pop it.
			if (syntaxNode->parentNode && *syntaxNode->parentNode->name == "statement-list")
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
				instructionList.AddTail(popInstruction);
			}
		}
		else if (*syntaxNode->name == "if-statement")
		{
			if (syntaxNode->childList.GetCount() != 3 && syntaxNode->childList.GetCount() != 5)
				throw new CompileTimeException("Expected \"if-statement\" in AST to have exactly 3 or 5 children.", &syntaxNode->fileLocation);

			AssemblyData::Entry entry;

			// Execute conditional instructions.  What remains on the evaluation stack top gets consumed by the branch instruction.
			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			
			// The branch instruction falls through if the condition passes, and jumps if the condition fails.
			BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>();
			instructionList.AddTail(branchInstruction);
			
			// Lay down condition-pass instructions.
			LinkedList<Instruction*> passInstructionList;
			this->GenerateInstructionListRecursively(passInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);
			instructionList.Append(passInstructionList);

			// Else clause?
			if (syntaxNode->childList.GetCount() != 5)
			{
				// No.  Setup jump-hint on the branch instruction to jump to instruction just after the last condition-pass instruction.
				entry.Reset();
				entry.jumpDelta = passInstructionList.GetCount() + 1;
				entry.string = "branch";
				branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
			}
			else
			{
				// Yes.  Before laying down the condition-fail instructions, we want an unconditional jump that goes over them if the condition passed.
				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				entry.Reset();
				entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
				jumpInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(jumpInstruction);

				// Okay, now lay down the condition-fail instructions.
				LinkedList<Instruction*> failInstructionList;
				this->GenerateInstructionListRecursively(failInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
				instructionList.Append(failInstructionList);

				// We have enough now to resolve the jump-delta for getting over the else-clause.
				entry.Reset();
				entry.jumpDelta = failInstructionList.GetCount() + 1;
				entry.string = "jump";
				jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);

				// We have enough now to resolve the conditional jump instruction.
				entry.Reset();
				entry.instruction = failInstructionList.GetHead()->value;
				branchInstruction->assemblyData->configMap.Insert("branch", entry);
			}
		}
		else if (*syntaxNode->name == "while-statement")
		{
			if (syntaxNode->childList.GetCount() != 3)
				throw new CompileTimeException("Expected \"while-statement\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

			AssemblyData::Entry entry;

			// Lay down the conditional instructions first.  What remains is a value on the eval stack who's truthiness we'll use in a branch instruction.
			LinkedList<Instruction*> conditionalInstructionList;
			this->GenerateInstructionListRecursively(conditionalInstructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			instructionList.Append(conditionalInstructionList);

			// The branch falls through if the bool is true, or jumps in the bool is false.  We don't yet know how far to jump to get over the while-loop body.
			BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>();
			instructionList.AddTail(branchInstruction);

			// Lay down while-loop body instructions.
			LinkedList<Instruction*> whileLoopBodyInstructionList;
			this->GenerateInstructionListRecursively(whileLoopBodyInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);
			instructionList.Append(whileLoopBodyInstructionList);

			// Unconditionally jump back to the top of the while-loop where the conditional is evaluated.
			JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
			entry.Reset();
			entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
			jumpInstruction->assemblyData->configMap.Insert("type", entry);
			entry.instruction = conditionalInstructionList.GetHead()->value;
			jumpInstruction->assemblyData->configMap.Insert("jump", entry);
			instructionList.AddTail(jumpInstruction);

			// We now know enough to resolve the branch jump delta.  It's the size of the body plus the unconditional jump.
			entry.Reset();
			entry.jumpDelta = whileLoopBodyInstructionList.GetCount() + 2;
			entry.string = "branch";
			branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
		else if (*syntaxNode->name == "do-while-statement")
		{
			if(syntaxNode->childList.GetCount() != 4 && syntaxNode->childList.GetCount() != 5)
				throw new CompileTimeException("Expected \"do-while-statement\" in AST to have exactly 4 or 5 children.", &syntaxNode->fileLocation);

			AssemblyData::Entry entry;

			// Lay down the first half of the loop instructions.
			LinkedList<Instruction*> initialLoopInstructionList;
			this->GenerateInstructionListRecursively(initialLoopInstructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			instructionList.Append(initialLoopInstructionList);

			// Now lay down the conditional instructions of the loop.  What should remain is a single value on the eval stack for our branch instruction.
			LinkedList<Instruction*> conditionalInstructionList;
			this->GenerateInstructionListRecursively(conditionalInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->value);
			instructionList.Append(conditionalInstructionList);

			// Condition failure means we jump; success, we fall through.
			BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>();
			instructionList.AddTail(branchInstruction);

			// Now lay down the last half of the loop instructions, if given.
			LinkedList<Instruction*> finalLoopInstructionList;
			if (syntaxNode->childList.GetCount() == 5)
			{
				this->GenerateInstructionListRecursively(finalLoopInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
				instructionList.Append(finalLoopInstructionList);
			}

			// Unconditionally jump back up to the top of the do-while-loop.
			JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
			entry.Reset();
			entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
			jumpInstruction->assemblyData->configMap.Insert("type", entry);
			entry.instruction = initialLoopInstructionList.GetHead()->value;
			jumpInstruction->assemblyData->configMap.Insert("jump", entry);
			instructionList.AddTail(jumpInstruction);

			// We now know enough to resolve the branch jump delta.
			entry.Reset();
			entry.jumpDelta = finalLoopInstructionList.GetCount() + 2;
			entry.string = "branch";
			branchInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		}
		else if (*syntaxNode->name == "yield-statement")
		{
			YieldInstruction* yeildInstruction = Instruction::CreateForAssembly<YieldInstruction>();
			instructionList.AddTail(yeildInstruction);
		}
		else if (*syntaxNode->name == "fork-statement")
		{
			if (syntaxNode->childList.GetCount() != 2 && syntaxNode->childList.GetCount() != 4)
				throw new CompileTimeException("Expected \"fork-statement\" in AST to have exactly 2 or 4 children.", &syntaxNode->fileLocation);

			AssemblyData::Entry entry;

			// A fork is like an unconditional jump, but it both does and doesn't jump.
			ForkInstruction* forkInstruction = Instruction::CreateForAssembly<ForkInstruction>();
			instructionList.AddTail(forkInstruction);

			LinkedList<Instruction*> forkedInstructionList;
			this->GenerateInstructionListRecursively(forkedInstructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			instructionList.Append(forkedInstructionList);

			if (syntaxNode->childList.GetCount() == 2)
			{
				entry.Reset();
				entry.jumpDelta = forkedInstructionList.GetCount() + 1;
				entry.string = "fork";
				forkInstruction->assemblyData->configMap.Insert("jump-delta", entry);
			}
			else if (syntaxNode->childList.GetCount() == 4)
			{
				entry.Reset();
				entry.jumpDelta = forkedInstructionList.GetCount() + 2;
				entry.string = "fork";
				forkInstruction->assemblyData->configMap.Insert("jump-delta", entry);

				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				entry.Reset();
				entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
				jumpInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(jumpInstruction);

				LinkedList<Instruction*> elseInstructionList;
				this->GenerateInstructionListRecursively(elseInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->value);
				instructionList.Append(elseInstructionList);

				entry.Reset();
				entry.jumpDelta = elseInstructionList.GetCount() + 1;
				entry.string = "jump";
				jumpInstruction->assemblyData->configMap.Insert("jump-delta", entry);
			}
		}
		else if (*syntaxNode->name == "binary-expression")
		{
			if(syntaxNode->childList.GetCount() != 3)
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
					this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

					// Now issue a store instruction.  Note that this also pops the value off the stack, which is
					// symmetrically consistent with its counter-part, the load instruction.
					StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
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
					this->GenerateInstructionListRecursively(instructionList, storeLocationNode->childList.GetHead()->value);

					// Second goes the field value.  (Not to be confused with the value that will be stored at the field value in the container value.)
					this->GenerateInstructionListRecursively(instructionList, storeLocationNode->childList.GetHead()->GetNext()->value);

					// Third goes the value to be store in the container value at the field value.
					this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

					// Finally, issue a math instruction to insert a value into the container value at the field value.
					MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
					AssemblyData::Entry entry;
					entry.code = MathInstruction::MathOp::SET_FIELD;
					mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
					instructionList.AddTail(mathInstruction);

					// Lastly, check out context.  If nothing wants the value we leave on the stack-stop, pop it.
					if (syntaxNode->parentNode && *syntaxNode->parentNode->name == "statement-list")
					{
						PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
						instructionList.AddTail(popInstruction);
					}
				}
			}
			else
			{
				// We first lay down the instruction that generate the left operand on top of the evaluation stack.
				this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);

				// Then we lay down the instructions that will generate the right operand on top of the evaluation stack.
				this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

				// At this point, we should have our left and right operands as the two top values of the evaluation stack.
				// So here we simply issue the appropriate math instruction to pop both those off, combine them, and then push the result.
				AssemblyData::Entry entry;
				entry.code = MathInstruction::TranslateBinaryOperatorInfixToken(*operationNode->name);
				if (entry.code == MathInstruction::MathOp::UNKNOWN)
					throw new CompileTimeException(FormatString("Failed to recognize math operation \"%s\" for \"binary-expression\" in AST.", operationNode->name->c_str()), &operationNode->fileLocation);

				MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
				mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
				instructionList.AddTail(mathInstruction);
			}
		}
		else if (*syntaxNode->name == "left-unary-expression" || *syntaxNode->name == "right-unary-expression")
		{
			const Parser::SyntaxNode* operationNode = nullptr;
			const Parser::SyntaxNode* operandNode = nullptr;

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

			this->GenerateInstructionListRecursively(instructionList, operandNode);

			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);
		}
		else if (*syntaxNode->name == "literal")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"literal\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
			instructionList.AddTail(pushInstruction);

			const Parser::SyntaxNode* literalTypeNode = syntaxNode->childList.GetHead()->value;
			const Parser::SyntaxNode* literalDataNode = (literalTypeNode->childList.GetCount() == 1) ? literalTypeNode->childList.GetHead()->value : nullptr;

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
				dataEntry.number = ::strtod(literalDataNode->name->c_str(), nullptr);
			}
			else if (*literalTypeNode->name == "list-literal")
				typeEntry.code = PushInstruction::DataType::EMPTY_LIST;
			else if (*literalTypeNode->name == "map-literal")
				typeEntry.code = PushInstruction::DataType::EMPTY_MAP;
			else
				throw new CompileTimeException(FormatString("Did not recognize \"%s\" data-type under \"literal\" in AST.", literalTypeNode->name->c_str()), &literalTypeNode->fileLocation);

			pushInstruction->assemblyData->configMap.Insert("type", typeEntry);
			pushInstruction->assemblyData->configMap.Insert("data", dataEntry);

			if (*literalTypeNode->name == "list-literal" || *literalTypeNode->name == "map-literal")
			{
				// In this case, next come the instructions that populate the list or map.
				this->GenerateInstructionListRecursively(instructionList, literalTypeNode);
			}

			// If the literal is not in the context of an assignment or expression of some kind, then we
			// need to issue a pop instruction here in order to not leak a value on the eval stack.
			if (syntaxNode->parentNode && *syntaxNode->parentNode->name == "statement-list")
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
				instructionList.AddTail(popInstruction);
			}
		}
		else if (*syntaxNode->name == "identifier")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"identifier\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			// Note that we assume we're generating code here in the context of an expression, but we're not
			// going to check that, because there's no obvious way I can think of at the moment without
			// introducing more fluff back into the AST.  Much of the fluff was removed before we were handed the AST.
			// Of course, identifiers can appear in other contexts, such as a function call or function definition.
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
			// The list may be empty, in which case, we don't find the node.
			const Parser::SyntaxNode* elementListNode = syntaxNode->FindChild("list-element-list", 1);
			if (elementListNode)
			{
				// We assume here that the list in question is already on the eval-stack.
				for (const LinkedList<Parser::SyntaxNode*>::Node* node = elementListNode->childList.GetHead(); node; node = node->GetNext())
				{
					// Push the element onto the eval stack.
					this->GenerateInstructionListRecursively(instructionList, node->value);

					// Now add the element to the list.  The element gets removed from the stack, but the list should remain.
					ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>();
					AssemblyData::Entry entry;
					entry.code = ListInstruction::PUSH_RIGHT;
					listInstruction->assemblyData->configMap.Insert("action", entry);
					instructionList.AddTail(listInstruction);
				}
			}
		}
		else if (*syntaxNode->name == "map-literal")
		{
			// The map may be empty, in which case, we don't find the node.
			const Parser::SyntaxNode* mapPairListNode = syntaxNode->FindChild("map-pair-list", 1);
			if (mapPairListNode)
			{
				for (const LinkedList<Parser::SyntaxNode*>::Node* node = mapPairListNode->childList.GetHead(); node; node = node->GetNext())
				{
					const Parser::SyntaxNode* mapPairNode = node->value;
					if (*mapPairNode->name != "map-pair")
						throw new CompileTimeException("Expected all children of \"map-pair-list\" to be \"map-pair\" in AST.", &mapPairNode->fileLocation);

					if (mapPairNode->childList.GetCount() != 3)
						throw new CompileTimeException("Expected \"map-pair\" node in AST to have exactly 3 children.", &mapPairNode->fileLocation);

					// Push the field value.
					this->GenerateInstructionListRecursively(instructionList, mapPairNode->childList.GetHead()->value);

					// Push the data value.
					this->GenerateInstructionListRecursively(instructionList, mapPairNode->childList.GetHead()->GetNext()->GetNext()->value);

					// Now insert the data value at the field value.  Field and data values are popped; the map value remains on the stack top.
					MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>();
					AssemblyData::Entry entry;
					entry.code = MapInstruction::Action::INSERT;
					mapInstruction->assemblyData->configMap.Insert("action", entry);
					instructionList.AddTail(mapInstruction);
				}
			}
		}
		else if (*syntaxNode->name == "map-keys-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"map-keys-expression\" in AST to have exactly 2 children.", &syntaxNode->fileLocation);

			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);

			MapInstruction* mapInstruction = Instruction::CreateForAssembly<MapInstruction>();
			AssemblyData::Entry entry;
			entry.code = MapInstruction::Action::MAKE_KEY_LIST;
			mapInstruction->assemblyData->configMap.Insert("action", entry);
			instructionList.AddTail(mapInstruction);
		}
		else if (*syntaxNode->name == "list-push-pop-expression")
		{
			if (syntaxNode->childList.GetCount() != 3)
				throw new CompileTimeException("Expected \"list-push-pop-expression\" in AST to have exactly 3 children.", &syntaxNode->fileLocation);

			const Parser::SyntaxNode* actionNode = syntaxNode->childList.GetHead()->GetNext()->value;
			if (*actionNode->name == "-->" || *actionNode->name == "<--")
			{
				// Push the list onto the eval stack.  Note that if it's not a list, we'll only know at run-time.
				const Parser::SyntaxNode* listNode = (*actionNode->name == "-->") ? syntaxNode->childList.GetHead()->value : syntaxNode->childList.GetHead()->GetNext()->GetNext()->value;
				LinkedList<Instruction*> listInstructionList;
				this->GenerateInstructionListRecursively(listInstructionList, listNode);
				instructionList.Append(listInstructionList);

				// Now issue the list instruction to pop left or right.  The list is replaced with the popped value on the eval stack.
				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>();
				AssemblyData::Entry entry;
				entry.code = (*actionNode->name == "-->") ? ListInstruction::POP_RIGHT : ListInstruction::POP_LEFT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(listInstruction);

				// TODO: Support "list --> other_container[key]"?

				// Lastly, store the popped value into the given identifier.
				const Parser::SyntaxNode* identifierNode = (*actionNode->name == "-->") ? syntaxNode->childList.GetHead()->GetNext()->GetNext()->value : syntaxNode->childList.GetHead()->value;
				if (*identifierNode->name != "identifier")
					throw new CompileTimeException(FormatString("List pop expected to store value in location given by name, but got no identifier.  Got \"%s\" instead.", identifierNode->name->c_str()), &syntaxNode->fileLocation);
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
				entry.Reset();
				entry.string = *identifierNode->childList.GetHead()->value->name;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);
			}
			else if (*actionNode->name == "--<" || *actionNode->name == ">--")
			{
				// Push the list onto the eval stack.  Note that if it's not a list, we'll only know at run-time.
				const Parser::SyntaxNode* listNode = (*actionNode->name == "--<") ? syntaxNode->childList.GetHead()->value : syntaxNode->childList.GetHead()->GetNext()->GetNext()->value;
				LinkedList<Instruction*> listInstructionList;
				this->GenerateInstructionListRecursively(listInstructionList, listNode);
				instructionList.Append(listInstructionList);

				// Now push the element onto the eval stack.  This can be anything, even another list or map.
				const Parser::SyntaxNode* elementNode = (*actionNode->name == "--<") ? syntaxNode->childList.GetHead()->GetNext()->GetNext()->value : syntaxNode->childList.GetHead()->value;
				LinkedList<Instruction*> elementInstructionList;
				this->GenerateInstructionListRecursively(elementInstructionList, elementNode);
				instructionList.Append(elementInstructionList);

				// Now issue the push instruction.  The element value will be consumed, and the list will remain on the eval stack.
				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>();
				AssemblyData::Entry entry;
				entry.code = (*actionNode->name == "--<") ? ListInstruction::PUSH_RIGHT : ListInstruction::PUSH_LEFT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				instructionList.AddTail(listInstruction);

				// Now check our context.  If we're an immediate child of a statement-list, then no one wants the list anymore.  Don't leak the list on the eval-stack.
				if (syntaxNode->parentNode && *syntaxNode->parentNode->name == "statement-list")
				{
					PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
					instructionList.AddTail(popInstruction);
				}
			}
			else
			{
				throw new CompileTimeException(FormatString("Did not recognize action (%s) for list manipulation operation.", actionNode->name->c_str()), &actionNode->fileLocation);
			}
		}
		else if (*syntaxNode->name == "function-call")
		{
			if (syntaxNode->childList.GetCount() == 0)
				throw new CompileTimeException("Expected \"function-call\" in AST to have at least 1 child.", &syntaxNode->fileLocation);

			// This node may or may not be present.  Its absense simply means the call takes no arguments.
			const Parser::SyntaxNode* argListNode = syntaxNode->FindChild("argument-list", 1);

			// Special case: Is this a system call?
			SysCallInstruction::SysCall sysCall = SysCallInstruction::SysCall::UNKNOWN;
			if (*syntaxNode->childList.GetHead()->value->name == "identifier")
			{
				std::string funcName = *syntaxNode->childList.GetHead()->value->childList.GetHead()->value->name;
				sysCall = SysCallInstruction::TranslateAsSysCall(funcName);
			}

			if (sysCall != SysCallInstruction::SysCall::UNKNOWN)
			{
				// In the case of a system call, we pass all arguments on the eval-stack.
				if (argListNode)
				{
					for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
					{
						const Parser::SyntaxNode* argNode = node->value;
						this->GenerateInstructionListRecursively(instructionList, argNode);
					}
				}

				uint32_t argCountGiven = argListNode ? argListNode->childList.GetCount() : 0;
				uint32_t argCountExpected = SysCallInstruction::ArgumentCount(sysCall);
				if (argCountGiven != argCountExpected)
					throw new CompileTimeException(FormatString("System call 0x%04x takes %d arguments, not %d.", uint8_t(sysCall), argCountExpected, argCountGiven), &syntaxNode->childList.GetHead()->value->childList.GetHead()->value->fileLocation);

				// The system call should pop all its arguments off the evaluation stack.
				SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>();
				AssemblyData::Entry entry;
				entry.code = sysCall;
				sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
				instructionList.AddTail(sysCallInstruction);
			}
			else
			{
				// Push a list onto the eval-stack that will contain all the function arguments.  The function
				// itself is responsible for off-loading the list into its own named variables before executing.
				// Note that we do this even if the function doesn't take any arguments, because there is nothing
				// to stop a caller from trying to give it arguments anyway.  Any extra arguments are just ignored.
				PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				AssemblyData::Entry entry;
				entry.code = PushInstruction::DataType::EMPTY_LIST;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(pushInstruction);
				if (argListNode)
				{
					for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
					{
						const Parser::SyntaxNode* argNode = node->value;
						this->GenerateInstructionListRecursively(instructionList, argNode);
						ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>();
						entry.Reset();
						entry.code = ListInstruction::Action::PUSH_RIGHT;
						listInstruction->assemblyData->configMap.Insert("action", entry);
						instructionList.AddTail(listInstruction);
					}
				}

				// Push onto the eval-stack the address of the function to be called.  This could be a single
				// load instruction, or several instructions that ultimately leave an address on the stack top.
				this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);

				// Push scope so that the called function doesn't pollute the caller's name-space.
				ScopeInstruction* scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>();
				entry.Reset();
				entry.code = ScopeInstruction::ScopeOp::PUSH;
				scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
				instructionList.AddTail(scopeInstruction);

				// The return jump should jump to whatever instruction will end-up immediately after the jump instruction we make to actually call the function.
				pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				entry.Reset();
				entry.code = PushInstruction::DataType::ADDRESS;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				entry.jumpDelta = 3;
				entry.string = "data";
				pushInstruction->assemblyData->configMap.Insert("jump-delta", entry);
				instructionList.AddTail(pushInstruction);

				// Store the return address value in the newly pushed function scope.
				// Yes, this value could get poked by the calling function, but whatever, I can find a solution to that later.
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
				entry.Reset();
				entry.string = "__return_address__";
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);

				// Now make the call by jumping to the function at the run-time evaluated address.  All that
				// will remain on the eval-stack is the argument list value, which the called function is
				// responsible for popping off the eval-stack.
				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				entry.Reset();
				entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
				jumpInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(jumpInstruction);

				// Here now is the instruction that we will jump to when returning from the call.
				// The first thing we always do after returning from a call is to pop the function's scope.
				scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>();
				entry.Reset();
				entry.code = ScopeInstruction::ScopeOp::POP;
				scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
				instructionList.AddTail(scopeInstruction);
			}

			// We now look at the call in context to see if we need to leave the return result or clean it up.
			// Note that all functions will return a result whether a return statement is given or not.
			if (syntaxNode->FindParent("statement-list", 1) != nullptr)
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
				instructionList.AddTail(popInstruction);
			}
		}
		else if (*syntaxNode->name == "function-definition")
		{
			// Note that there is no real reason to enforce that function definitions appear at the root level.  We could allow them
			// to be defined anywhere, oddly.  But that's just it.  I don't want to create a false expectation that it matters where
			// a function definition is defined when in reality, it doesn't matter.  So just require them to always be at the root level.
			if (!syntaxNode->parentNode || *syntaxNode->parentNode->name != "statement-list")
				throw new CompileTimeException("Function definitions cannot appear anywhere but at the root level of a source file.", &syntaxNode->fileLocation);

			// Collect the function definitions in a list to be processed later, because we want them to appear at the end of the executable.
			// Otherwise, if we just layed them down wherever they were, we'd have to jump over them, and that would be stupid.
			this->functionDefinitionList.AddTail(syntaxNode);
		}
		else if (*syntaxNode->name == "return-statement")
		{
			if (syntaxNode->childList.GetCount() == 2)
				this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->value);
			else
			{
				PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				AssemblyData::Entry entry;
				entry.code = PushInstruction::DataType::UNDEFINED;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(pushInstruction);
			}

			this->GenerateFunctionReturnInstructions(instructionList);
		}
		else if (*syntaxNode->name == "membership-expression")
		{
			if (syntaxNode->childList.GetCount() != 3)
				throw new CompileTimeException("Expected \"membership-expression\" node in AST to have exactly 3 children.", &syntaxNode->fileLocation);

			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);
			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

			MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>();
			AssemblyData::Entry entry;
			entry.code = MathInstruction::CONTAINS;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);
		}
		else
		{
			throw new CompileTimeException(FormatString("Instruction generator does not yet handle case \"%s\".", syntaxNode->name->c_str()), &syntaxNode->fileLocation);
		}
	}

	void InstructionGenerator::GenerateFunctionReturnInstructions(LinkedList<Instruction*>& instructionList)
	{
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>();
		AssemblyData::Entry entry;
		entry.string = "__return_address__";
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(loadInstruction);

		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(jumpInstruction);
	}
}