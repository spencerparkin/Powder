#include "IOModule.h"
#include "MapValue.h"
#include "OpenFileFunctionValue.h"
#include "CloseFileFunctionValue.h"
#include "ReadLineFunctionValue.h"
#include "WriteLineFunctionValue.h"

__declspec(dllexport) Powder::MapValue* GenerateFunctionMap()
{
	Powder::MapValue* mapValue = new Powder::MapValue();

	mapValue->SetField("open", new OpenFileFunctionValue(), true);
	mapValue->SetField("close", new CloseFileFunctionValue(), true);
	mapValue->SetField("read_line", new ReadLineFunctionValue(), true);
	mapValue->SetField("write_line", new WriteLineFunctionValue(), true);

	return mapValue;
}