#pragma once

#include "Defines.h"
#include "HashMap.hpp"

namespace Powder
{
	class Value;
	class ListValue;
	class ExtensionModule;
	class VirtualMachine;

	typedef ExtensionModule* (*RegisterExtensionModuleProc)(void);

	class POWDER_API ExtensionModule
	{
	public:
		ExtensionModule();
		virtual ~ExtensionModule();

		class Function
		{
		public:
			Function();
			virtual ~Function();

			// This method will run in a different DLL, and will be expected
			// to allocate its own values, all of which will get freed, eventually,
			// by the GC of this DLL.  Are we in danger of cross-heap contamination?
			// If so, we'll need to expose to extensions some form of value factory.
			virtual Value* Call(const ListValue& argListValue) = 0;
		};

		virtual void RegisterFunctions(HashMap<Function*>& functionMap) = 0;
	};
}