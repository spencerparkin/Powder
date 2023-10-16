#pragma once

namespace Powder
{
	class MapValue;
}

#define UTILS_MODULE_API		__declspec(dllexport)

extern "C" UTILS_MODULE_API Powder::MapValue* GeneratefunctionMap();