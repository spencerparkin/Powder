#include "BreakOrContinueStatementHandler.h"
#include "JumpInstruction.h"
#include "Error.h"

using namespace Powder;

BreakOrContinueStatementHandler::BreakOrContinueStatementHandler()
{
}

/*virtual*/ BreakOrContinueStatementHandler::~BreakOrContinueStatementHandler()
{
}

/*virtual*/ bool BreakOrContinueStatementHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
{
	if (*syntaxNode->text != "break-statement" && *syntaxNode->text != "continue-statement")
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Sanity check for break and continue statement syntax handler failed.");
		return false;
	}

	const ParseParty::Parser::SyntaxNode* loopConstructNode = syntaxNode->GetParent();
	while (loopConstructNode)
	{
		if (*loopConstructNode->text == "for-statement" || *loopConstructNode->text == "do-while-statement" || *loopConstructNode->text == "while-statement")
			break;

		loopConstructNode = loopConstructNode->GetParent();
	}

	if (!loopConstructNode)
	{
		error.Add(std::string(syntaxNode->fileLocation) + "Break or continue statement was not found in the context of a loop construct.");
		return false;
	}

	JumpInstruction* jumpInstruction = Instruction::CreateForAssembly<JumpInstruction>(syntaxNode->fileLocation);
	AssemblyData::Entry entry;
	entry.instruction = nullptr;	// We can't resolve the jump-to instruction here.  The bounding loop-construct handler will have to do that.
	entry.ptr = (void*)loopConstructNode;
	entry.string = *syntaxNode->text;
	jumpInstruction->assemblyData->configMap.Insert("jump", entry);
	entry.Reset();
	entry.code = JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
	jumpInstruction->assemblyData->configMap.Insert("type", entry);
	instructionList.AddTail(jumpInstruction);

	return true;
}