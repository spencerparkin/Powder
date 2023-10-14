#pragma once

namespace Powder
{
	class MapValue;
}

#define MATH_MODULE_API		__declspec(dllexport)

extern "C" MATH_MODULE_API Powder::MapValue* GeneratefunctionMap();