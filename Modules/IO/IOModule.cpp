#include "IOModule.h"
#include "MapValue.h"
#include "OpenFileFunctionValue.h"
#include "CloseFileFunctionValue.h"
#include "ReadLineFunctionValue.h"
#include "WriteLineFunctionValue.h"

__declspec(dllexport) Powder::MapValue* GenerateFunctionMap()
{
	Powder::MapValue* mapValue = new Powder::MapValue();

	mapValue->SetField("open", new OpenFileFunctionValue());
	mapValue->SetField("close", new CloseFileFunctionValue());
	mapValue->SetField("read_line", new ReadLineFunctionValue());
	mapValue->SetField("write_line", new WriteLineFunctionValue());

	return mapValue;
}