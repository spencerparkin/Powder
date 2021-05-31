#pragma once

#include "Value.h"
#include <vector>

namespace Powder
{
	class POWDER_API ListValue : public Value
	{
	public:
		ListValue();
		virtual ~ListValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;

		void Clear();

		//void LPush();
		//void LPop();
		//void RPush();
		//void RPop();
		
		Value* GetValueAt(uint64_t i);
		bool SetValueAt(uint64_t i, Value* value);

	private:

		std::vector<Value*>* valueArray;
	};
}