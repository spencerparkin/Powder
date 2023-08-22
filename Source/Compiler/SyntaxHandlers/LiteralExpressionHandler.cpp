#include "LiteralExpressionHandler.h"
#include "PopInstruction.h"
#include "PushInstruction.h"
#include "Assembler.h"

namespace Powder
{
	LiteralExpressionHandler::LiteralExpressionHandler()
	{
	}

	/*virtual*/ LiteralExpressionHandler::~LiteralExpressionHandler()
	{
	}

	/*virtual*/ void LiteralExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 1)
			throw new CompileTimeException("Expected \"literal\" in AST to have exactly one child.", &syntaxNode->fileLocation);

		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(pushInstruction);

		const ParseParty::Parser::SyntaxNode* literalTypeNode = syntaxNode->GetChild(0);
		const ParseParty::Parser::SyntaxNode* literalDataNode = (literalTypeNode->childList.GetCount() == 1) ? literalTypeNode->GetChild(0) : nullptr;

		AssemblyData::Entry typeEntry;
		AssemblyData::Entry dataEntry;

		if (*literalTypeNode->text == "undefined")
			typeEntry.code = PushInstruction::DataType::UNDEFINED;
		else if (*literalTypeNode->text == "string-literal")
		{
			typeEntry.code = PushInstruction::DataType::STRING;
			dataEntry.string = *literalDataNode->text;
		}
		else if (*literalTypeNode->text == "number-literal")
		{
			typeEntry.code = PushInstruction::DataType::NUMBER;
			dataEntry.number = ::strtod(literalDataNode->text->c_str(), nullptr);
		}
		else if (*literalTypeNode->text == "list-literal")
			typeEntry.code = PushInstruction::DataType::EMPTY_LIST;
		else if (*literalTypeNode->text == "map-literal")
			typeEntry.code = PushInstruction::DataType::EMPTY_MAP;
		else
			throw new CompileTimeException(FormatString("Did not recognize \"%s\" data-type under \"literal\" in AST.", literalTypeNode->text->c_str()), &literalTypeNode->fileLocation);

		pushInstruction->assemblyData->configMap.Insert("type", typeEntry);
		pushInstruction->assemblyData->configMap.Insert("data", dataEntry);

		if (*literalTypeNode->text == "list-literal" || *literalTypeNode->text == "map-literal")
		{
			// In this case, next come the instructions that populate the list or map.
			instructionGenerator->GenerateInstructionListRecursively(instructionList, literalTypeNode);
		}

		// If the literal is not in the context of an assignment or expression of some kind, then we
		// need to issue a pop instruction here in order to not leak a value on the eval stack.
		if (this->PopNeededForExpression(syntaxNode))
		{
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}
	}
}