#include "BumpExpressionHandler.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
#include "MathInstruction.h"
#include "PushInstruction.h"
#include "PopInstruction.h"
#include "Assembler.h"
#include "Error.h"

namespace Powder
{
	BumpExpressionHandler::BumpExpressionHandler()
	{
	}

	/*virtual*/ BumpExpressionHandler::~BumpExpressionHandler()
	{
	}

	/*virtual*/ bool BumpExpressionHandler::HandleSyntaxNode(const ParseParty::Parser::SyntaxNode* syntaxNode, LinkedList<Instruction*>& instructionList, InstructionGenerator* instructionGenerator, Error& error)
	{
		AssemblyData::Entry entry;

		if (syntaxNode->GetChildCount() != 2)
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Expected \"bump-expression\" in AST to have exactly 2 children.");
			return false;
		}

		const ParseParty::Parser::SyntaxNode* storeLocationNode = nullptr;
		const ParseParty::Parser::SyntaxNode* operationNode = nullptr;

		// First, determine the parts of the expression.
		if (*syntaxNode->text == "pre-bump-expression")
		{
			operationNode = syntaxNode->GetChild(0);
			storeLocationNode = syntaxNode->GetChild(1);
		}
		else if (*syntaxNode->text == "post-bump-expression")
		{
			operationNode = syntaxNode->GetChild(1);
			storeLocationNode = syntaxNode->GetChild(0);
		}
		else
		{
			error.Add(std::string(syntaxNode->fileLocation) + std::format("Expected pre or post bump expression, not \"{}\".", syntaxNode->text->c_str()));
			return false;
		}

		if (*operationNode->text != "++" && *operationNode->text != "--")
		{
			error.Add(std::string(syntaxNode->fileLocation) + std::format("Bump operation \"{}\" not recognized.", operationNode->text->c_str()));
			return false;
		}

		// Second, load the value in question onto the top of the eval stack.
		if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode, error))
		{
			error.Add(std::string(syntaxNode->fileLocation) + "Failed to load bump value.");
			return false;
		}

		// In the case of post-bump, load it redundantly onto the eval stack.
		if (*syntaxNode->text == "post-bump-expression")
		{
			PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(storeLocationNode->fileLocation);
			entry.Reset();
			entry.code = PushInstruction::DataType::EXISTING_VALUE;
			pushInstruction->assemblyData->configMap.Insert("type", entry);
			entry.Reset();
			entry.offset = 0;
			pushInstruction->assemblyData->configMap.Insert("data", entry);
			instructionList.AddTail(pushInstruction);
		}

		// Now push a one onto the eval-stack.
		PushInstruction* pushInstruction = Instruction::CreateForAssembly<PushInstruction>(storeLocationNode->fileLocation);
		entry.Reset();
		entry.code = PushInstruction::DataType::NUMBER;
		pushInstruction->assemblyData->configMap.Insert("type", entry);
		entry.Reset();
		entry.number = 1.0;
		pushInstruction->assemblyData->configMap.Insert("data", entry);
		instructionList.AddTail(pushInstruction);

		// Finally, perform the bump.
		MathInstruction* mathInstruction = Instruction::CreateForAssembly<MathInstruction>(operationNode->fileLocation);
		entry.Reset();
		entry.code = (*operationNode->text == "++") ? MathInstruction::ADD : MathInstruction::SUBTRACT;
		mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
		instructionList.AddTail(mathInstruction);

		// We now store the result into the l-value.  Note that this also pops the calculated value.
		if (*storeLocationNode->text == "@identifier")
		{
			StoreInstruction* storeInstruction = Instruction::CreateForAssembly<StoreInstruction>(syntaxNode->fileLocation);
			entry.Reset();
			entry.string = *storeLocationNode->GetChild(0)->text;
			storeInstruction->assemblyData->configMap.Insert("name", entry);
			instructionList.AddTail(storeInstruction);
		}
		else if (*storeLocationNode->text == "container-field-expression")
		{
			// Push the container value.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->GetChild(0), error))
			{
				error.Add(std::string(storeLocationNode->fileLocation) + "Failed to generate instructions for container value.");
				return false;
			}

			// Push the field value.
			if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode->GetChild(1), error))
			{
				error.Add(std::string(storeLocationNode->fileLocation) + "Failed to generate instructions for the field value.");
				return false;
			}

			// The value to store is already in the stack, but not in the position the math instruction will expect, so push it now.
			pushInstruction = Instruction::CreateForAssembly<PushInstruction>(storeLocationNode->fileLocation);
			entry.Reset();
			entry.code = PushInstruction::DataType::EXISTING_VALUE;
			pushInstruction->assemblyData->configMap.Insert("type", entry);
			entry.Reset();
			entry.offset = 2;
			pushInstruction->assemblyData->configMap.Insert("data", entry);
			instructionList.AddTail(pushInstruction);

			// Note that this will pop the last 3 values pushed off the eval-stack.
			mathInstruction = Instruction::CreateForAssembly<MathInstruction>(operationNode->fileLocation);
			entry.Reset();
			entry.code = MathInstruction::SET_FIELD;
			mathInstruction->assemblyData->configMap.Insert("mathOp", entry);
			instructionList.AddTail(mathInstruction);

			// Now for consistency' sake with the other case, we need to pop the calculated value.
			PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
			instructionList.AddTail(popInstruction);
		}

		// Determine whether we need to leave a value on the eval-stack as a result of the expression.
		bool popNeeded = this->PopNeededForExpression(syntaxNode);

		// In the case of a pre-bump, we now need to load the bumped value as the result of the expression.
		if (*syntaxNode->text == "pre-bump-expression")
		{
			// That is, unless no one wants it.
			if (!popNeeded)
			{
				if (!instructionGenerator->GenerateInstructionListRecursively(instructionList, storeLocationNode, error))
				{
					error.Add(std::string(syntaxNode->fileLocation) + "Failed to load bump value.");
					return false;
				}
			}
		}
		else if (*syntaxNode->text == "post-bump-expression")
		{
			// In the case of post-bump, the result is already loaded, and we just pop it if no one wants it.
			if (popNeeded)
			{
				PopInstruction* popInstruction = Instruction::CreateForAssembly<PopInstruction>(syntaxNode->fileLocation);
				instructionList.AddTail(popInstruction);
			}
		}

		return false;
	}
}