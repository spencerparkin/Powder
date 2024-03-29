#include "FunctionDefinitionExpressionHandler.h"
#include "PushInstruction.h"
#include "ListInstruction.h"
#include "LoadInstruction.h"
#include "StoreInstruction.h"
#include "PopInstruction.h"
#include "LoadInstruction.h"
#include "JumpInstruction.h"
#include "ScopeInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	FunctionDefinitionExpressionHandler::FunctionDefinitionExpressionHandler()
	{
	}

	/*virtual*/ FunctionDefinitionExpressionHandler::~FunctionDefinitionExpressionHandler()
	{
	}

	/*virtual*/ bool FunctionDefinitionExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		// This isn't a fool-proof check, but it's something.
		if (syntaxNode->FindParent("statement-list", 1) != nullptr)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Anonymous function not assigned at definition.");
			return false;
		}

		// Push the address of the function onto the eval-stack.  This allows for storing it in a variable or just immediately calling it.
		PushInstruction* funcAddressPushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		const ParseParty::Parser::SyntaxNode* captureListNode = syntaxNode->FindChild("capture-list", 1);
		AssemblyData::Entry entry;
		entry.code = (captureListNode != nullptr) ? PushInstruction::DataType::CLOSURE : PushInstruction::DataType::ADDRESS;
		funcAddressPushInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(funcAddressPushInstruction);

		// If we're dealing with a closure here, it's now time to capture values for the closure.
		if (captureListNode)
		{
			// Push scope that will capture the values for the closure.
			ScopeInstruction* scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>(captureListNode->fileLocation);
			entry.Reset();
			entry.code = ScopeInstruction::ScopeOp::PUSH;
			scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
			instructionList.AddTail(scopeInstruction);

			// Capture the desired values from the scope containing the current scope in the current scope.
			for (const ParseParty::Parser::SyntaxNode* captureNode : *captureListNode->childList)
			{
				if (*captureNode->text != "@identifier")
				{
					error.Add(std::string(captureNode->fileLocation) + "Expected all children of \"capture-list\" in AST to be \"@identifier\".");
					return false;
				}

				entry.Reset();
				entry.string = *captureNode->GetChild(0)->text;
				LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(captureNode->fileLocation);
				loadInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(loadInstruction);
				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(captureNode->fileLocation);
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				instructionList.AddTail(storeInstruction);
			}

			// Bind the current scope to the closure.
			scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>(captureListNode->fileLocation);
			entry.Reset();
			entry.code = ScopeInstruction::ScopeOp::BIND;
			scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
			instructionList.AddTail(scopeInstruction);

			// Lastly, pop the scope bound to the closure.
			scopeInstruction = Instruction::CreateForAssembly<ScopeInstruction>(captureListNode->fileLocation);
			entry.Reset();
			entry.code = ScopeInstruction::ScopeOp::POP;
			scopeInstruction->assemblyData->configMap.Insert("scopeOp", entry);
			instructionList.AddTail(scopeInstruction);
		}

		// Typically we'll just be storing the function address rather than calling it immediately, so we'll want to jump over the definition.
		JumpInstruction* hopOverFuncInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::Type::JUMP_TO_EMBEDDED_ADDRESS;
		hopOverFuncInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(hopOverFuncInstruction);

		// Make sure a definition is given in the AST.
		const ParseParty::Parser::SyntaxNode* functionStatementListNode = syntaxNode->FindChild("statement-list", 2);
		if (!functionStatementListNode)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"function-definition\" to have \"statement-list\" in AST.");
			return false;
		}

		// The instructions of a function consist of the code for off-loading the arguments, then the function body.
		LinkedList<Instruction*> functionInstructionList;
		
		// If this doesn't exist, then the function doesn't take any arguments.
		const ParseParty::Parser::SyntaxNode* argListNode = syntaxNode->FindChild("identifier-list", 1);
		if (argListNode)
		{
			for (const ParseParty::Parser::SyntaxNode* argNode : *argListNode->childList)
			{
				if (*argNode->text != "@identifier")
				{
					DeleteList<Instruction*>(functionInstructionList);
					error.Add(std::string(argNode->fileLocation) + "Expected all children of \"identifier-list\" in AST to be \"@identifier\".");
					return false;
				}

				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(argNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = ListInstruction::Action::POP_LEFT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				functionInstructionList.AddTail(listInstruction);

				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(argNode->fileLocation);
				entry.Reset();
				entry.string = *argNode->GetChild(0)->text;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				functionInstructionList.AddTail(storeInstruction);
			}
		}

		// Don't leak the argument list.
		PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
		functionInstructionList.AddTail(popInstruction);

		// Lay down the function body.
		if (!instructionGenerator->GenerateInstructionListRecursively(functionInstructionList, functionStatementListNode, error))
		{
			DeleteList<Instruction*>(functionInstructionList);
			error.Add(std::string(functionStatementListNode->fileLocation) + "Failed to generate instructions for function body.");
			return false;
		}

		// The end of the function might already have a return, but if it doesn't, this doesn't hurt.
		// We ensure that all functions return a value no matter what, because the calling code will always
		// expect it, even if it is going to discard it.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::NULL_VALUE;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		functionInstructionList.AddTail(pushInstruction);

		// The caller should have set this up for us.
		LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.string = "__return_address__";
		loadInstruction->assemblyData->configMap.Insert("name", entry);
		functionInstructionList.AddTail(loadInstruction);

		// Jump back to the caller, just after the jump that makes the call.
		JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::JUMP_TO_LOADED_ADDRESS;
		jumpInstruction->assemblyData->configMap.Insert("type", entry);
		functionInstructionList.AddTail(jumpInstruction);

		// Lay down the instructions for the function.  They're just injected right where the definition is encountered.
		instructionList.Append(functionInstructionList);

		// Patch the push instruction that effectively declares the anonymous function.
		entry.Reset();
		entry.instruction = functionInstructionList.GetHead()->value;
		funcAddressPushInstruction->assemblyData->configMap.Insert("data", entry);

		// Lastly, patch the jump instruction that jumps over the function definition.
		entry.Reset();
		entry.jumpDelta = functionInstructionList.GetCount() + 1;
		entry.string = "jump";
		hopOverFuncInstruction->assemblyData->configMap.Insert("jump-delta", entry);

		return true;
	}
}