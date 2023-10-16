#include "UtilsModule.h"
#include "SortFunctionValue.h"
#include "MapValue.h"

Powder::MapValue* GenerateFunctionMap()
{
	Powder::MapValue* mapValue = new Powder::MapValue();

	mapValue->SetField("sort", new SortFunctionValue());

	return mapValue;
}