#include "FunctionDefinitionExpressionHandler.h"
#include "PushInstruction.h"
#include "ListInstruction.h"
#include "StoreInstruction.h"
#include "PopInstruction.h"
#include "LoadInstruction.h"
#include "JumpInstruction.h"
#include "Assembler.h"

namespace Powder
{
	FunctionDefinitionExpressionHandler::FunctionDefinitionExpressionHandler()
	{
	}

	/*virtual*/ FunctionDefinitionExpressionHandler::~FunctionDefinitionExpressionHandler()
	{
	}

	/*virtual*/ void FunctionDefinitionExpressionHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		// This isn't a fool-proof check, but it's something.
		if (syntaxNode->FindParent("statement-list", 1) != nullptr)
			throw new CompileTimeException("Anonymous function not assigned at definition.", &syntaxNode->fileLocation);

		// Push the address of the function onto the eval-stack.  This allows for storing it in a variable or just immediately calling it.
		PushInstruction* funcAddressPushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		AssemblyData::Entry entry;
		entry.code = PushInstruction::DataType::ADDRESS;
		funcAddressPushInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(funcAddressPushInstruction);

		// Typically we'll just be storing the function address rather than calling it immediately, so we'll want to jump over the definition.
		JumpInstruction* hopeOverFuncInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = JumpInstruction::Type::JUMP_TO_EMBEDDED_ADDRESS;
		hopeOverFuncInstruction->assemblyData->configMap.Insert("type", entry);
		instructionList.AddTail(hopeOverFuncInstruction);

		// Make sure a definition is given in the AST.
		const Parser::SyntaxNode* functionStatementListNode = syntaxNode->FindChild("statement-list", 2);
		if (!functionStatementListNode)
			throw new CompileTimeException("Expected \"function-definition\" to have \"statement-list\" in AST.", &syntaxNode->fileLocation);

		// The instructions of a function consist of the code for off-loading the arguments, then the function body.
		LinkedList<Instruction*> functionInstructionList;
		
		// If this doesn't exist, then the function doesn't take any arguments.
		const Parser::SyntaxNode* argListNode = syntaxNode->FindChild("identifier-list", 1);
		if (argListNode)
		{
			for (const LinkedList<Parser::SyntaxNode*>::Node* node = argListNode->childList.GetHead(); node; node = node->GetNext())
			{
				const Parser::SyntaxNode* argNode = node->value;
				if (*argNode->name != "identifier")
					throw new CompileTimeException("Expected all children of \"identifier-list\" in AST to be \"identifier\".", &argNode->fileLocation);

				ListInstruction* listInstruction = Instruction::CreateForAssembly<ListInstruction>(argNode->fileLocation);
				AssemblyData::Entry entry;
				entry.code = ListInstruction::Action::POP_LEFT;
				listInstruction->assemblyData->configMap.Insert("action", entry);
				functionInstructionList.AddTail(listInstruction);

				StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(argNode->fileLocation);
				entry.Reset();
				entry.string = *argNode->childList.GetHead()->value->name;
				storeInstruction->assemblyData->configMap.Insert("name", entry);
				functionInstructionList.AddTail(storeInstruction);
			}
		}

		// Don't leak the argument list.
		PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
		functionInstructionList.AddTail(popInstruction);

		// Lay down the function body.
		instructionGenerator->GenerateInstructionListRecursively(functionInstructionList, functionStatementListNode);

		// The end of the function might already have a return, but if it doesn't, this doesn't hurt.
		// We ensure that all functions return a value no matter what, because the calling code will always
		// expect it, even if it is going to discard it.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::UNDEFINED;
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
		hopeOverFuncInstruction->assemblyData->configMap.Insert("jump-delta", entry);
	}
}