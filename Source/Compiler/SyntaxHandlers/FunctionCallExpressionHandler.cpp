#include "FunctionCallExpressionHandler.h"
#include "SysCallInstruction.h"
#include "PushInstruction.h"
#include "ScopeInstruction.h"
#include "JumpInstruction.h"
#include "StoreInstruction.h"
#include "ListInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"

namespace Powder
{
	FunctionCallExpressionHandler::FunctionCallExpressionHandler()
	{
	}

	/*virtual*/ FunctionCallExpressionHandler::~FunctionCallExpressionHandler()
	{
	}

	/*virtual*/ void FunctionCallExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->GetChildCount() == 0)
			throw new CompileTimeException("Expected \"function-call\" in AST to have at least 1 child.", &syntaxNode->fileLocation);

		// This node may or may not be present.  Its absense simply means the call takes no arguments.
		const ParseParty::Parser::SyntaxNode* argListNode = syntaxNode->FindChild("argument-list", 1);

		// Special case: Is this a system call?
		SysCallInstruction::SysCall sysCall = SysCallInstruction::SysCall::UNKNOWN;
		if (*syntaxNode->GetChild(0)->text == "identifier")
		{
			std::string funcName = *syntaxNode->GetChild(0)->GetChild(0)->text;
			sysCall = SysCallInstruction::TranslateAsSysCall(funcName);
		}

		if (sysCall != SysCallInstruction::SysCall::UNKNOWN)
		{
			// In the case of a system call, we pass all arguments on the eval-stack.
			if (argListNode)
			{
				for(const ParseParty::Parser::SyntaxNode* argNode : *argListNode->childList)
					instructionGenerator->GenerateInstructionListRecursively(instructionList, argNode);
			}

			uint32_t argCountGiven = argListNode ? argListNode->GetChildCount() : 0;
			uint32_t argCountExpected = SysCallInstruction::ArgumentCount(sysCall);
			if (argCountGiven != argCountExpected)
				throw new CompileTimeException(FormatString("System call 0x%04x takes %d arguments, not %d.", uint8_t(sysCall), argCountExpected, argCountGiven), &syntaxNode->GetChild(0)->GetChild(0)->fileLocation);

			// The system call should pop all its arguments off the evaluation stack.
			SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(syntaxNode->fileLocation);
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
			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
			AssemblyData::Entry entry;
			entry.code = PushInstruction::DataType::EMPTY_LIST;
			pushInstruction->assemblyData->configMap.Insert("type", entry);
			instructionList.AddTail(pushInstruction);
			if (argListNode)
			{
				for (const ParseParty::Parser::SyntaxNode* argNode : *argListNode->childList)
				{
					instructionGenerator->GenerateInstructionListRecursively(instructionList, argNode);
					ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(syntaxNode->fileLocation);
					entry.Reset();
					entry.code = ListInstruction::Action::PUSH_RIGHT;
					listInstruction->assemblyData->configMap.Insert("action", entry);
					instructionList.AddTail(listInstruction);
				}
			}

			// Push onto the eval-stack the address of the function to be called.  This could be a single
			// load instruction, or several instructions that ultimately leave an address on the stack top.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, syntaxNode->GetChild(0));

			// We're now ready to make the call.
			this->GenerateCallInstructions(instructionList, syntaxNode->fileLocation);
		}

		// We now look at the call in context to see if we need to leave the return result or clean it up.
		// Note that all functions will return a result whether a return statement is given or not.
		// If no return statement is given, undefined is returned by default.  It is very important that
		// we do not leak values on the eval-stack.  Statements always leave the eval-stack untouched.
		// Expressions should always leave exactly one value on top of the eval-stack.
		if(this->PopNeededForExpression(syntaxNode))
		{
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}
	}

	/*static*/ void FunctionCallExpressionHandler::GenerateCallInstructions(LinkedList<Instruction*>& instructionList, const ParseParty::Lexer::FileLocation& fileLocation)
	{
		// Push scope so that the called function doesn't pollute the caller's name-space.
		ScopeInstruction* scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>(fileLocation);
		AssemblyData::Entry entry;
		entry.code = ScopeInstruction::ScopeOp::PUSH;
		scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
		instructionList.AddTail(scopeInstruction);

		// The return jump should jump to whatever instruction will end-up immediately after the jump instruction we make to actually call the function.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::ADDRESS;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.jumpDelta = 3;
		entry.string = "data";
		pushInstruction->assemblyData->configMap.Insert("jump-delta", entry);
		instructionList.AddTail(pushInstruction);

		// Store the return address value in the newly pushed function scope.
		// Yes, this value could get poked by the calling function, but whatever, I can find a solution to that later.
		StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(fileLocation);
		entry.Reset();
		entry.string = "__return_address__";
		storeInstruction->assemblyData->configMap.Insert("name", entry);
		instructionList.AddTail(storeInstruction);

		// Now make the call by jumping to the function at the run-time evaluated address.  All that
		// will remain on the eval-stack is the argument list value, which the called function is
		// responsible for popping off the eval-stack.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		*jumpInstruction->assemblyData->debuggerHelp = "function_call";
		instructionList.AddTail(jumpInstruction);

		// Here now is the instruction that we will jump to when returning from the call.
		// The first thing we always do after returning from a call is to pop the function's scope.
		scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>(fileLocation);
		entry.Reset();
		entry.code = ScopeInstruction::ScopeOp::POP;
		scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
		*scopeInstruction->assemblyData->debuggerHelp = "function_return";
		instructionList.AddTail(scopeInstruction);
	}
}