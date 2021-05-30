#pragma once

namespace Powder
{
	template<typename T>
	class LinkedList
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
			Node* node = new Node(value);

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

		void InsertBefore(Node* before, T value)
		{
			Node* node = new Node(value);

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

		void Remove(Node* node)
		{
			if (this->head == node)
				this->head = this->head->next;

			if (this->tail == node)
				this->tail = this->tail->prev;

			node->Decouple();
			delete node;
			this->count--;
		}

		void RemoveAll()
		{
			while (this->count > 0)
				this->Remove(this->head);
		}

		Node* GetHead() { return this->head; }
		Node* GetTail() { return this->tail; }

		const Node* GetHead() const { return this->head; }
		const Node* GetTail() const { return this->tail; }

		unsigned int GetCount() const { return this->count; }

	private:

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
