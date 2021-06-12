#pragma once

#include "Defines.h"

namespace Powder
{
	template<typename T>
	class POWDER_API LinkedList
	{
	public:

		LinkedList()
		{
			this->head = nullptr;
			this->tail = nullptr;
			this->count = 0;
		}

		virtual ~LinkedList()
		{
			this->RemoveAll();
		}

		class Node
		{
			friend class LinkedList;

		public:

			Node(T givenValue)
			{
				this->next = nullptr;
				this->prev = nullptr;
				this->value = givenValue;
			}

			virtual ~Node()
			{
			}

			void Couple()
			{
				if (this->next)
					this->next->prev = this;

				if (this->prev)
					this->prev->next = this;
			}

			void Decouple()
			{
				if (this->next)
					this->next->prev = this->prev;

				if (this->prev)
					this->prev->next = this->next;
			}

			T value;

			Node* GetNext() { return this->next; }
			Node* GetPrev() { return this->prev; }

			const Node* GetNext() const { return this->next; }
			const Node* GetPrev() const { return this->prev; }

		private:
			Node* next;
			Node* prev;
		};

		void AddTail(T value)
		{
			this->InsertAfter(this->tail, value);
		}

		void AddHead(T value)
		{
			this->InsertBefore(this->head, value);
		}

		void InsertAfter(Node* after, T value)
		{
			this->InsertNodeAfter(after, new Node(value));
		}

		void InsertBefore(Node* before, T value)
		{
			this->InsertNodeBefore(before, new Node(value));
		}

		void Remove(Node* node)
		{
			this->RemoveNode(node);
			delete node;
		}

		void RemoveAll()
		{
			while (this->count > 0)
				this->Remove(this->head);
		}

		void Append(const LinkedList<T>& list)
		{
			for (Node* node = list.head; node; node = node->next)
				this->AddTail(node->value);
		}

		void Prepend(const LinkedList<T>& list)
		{
			for (Node* node = list.tail; node; node = node->prev)
				this->AddHead(node->value);
		}

		void BubbleSort(void)
		{
			int swapCount = 0;
			do
			{
				swapCount = 0;
				Node* nodeA = this->head;
				while (nodeA && nodeA->next)
				{
					Node* nodeB = nodeA->next;
					int keyA = nodeA->value.SortKey();
					int keyB = nodeB->value.SortKey();
					if (keyA <= keyB)
						nodeA = nodeB;
					else
					{
						this->RemoveNode(nodeA);
						this->InsertNodeAfter(nodeB, nodeA);
						swapCount++;
					}
				}
			} while (swapCount > 0);
		}

		Node* GetHead() { return this->head; }
		Node* GetTail() { return this->tail; }

		const Node* GetHead() const { return this->head; }
		const Node* GetTail() const { return this->tail; }

		unsigned int GetCount() const { return this->count; }

	private:

		void InsertNodeAfter(Node* after, Node* node)
		{
			if (!after)
				this->head = this->tail = node;
			else
			{
				node->prev = after;
				node->next = after->next;
				node->Couple();
				if (!node->next)
					this->tail = node;
			}

			this->count++;
		}

		void InsertNodeBefore(Node* before, Node* node)
		{
			if (!before)
				this->head = this->tail = node;
			else
			{
				node->next = before;
				node->prev = before->prev;
				node->Couple();
				if (!node->prev)
					this->head = node;
			}

			this->count++;
		}

		void RemoveNode(Node* node)
		{
			if (this->head == node)
				this->head = this->head->next;

			if (this->tail == node)
				this->tail = this->tail->prev;

			node->Decouple();
			this->count--;
		}

		Node* head;
		Node* tail;
		unsigned int count;
	};

	template<typename T>
	void DeleteList(LinkedList<T>& list)
	{
		while (list.GetCount() > 0)
		{
			auto node = list.GetHead();
			T value = node->value;
			delete value;
			list.Remove(node);
		}
	}
}
