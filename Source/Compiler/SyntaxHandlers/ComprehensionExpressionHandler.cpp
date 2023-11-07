#include "ComprehensionExpressionHandler.h"
#include "ForEachStatementHandler.h"
#include "FunctionCallExpressionHandler.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "SysCallInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "MathInstruction.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "ListInstruction.h"
#include "MapInstruction.h"
#include "Error.h"

using namespace Powder;

ComprehensionExpressionHandler::ComprehensionExpressionHandler()
{
}

/*virtual*/ ComprehensionExpressionHandler::~ComprehensionExpressionHandler()
{
}

/*virtual*/ bool ComprehensionExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	if (syntaxNode->GetChildCount() != 1)
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Expected comprehension expression to have exactly one child.");
		return false;
	}

	AssemblyData::Entry entry;

	const ParseParty::Parser::SyntaxNode* comprehensionTypeNode = syntaxNode->GetChild(0);
	if (*comprehensionTypeNode->text == "list-comprehension-expression")
		entry.code = PushInstruction::DataType::EMPTY_LIST;
	else if (*comprehensionTypeNode->text == "map-comprehension-expression")
		entry.code = PushInstruction::DataType::EMPTY_MAP;
	else if (*comprehensionTypeNode->text == "set-comprehension-expression")
		entry.code = PushInstruction::DataType::EMPTY_SET;
	else
	{
		error.Add(std::string(comprehensionTypeNode->fileLocation) + "Did not recognize comprehension type: " + *comprehensionTypeNode->text);
		return false;
	}

	auto pushInstruction = Instruction::CreateForAssembly<PushInstruction>(comprehensionTypeNode->fileLocation);
	pushInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(pushInstruction);

	const ParseParty::Parser::SyntaxNode* internalsNode = comprehensionTypeNode->GetChild(0);
	if (!internalsNode)
	{
		error.Add(std::string(comprehensionTypeNode->fileLocation) + "Did not find internals sub-node.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* elementNode = internalsNode->GetChild(0);
	if (!elementNode)
	{
		error.Add(std::string(internalsNode->fileLocation) + "Did not find element sub-node.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* iterationNode = internalsNode->GetChild(2);
	if (!iterationNode || *iterationNode->text != "for-iteration-expression")
	{
		error.Add(std::string(internalsNode->fileLocation) + "Did not find for-iteration-expression sub-node.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* identifierNode = iterationNode->GetChild(0);
	if (!identifierNode || *identifierNode->text != "@identifier")
	{
		error.Add(std::string(identifierNode->fileLocation) + "Expected \"@identifier\" in AST as child of \"for-iteration-expression\" node.");
		return false;
	}

	// Push the container value or iterator function onto the eval-stack.
	const ParseParty::Parser::SyntaxNode* iteratorNode = iterationNode->GetChild(2);
	if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, iteratorNode, error))
	{
		error.Add(std::string(iteratorNode->fileLocation) + "Failed to generate instructions to push container value or iterator function onto the eval stack.");
		return false;
	}

	// Convert it to an iterator if necessary.
	SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(iteratorNode->fileLocation);
	entry.Reset();
	entry.code = SysCallInstruction::AS_ITERATOR;
	sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
	instructionList.AddTail(sysCallInstruction);

	// Push an argument list onto the eval stack, then push the iterator function address on top of that.
	if (!ForEachStatementHandler::GenerateInstructionForIteratorCallSetup(instructionList, "reset", iterationNode->fileLocation, error))
	{
		error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instruction to setup iterator call.");
		return false;
	}

	// We're now ready to make the call to the iterator to reset iteration (to set it up for the first time, really.)
	if (!FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation, error))
	{
		error.Add(std::string(iterationNode->fileLocation) + "Failed to geneate instruction to make function call.");
		return false;
	}

	// The result of resetting iteration is always a value we throw away, so do that now.
	PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
	instructionList.AddTail(popInstruction);

	// We're now ready to begin the loop instructions.  Call the iterator to generate the first/next value.
	const LinkedList<Instruction*>::Node* forLoopHeadNode = instructionList.GetTail();
	if (!ForEachStatementHandler::GenerateInstructionForIteratorCallSetup(instructionList, "next", iterationNode->fileLocation, error))
	{
		DeleteList<Instruction*>(instructionList);
		error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instruction to setup iterator call.");
		return false;
	}

	if (!FunctionCallExpressionHandler::GenerateCallInstructions(instructionList, iterationNode->fileLocation, error))
	{
		DeleteList<Instruction*>(instructionList);
		error.Add(std::string(iterationNode->fileLocation) + "Failed to generate instructions to make iterator call.");
		return false;
	}

	// Store the returned value in the loop iteration variable.
	StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(iterationNode->fileLocation);
	entry.Reset();
	entry.string = *identifierNode->GetChild(0)->text;
	storeInstruction->assemblyData->configMap.Insert("name", entry);
	instructionList.AddTail(storeInstruction);

	// Iteration ends when the iteration variable becomes null, so test that now.
	pushInstruction = Instruction::CreateForAssembly<PushInstruction>(iterationNode->fileLocation);
	entry.Reset();
	entry.code = PushInstruction::NULL_VALUE;
	pushInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(pushInstruction);
	LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>(iterationNode->fileLocation);
	entry.Reset();
	entry.string = *identifierNode->GetChild(0)->text;
	loadInstruction->assemblyData->configMap.Insert("name", entry);
	instructionList.AddTail(loadInstruction);
	MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(iterationNode->fileLocation);
	entry.Reset();
	entry.code = MathInstruction::NOT_EQUAL;
	mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
	instructionList.AddTail(mathInstruction);
	BranchInstruction* loopExitBranchInstruction = Instruction::CreateForAssembly<BranchInstruction>(iterationNode->fileLocation);
	instructionList.AddTail(loopExitBranchInstruction);

	// The body of our for-loop is now a process of evaluating one or more expressions and then using them to populate a container.
	// But first, we need to check if a condition is given.
	if (internalsNode->GetChild(3) && *internalsNode->GetChild(3)->text == "if")
	{
		const ParseParty::Parser::SyntaxNode* conditionNode = internalsNode->GetChild(4);
		if (!conditionNode)
		{
			error.Add(std::string(internalsNode->GetChild(3)->fileLocation) + "Expected condition expression after \"if\" clause of comprehension.");
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, conditionNode, error))
		{
			error.Add(std::string(conditionNode->fileLocation) + "Failed to generate \"if\" clause of comprehension.");
			return false;
		}

		BranchInstruction* branchInstruction = Instruction::CreateForAssembly<BranchInstruction>(conditionNode->fileLocation);
		entry.Reset();
		entry.instruction = forLoopHeadNode->GetNext()->value;
		branchInstruction->assemblyData->configMap.Insert("branch", entry);
		instructionList.AddTail(branchInstruction);
	}

	// Presently, the iterator is at the stack top, but we need the container at the top.
	pushInstruction = Instruction::CreateForAssembly<PushInstruction>(elementNode->fileLocation);
	entry.Reset();
	entry.code = PushInstruction::EXISTING_VALUE;
	pushInstruction->assemblyData->configMap.Insert("type", entry);
	entry.Reset();
	entry.offset = 1;
	pushInstruction->assemblyData->configMap.Insert("data", entry);
	instructionList.AddTail(pushInstruction);

	// Run code now that will leave a value on the stack that we can use to populate a list or set, or two values on the stack for populating a map.
	if (*comprehensionTypeNode->text == "list-comprehension-expression" || *comprehensionTypeNode->text == "set-comprehension-expression")
	{
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, elementNode, error))
		{
			error.Add("Failed to generate instructions for element expression in comprehenstion.");
			return false;
		}
	}
	else if (*comprehensionTypeNode->text == "map-comprehension-expression")
	{
		if (*elementNode->text != "map-pair")
		{
			error.Add(std::string(elementNode->fileLocation) + "Expected map-pair as element node in map comprehension.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* keyExpressionNode = elementNode->GetChild(0);
		if (!keyExpressionNode)
		{
			error.Add(std::string(elementNode->fileLocation) + "Expected key expression node for element of map comprehension.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* valueExpressionNode = elementNode->GetChild(2);
		if (!valueExpressionNode)
		{
			error.Add(std::string(elementNode->fileLocation) + "Expected value expression node for element of map comprehension.");
			return false;
		}

		// We must push the key first, then the value.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, keyExpressionNode, error))
		{
			error.Add(std::string(keyExpressionNode->fileLocation) + "Failed to generate instructions for pushing key expression of map comprehension.");
			return false;
		}

		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, valueExpressionNode, error))
		{
			error.Add(std::string(valueExpressionNode->fileLocation) + "Failed to generate instructions for pushing value expression of map comprehension.");
			return false;
		}
	}

	// Load the value in the set or list, or the key/value pair in the map.
	if (*comprehensionTypeNode->text == "list-comprehension-expression")
	{
		auto listInstruction = Instruction::CreateForAssembly<ListInstruction>(elementNode->fileLocation);
		entry.Reset();
		entry.code = ListInstruction::Action::PUSH_RIGHT;
		listInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(listInstruction);
	}
	else if (*comprehensionTypeNode->text == "set-comprehension-expression")
	{
		SysCallInstruction* sysCallInstruction = Instruction::CreateForAssembly<SysCallInstruction>(elementNode->fileLocation);
		entry.Reset();
		entry.code = SysCallInstruction::AS_ITERATOR;
		sysCallInstruction->assemblyData->configMap.Insert("sysCall", entry);
		instructionList.AddTail(sysCallInstruction);
	}
	else if (*comprehensionTypeNode->text == "map-comprehension-expression")
	{
		auto mapInstruction = Instruction::CreateForAssembly<MapInstruction>(elementNode->fileLocation);
		entry.Reset();
		entry.code = MapInstruction::Action::INSERT;
		mapInstruction->assemblyData->configMap.Insert("action", entry);
		instructionList.AddTail(mapInstruction);
	}

	// Pop the container, because we need the iterator at the top of the stack now.
	popInstruction = Instruction::CreateForAssembly<PopInstruction>(elementNode->fileLocation);
	instructionList.AddTail(popInstruction);

	// Lastly, unconditionally jump back up to the top of the for-loop and do it all over again.
	JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(iterationNode->fileLocation);
	entry.Reset();
	entry.instruction = forLoopHeadNode->GetNext()->value;
	jumpInstruction->assemblyData->configMap.Insert("jump", entry);
	entry.Reset();
	entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
	jumpInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(jumpInstruction);

	// Finally, pop the iterator function off the eval-stack, leaving only the newly populated container behind.
	popInstruction = Instruction::CreateForAssembly<PopInstruction>(iterationNode->fileLocation);
	instructionList.AddTail(popInstruction);

	// We can now patch our branch that takes us out of the for-loop.
	entry.Reset();
	entry.instruction = popInstruction;
	loopExitBranchInstruction->assemblyData->configMap.Insert("branch", entry);

	return true;
}