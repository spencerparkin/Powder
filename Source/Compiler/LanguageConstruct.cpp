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

	LanguageConstruct* LanguageConstruct::TryParseWithConstructList(TokenList& tokenList, LinkedList<LanguageConstruct*>& constructList)
	{
		while (constructList.GetHead() > 0)
		{
			LinkedList<LanguageConstruct*>::Node* node = constructList.GetHead();
			LanguageConstruct* languageConstruct = node->value;
			constructList.Remove(node);

			if (!languageConstruct->Parse(tokenList))
				delete languageConstruct;
			else
			{
				DeleteList<LanguageConstruct*>(constructList);
				return languageConstruct;
			}
		}

		return nullptr;
	}

	TokenList::Node* LanguageConstruct::FindRootLevelToken(TokenList& tokenList, const std::string& text, SearchDirection searchDirection)
	{
		uint32_t level = 0;

		TokenList::Node* node = nullptr;
		if (searchDirection == SearchDirection::RIGHTWARD)
			node = tokenList.GetHead();
		else if (searchDirection == SearchDirection::LEFTWARD)
			node = tokenList.GetTail();

		while (node)
		{
			node->value.AdjustLevel(level);

			if(level == 0 && node->value.text == text)
				return node;

			if (searchDirection == SearchDirection::RIGHTWARD)
				node = node->GetNext();
			else if (searchDirection == SearchDirection::LEFTWARD)
				node = node->GetPrev();
		}

		return node;
	}

	TokenList::Node* LanguageConstruct::FindCloserMatchingOpener(TokenList::Node* node)
	{
		if (!node || node->value.type != Token::OPENER)
			return nullptr;

		std::string openerText = node->value.text;
		uint32_t level = 0;

		do
		{
			node->value.AdjustLevel(level);
			if (level == 0)
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

	void LanguageConstruct::ExtractSubList(TokenList& parentList, TokenList& childList, TokenList::Node* leftNode, TokenList::Node* rightNode, bool trimBothEnds)
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

		if (trimBothEnds)
		{
			if (childList.GetCount() > 0)
				childList.Remove(childList.GetHead());

			if (childList.GetCount() > 0)
				childList.Remove(childList.GetTail());
		}
	}
}