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

	/*virtual*/ void LiteralExpressionHandler::HandleSyntaxNode(const Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator)
	{
		if (syntaxNode->childList.GetCount() != 1)
			throw new CompileTimeException("Expected \"literal\" in AST to have exactly one child.", &syntaxNode->fileLocation);

		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(syntaxNode->fileLocation);
		instructionList.AddTail(pushInstruction);

		const Parser::SyntaxNode* literalTypeNode = syntaxNode->childList.GetHead()->value;
		const Parser::SyntaxNode* literalDataNode = (literalTypeNode->childList.GetCount() == 1) ? literalTypeNode->childList.GetHead()->value : nullptr;

		AssemblyData::Entry typeEntry;
		AssemblyData::Entry dataEntry;

		if (*literalTypeNode->name == "undefined")
			typeEntry.code = PushInstruction::DataType::UNDEFINED;
		else if (*literalTypeNode->name == "string-literal")
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