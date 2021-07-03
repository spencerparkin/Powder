#include "IOModule.h"
#include "VirtualMachine/Values/MapValue.h"

__declspec(dllexport) Powder::MapValue* GenerateFunctionMap()
{
	Powder::MapValue* mapValue = new Powder::MapValue();

	// TODO: Populate map with CppFunctionValue instances.

	return mapValue;
}