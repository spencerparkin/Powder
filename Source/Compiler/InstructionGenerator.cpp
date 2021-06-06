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
#include "ScopeInstruction.h"
#include "Exceptions.hpp"
#include "HashMap.hpp"

namespace Powder
{
	InstructionGenerator::InstructionGenerator()
	{
	}

	/*virtual*/ InstructionGenerator::~InstructionGenerator()
	{
		this->functionSignatureMap.DeleteAndClear();
	}

	void InstructionGenerator::GenerateInstructionList(LinkedList<Instruction*>& instructionList, HashMap<Instruction*>& functionMap, const Parser::SyntaxNode* rootSyntaxNode)
	{
		this->GatherAllFunctionSignatures(rootSyntaxNode);

		this->GenerateInstructionListRecursively(instructionList, rootSyntaxNode);

		if (this->functionDefinitionList.GetCount() > 0)
		{
			// Issue a halt instruction here so that we don't crash into the first subroutine of the program.
			SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>();
			AssemblyData::Entry entry;
			entry.code = SysCallInstruction::SysCall::EXIT;
			sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
			instructionList.AddTail(sysCallInstruction);

			for (LinkedList<const Parser::SyntaxNode*>::Node* node = this->functionDefinitionList.GetHead(); node; node = node->GetNext())
			{
				const Parser::SyntaxNode* functionDefNode = node->value;
				const Parser::SyntaxNode* functionStatementListNode = functionDefNode->FindChild("statement-list", 2);
				if (!functionStatementListNode)
					throw new CompileTimeException("Expected \"function-definition\" to have \"statement-list\" in AST.", &functionDefNode->fileLocation);

				const Parser::SyntaxNode* identifierNode = functionDefNode->FindChild("identifier", 1);
				if (!identifierNode)
					throw new CompileTimeException("Expected \"function-definition\" in AST to have \"identifier\" child.", &functionDefNode->fileLocation);

				std::string funcName = *identifierNode->childList.GetHead()->value->name;

				LinkedList<Instruction*> functionInstructionList;
				this->GenerateInstructionListRecursively(functionInstructionList, node->value);

				// Populate a map we'll use later to resolve function call jumps.
				functionMap.Insert(funcName.c_str(), functionInstructionList.GetHead()->value);

				// The end of the function might already have a return, but if it doesn't, this doesn't hurt.
				PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				AssemblyData::Entry entry;
				entry.code = PushInstruction::DataType::UNDEFINED;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				instructionList.AddTail(pushInstruction);
				this->GenerateFunctionReturnInstructions(instructionList);

				// Finally, lay down the instructions for the function.
				instructionList.Append(functionInstructionList);
			}
		}
	}

