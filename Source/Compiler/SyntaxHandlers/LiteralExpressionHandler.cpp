#include "LiteralExpressionHandler.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	LiteralExpressionHandler::LiteralExpressionHandler()
	{
	}

	/*virtual*/ LiteralExpressionHandler::~LiteralExpressionHandler()
	{
	}

	/*virtual*/ bool LiteralExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		if (syntaxNode->GetChildCount() != 1)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"literal\" in AST to have exactly one child.");
			return false;
		}

		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(pushInstruction);

		const ParseParty::Parser::SyntaxNode* literalTypeNode = syntaxNode->GetChild(0);
		const ParseParty::Parser::SyntaxNode* literalDataNode = (literalTypeNode->GetChildCount() == 1) ? literalTypeNode->GetChild(0) : nullptr;

		AssemblyData::Entry typeEntry;
		AssemblyData::Entry dataEntry;

		if (*literalTypeNode->text == "null")
			typeEntry.code = PushInstruction::DataType::NULL_VALUE;
		else if (*literalTypeNode->text == "@string")
		{
			typeEntry.code = PushInstruction::DataType::STRING;
			dataEntry.string = *literalDataNode->text;
		}
		else if (*literalTypeNode->text == "@number")
		{
			typeEntry.code = PushInstruction::DataType::NUMBER;
			dataEntry.number = ::strtod(literalDataNode->text->c_str(), nullptr);
		}
		else if (*literalTypeNode->text == "bool-literal")
		{
			typeEntry.code = PushInstruction::DataType::BOOL;
			if (*literalDataNode->text == "true")
				dataEntry.number = 1.0;
			else if (*literalDataNode->text == "false")
				dataEntry.number = 0.0;
			else
			{
				error.Add(std::string(literalDataNode->fileLocation) + std::format("Expected bool literal to be \"true\" or \"false\", not \"{}\".", literalDataNode->text->c_str()));
				return false;
			}
		}
		else if (*literalTypeNode->text == "list-literal")
			typeEntry.code = PushInstruction::DataType::EMPTY_LIST;
		else if (*literalTypeNode->text == "map-literal")
			typeEntry.code = PushInstruction::DataType::EMPTY_MAP;
		else if (*literalTypeNode->text == "set-literal")
			typeEntry.code = PushInstruction::DataType::EMPTY_SET;
		else
		{
			error.Add(std::string(literalTypeNode->fileLocation) + std::format("Did not recognize \"{}\" data-type under \"literal\" in AST.", literalTypeNode->text->c_str()));
			return false;
		}

		pushInstruction->assemblyData->configMap.Insert("type", typeEntry);
		pushInstruction->assemblyData->configMap.Insert("data", dataEntry);

		if (*literalTypeNode->text == "list-literal" || *literalTypeNode->text == "map-literal" || *literalTypeNode->text == "set-literal")
		{
			// In this case, next come the instructions that populate the list, map or set.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, literalTypeNode, error))
			{
				error.Add(std::string(literalTypeNode->fileLocation) + "Failed to generate instruction to populate list or map.");
				return false;
			}
		}

		// If the literal is not in the context of an assignment or expression of some kind, then we
		// need to issue a pop instruction here in order to not leak a value on the eval stack.
		if (this->PopNeededForExpression(syntaxNode))
		{
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}

		return true;
	}
}