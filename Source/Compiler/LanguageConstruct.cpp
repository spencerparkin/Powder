#include "LanguageConstruct.h"
#include "Tokenizer.h"

namespace Powder
{
	LanguageConstruct::LanguageConstruct()
	{
	}

	/*virtual*/ LanguageConstruct::~LanguageConstruct()
	{
	}

	TokenList::Node* LanguageConstruct::FindCloserMatchingOpener(TokenList::Node* node)
	{
		if (!node || node->value.type != Token::OPENER)
			return nullptr;

		std::string openerText = node->value.text;
		uint32_t count = 0;

		do
		{
			if (node->value.type == Token::OPENER)
				count++;
			else if (node->value.type == Token::CLOSER)
				count--;
			
			if (count == 0)
				break;
			
			node = node->GetNext();
		} while (node);

		if (node)
		{
			std::string closerText = node->value.text;
			if (openerText == "(" && closerText != ")")
				node = nullptr;
			if (openerText == "{" && closerText != "}")
				node = nullptr;
			if (closerText == "[" && closerText != "]")
				node = nullptr;
		}

		return node;
	}

	void LanguageConstruct::ExtractSubList(TokenList& parentList, TokenList& childList, TokenList::Node* leftNode, TokenList::Node* rightNode)
	{
		while (true)
		{
			childList.AddTail(leftNode->value);
			TokenList::Node* nextNode = leftNode->GetNext();
			parentList.Remove(leftNode);
			if (leftNode == rightNode)
				break;
			leftNode = nextNode;
		}
	}
}