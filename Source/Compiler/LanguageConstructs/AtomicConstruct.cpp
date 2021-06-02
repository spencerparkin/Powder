#include "AtomicConstruct.h"
#include "Tokenizer.h"

namespace Powder
{
	AtomicConstruct::AtomicConstruct()
	{
		this->token = new Token();
	}

	/*virtual*/ AtomicConstruct::~AtomicConstruct()
	{
		delete this->token;
	}

	/*virtual*/ bool AtomicConstruct::Parse(TokenList& tokenList)
	{
		if (tokenList.GetCount() != 1)
			return false;

		TokenList::Node* node = tokenList.GetHead();
		if (node->value.type == Token::NUMBER || node->value.type == Token::STRING || node->value.type == Token::IDENTIFIER)
		{
			*this->token = node->value;
			tokenList.Remove(node);
			return true;
		}

		return false;
	}

	/*virtual*/ void AtomicConstruct::GenerateInstructionSequence(LinkedList<Instruction*>& instructionList)
	{
		// This will typically just be a single push instruction to create a
		// new immediate value on the evaluation stack.  But it could also
		// be a load instruction to get a value from scope and then put that
		// on the evaluation stack.
	}
}