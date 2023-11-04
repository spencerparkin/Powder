#pragma once

#include "ContainerValue.h"
#include "CppFunctionValue.h"
#include "HashMap.hpp"
#include "Reference.h"

namespace Powder
{
	class ListValue;

	class POWDER_API MapValue : public ContainerValue
	{
	public:
		MapValue();
		virtual ~MapValue();

		virtual Value* Copy() const override;
		virtual Value* CombineWith(const Value* value, MathInstruction::MathOp mathOp, Executor* executor) const override;
		virtual std::string ToString() const override;
		virtual bool SetField(Value* fieldValue, Value* dataValue, Error& error) override;
		virtual Value* GetField(Value* fieldValue, Error& error) override;
		virtual bool DelField(Value* fieldValue, GC::CriticalReference<Value>& valueRef, Error& error) override;
		virtual BooleanValue* IsMember(const Value* value) const override;
		virtual CppFunctionValue* MakeIterator(void) override;
		virtual std::string GetTypeString() const override;
		virtual std::string GetSetKey() const override;

		ListValue* GenerateKeyListValue();

		void SetField(const char* key, Value* dataValue);
		Value* GetField(const char* key);
		bool DelField(const char* key, GC::CriticalReference<Value>& valueRef);

		HashMap<GC::NonCriticalReference<Value>>& GetValueMap() { return this->valueMap; }
		const HashMap<GC::NonCriticalReference<Value>>& GetValueMap() const { return this->valueMap; }

		virtual bool IterationBegin(void*& userData) override;
		virtual Object* IterationNext(void* userData) override;
		virtual void IterationEnd(void* userData) override;

	private:

		// TODO: Maybe replace this with std::map?
		HashMap<GC::NonCriticalReference<Value>> valueMap;
	};

	class POWDER_API MapValueIterator : public CppFunctionValue
	{
	public:
		MapValueIterator(MapValue* mapValue);
		virtual ~MapValueIterator();

		virtual bool Call(ListValue* argListValue, GC::CriticalReference<Value>& returnValueRef, CppCallingContext& context, Error& error) override;

		GC::Reference<MapValue, false> mapValueRef;
		HashMap<GC::NonCriticalReference<Value>>::iterator mapIter;
	};
}