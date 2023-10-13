#pragma once

#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "Reference.h"

namespace Powder
{
	class SetValueIterator;

	class POWDER_API SetValue : public ContainerValue
	{
		friend SetValueIterator;

	public:
		SetValue();
		virtual ~SetValue();

		// For the VM:
		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual std::string GetTypeString() const override;
		virtual bool AddMember(Value* value, Error& error) override;
		virtual bool RemoveMember(Value* value, Error& error) override;
		virtual BooleanValue* IsMember(const Value* value) const override;
		virtual CppFunctionValue* MakeIterator(void) override;

		// For the GC:
		virtual bool IterationBegin(void*& userData) override;
		virtual Object* IterationNext(void* userData) override;
		virtual void IterationEnd(void* userData) override;

		// Misc:
		void Clear();
		SetValue* CalculateUnionWith(const SetValue* setValue) const;
		SetValue* CalculateIntersectionWith(const SetValue* setValue) const;
		SetValue* CalculateDifferenceWith(const SetValue* setValue) const;
		bool IsEQualTo(const SetValue* setValue) const;

		typedef std::map<Value*, GC::Reference<Value, false>*> Map;

	private:
		
		Map* map;
	};

	class POWDER_API SetValueIterator : public CppFunctionValue
	{
	public:
		SetValueIterator(SetValue* setValue);
		virtual ~SetValueIterator();

		virtual bool Call(ListValue* argListValue, GC::Reference<Value, true>& returnValueRef, Error& error) override;

		GC::Reference<SetValue, false> setValueRef;
		SetValue::Map::iterator* mapIter;
	};
}