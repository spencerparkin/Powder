#pragma once

#include "GCCollectable.h"
#include "MathInstruction.h"
#include <string>

namespace Powder
{
	class Executor;

	// These are the fundamental units of data processed by the machine.
	// A value may refer to a mutable or immutable type of data.
	class POWDER_API Value : public GCCollectable
	{
	public:

		Value();
		virtual ~Value();

		virtual Value* Copy() const;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const;
		virtual std::string ToString() const;
		virtual bool FromString(const std::string& str);
		virtual bool AsBoolean() const;
	};
}