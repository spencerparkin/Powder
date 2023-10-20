#include "MathModule.h"
#include "MapValue.h"
#include "NewComplexValueFunction.h"

__declspec(dllexport) Powder::MapValue* GenerateFunctionMap()
{
	Powder::MapValue* mapValue = new Powder::MapValue();

	mapValue->SetField("complex", new NewComplexValueFunction());

	return mapValue;
}