	void InstructionGenerator::GenerateInstructionListRecursively(LinkedList<Instruction*>& instructionList, const Parser::SyntaxNode* syntaxNode)
	{
		if (*syntaxNode->name == "statement-list")
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
			{
				if (*node->value->name != "statement")
					throw new CompileTimeException("Expected \"statement\" under \"statement-list\" in AST", &node->value->fileLocation);

				// We simply execute the code for each statement in order.
				this->GenerateInstructionListRecursively(instructionList, node->value);
			}
		}
		else if (*syntaxNode->name == "statement")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"statement\" in AST to have exactly one child.", &syntaxNode->fileLocation);

			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);
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
				this->GenerateInstructionListRecursively(failInstructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->GetNext()->GetNext()->value);
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

			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);
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
				this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->GetNext()->GetNext()->value);

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
		else if (*syntaxNode->name == "unary-expression")
		{
			if (syntaxNode->childList.GetCount() != 2)
				throw new CompileTimeException("Expected \"unary-expression\" in AST to have exactly 1 child.", &syntaxNode->fileLocation);

			this->GenerateInstructionListRecursively(instructionList, syntaxNode->childList.GetHead()->value);
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
				this->GenerateInstructionListRecursively(instructionList, literalTypeNode);
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

			if (identifierNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"identifier\" node of \"function-call\" node in AST to have exactly 1 child.", &identifierNode->fileLocation);

			std::string funcName = *identifierNode->childList.GetHead()->value->name;
			SysCallInstruction::SysCall sysCall = SysCallInstruction::TranslateAsSysCall(funcName);

			const FunctionSignature* functionSignature = nullptr;

			if (sysCall == SysCallInstruction::UNKNOWN)
			{
				ScopeInstruction* scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>();
				AssemblyData::Entry entry;
				entry.code = ScopeInstruction::ScopeOp::PUSH;
				scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
				instructionList.AddTail(scopeInstruction);

				functionSignature = this->functionSignatureMap.Lookup(funcName.c_str());
				if (!functionSignature)
					throw new CompileTimeException(FormatString("Failed to look-up function signature for function \"%s\".", funcName.c_str()), &identifierNode->fileLocation);
			}

			uint32_t argCount = 0;

			const Parser::SyntaxNode* argListNode = syntaxNode->FindChild("argument-list", 1);
			if (argListNode)
			{
				for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
				{
					const Parser::SyntaxNode* argNode = node->value;
					if (*argNode->name != "argument")
						throw new CompileTimeException("Expected \"argument-list\" in AST to have only \"argument\" children.", &argNode->fileLocation);

					if (!argNode->childList.GetCount() == 1)
						throw new CompileTimeException("Expected \"argument\" in AST to have exactly one child.", &argNode->fileLocation);

					if (*argNode->childList.GetHead()->value->name != "expression" && *argNode->childList.GetHead()->value->name != "function-call")
						throw new CompileTimeException("Expected \"argument\" of system call to be an \"expression\" or \"function-call\".", &argNode->fileLocation);

					this->GenerateInstructionListRecursively(instructionList, argNode->childList.GetHead()->value);

					// System calls are passed their arguments on the eval stack, but user-defined functions are passed named arguments by scope stack.
					if (sysCall == SysCallInstruction::UNKNOWN && argCount < functionSignature->namedArgsArray.size())
					{
						StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
						AssemblyData::Entry entry;
						entry.string = functionSignature->namedArgsArray[argCount++];
						storeInstruction->assemblyData->configMap.Insert("name", entry);
						instructionList.AddTail(storeInstruction);
					}
				}
			}			

			if (sysCall != SysCallInstruction::SysCall::UNKNOWN)
			{
				argCount = (argListNode ? argListNode->childList.GetCount() : 0);
				int sysCallArgCount = SysCallInstruction::ArgumentCount(sysCall);
				if (argCount != sysCallArgCount)
					throw new CompileTimeException(FormatString("System call 0x%04x takes %d arguments, not %d.", uint8_t(sysCall), sysCallArgCount, argCount), &identifierNode->fileLocation);

				// The system call should pop all its arguments off the evaluation stack.
				SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>();
				AssemblyData::Entry entry;
				entry.code = sysCall;
				sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
				instructionList.AddTail(sysCallInstruction);
			}
			else
			{
				if (functionSignature->namedArgsArray.size() != argCount)
					throw new CompileTimeException(FormatString("Function \"%s\" takes %d arguments, but %d were given.", funcName.c_str(), functionSignature->namedArgsArray.size(), argCount), &syntaxNode->fileLocation);

				// Jump to whatever instruction will end-up after the jump instruction we make to actually call the function.
				PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>();
				AssemblyData::Entry entry;
				entry.code = PushInstruction::DataType::ADDRESS;
				pushInstruction->assemblyData->configMap.Insert("type", entry);
				entry.jumpDelta = 3;
				entry.string = "data";
				pushInstruction->assemblyData->configMap.Insert("jump-delta", entry);
				instructionList.AddTail(pushInstruction);

				// Yes, this value could get poked by the calling function, but whatever, I can find a solution to that later.
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>();
				entry.string = "__return_address__";
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);

				// We will resolve the jump address in a separate pass, once we've become aware of all user-defined functions.
				JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
				entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
				jumpInstruction->assemblyData->configMap.Insert("type", entry);
				entry.string = funcName;
				jumpInstruction->assemblyData->configMap.Insert("jump-func", entry);
				jumpInstruction->assemblyData->fileLocation = identifierNode->fileLocation;
				instructionList.AddTail(jumpInstruction);

				// We now look at the call in context to see if we need to leave the return result or clean it up.
				// For now, the only case I can see is when we make the call as the entirety of a program statement.
				if (syntaxNode->FindParent("statement", 2) != nullptr)
				{
					PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>();
					instructionList.AddTail(popInstruction);
				}
			}
		}
		else if (*syntaxNode->name == "function-definition")
		{
			// I may revisit the idea of functions and try to make a variety of them (lambdas?) that are first-class
			// citizens of the language (i.e., a type of value that can float around the VM and be manipulated or calculated with, etc.)

			// Note that there is no real reason to enforce that function definitions appear at the root level.  We could allow them
			// to be defined anywhere, oddly.  But that's just it.  I don't want to create a false expectation that it matters where
			// a function definition is defined when in reality, it doesn't matter.  So just require them to always be at the root level.
			if (syntaxNode->parentNode->parentNode->parentNode != nullptr)
				throw new CompileTimeException("Function definitions cannot appear anywhere but at the root level of a source file.", &syntaxNode->fileLocation);

			// For now we just collect the function definitions, because we want them to
			// appear at the end of the executable.  Otherwise, if we just layed them down
			// wherever they were, we'd have to jump over them.
			this->functionDefinitionList.AddTail(syntaxNode);
		}
		else if (*syntaxNode->name == "return-statement")
		{
			if (syntaxNode->childList.GetCount() == 2 && *syntaxNode->childList.GetHead()->GetNext()->value->name == "expression")
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
	}

	void InstructionGenerator::GenerateFunctionReturnInstructions(LinkedList<Instruction*>& instructionList)
	{
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>();
		AssemblyData::Entry entry;
		entry.string = "__return_address__";
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(loadInstruction);

		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>();
		entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(jumpInstruction);
	}

	void InstructionGenerator::GatherAllFunctionSignatures(const Parser::SyntaxNode* syntaxNode)
	{
		if (*syntaxNode->name == "function-definition")
		{
			const Parser::SyntaxNode* identifierNode = syntaxNode->FindChild("identifier", 1);
			if (!identifierNode)
				throw new CompileTimeException("Expected \"function-definition\" in AST to have \"identifier\" child.", &syntaxNode->fileLocation);

			std::string funcName = *identifierNode->childList.GetHead()->value->name;
			FunctionSignature* functionSignature = new FunctionSignature();

			const Parser::SyntaxNode* identifierListNode = syntaxNode->FindChild("identifier-list", 2);
			if (identifierListNode)
			{
				for (const LinkedList<Parser::SyntaxNode*>::Node* node = identifierListNode->childList.GetHead(); node; node = node->GetNext())
					functionSignature->namedArgsArray.push_back(*node->value->name);
			}

			this->functionSignatureMap.Insert(funcName.c_str(), functionSignature);
		}

		for (const LinkedList<Parser::SyntaxNode*>::Node* node = syntaxNode->childList.GetHead(); node; node = node->GetNext())
			this->GatherAllFunctionSignatures(node->value);
	}
}