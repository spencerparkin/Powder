#include "InstructionGenerator.h"
#include "Assembler.h"
#include "BranchInstruction.h"
#include "JumpInstruction.h"
#include "LoadInstruction.h"
#include "PushInstruction.h"
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
					throw new CompileTimeException("Expected \"statement\" under \"statement-list\" in AST");

				// We simply execute the code for each statement in order.
				this->GenerateInstructionList(instructionList, node->value);
			}
		}
		else if (*syntaxNode->name == "statement")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"statement\" in AST to have exactly one child.");

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "if-statement")
		{
			if (syntaxNode->childList.GetCount() != 3 && syntaxNode->childList.GetCount() != 5)
				throw new CompileTimeException("Expected \"if-statement\" in AST to have exactly 3 or 5 children.");

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
				throw new CompileTimeException("Expected \"expression\" in AST to have exactly one child.");

			this->GenerateInstructionList(instructionList, syntaxNode->childList.GetHead()->value);
		}
		else if (*syntaxNode->name == "binary-expression")
		{
			//...
		}
		else if (*syntaxNode->name == "unary-expression")
		{
			//...
		}
		else if (*syntaxNode->name == "literal")
		{
			if (syntaxNode->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"literal\" in AST to have exactly one child.");

			if (syntaxNode->childList.GetHead()->value->childList.GetCount() != 1)
				throw new CompileTimeException("Expected \"literal\" in AST to have exactly one grandchild.");

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
				throw new CompileTimeException(FormatString("Did not recognize \"%s\" data-type under \"literal\" in AST.", literalTypeNode->name->c_str()));
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
				throw new CompileTimeException("Expected \"identifier\" in AST to have exactly one child.");

			// TODO: Make sure that we're generating code in the context of an expression.  Look up the parent chain.

			const Parser::SyntaxNode* identifierNode = syntaxNode->childList.GetHead()->value;
			LoadInstruction* loadInstruction = Instruction::CreateForAssembly<LoadInstruction>();
			AssemblyData::Entry entry;
			entry.string = *identifierNode->name;
			loadInstruction->assemblyData->configMap.Insert("name", entry);
			instructionList.AddTail(loadInstruction);
		}
		else if (*syntaxNode->name == "list-literal")
		{
			//...
		}
	}
}