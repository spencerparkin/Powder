#pragma once

#include "Defines.h"
#include <vector>
#include <string>
#include "HashMap.hpp"

namespace Powder
{
	class Value;
	class ExtensionFunction;

	// An extension module is a DLL exposing a function named "RegisterExtensionFunctions"
	// and having the following signature.  It should add pointers to derivatives of the
	// ExtensionFunction class (see below) to the given map.  Powder does not take ownership
	// of the memory for such instances.
	typedef void (*RegisterExtensionFunctionsProc)(HashMap<ExtensionFunction*>&);

	// No interpreted language is truely useful unless it can be extended
	// with user modules that expose functionality in native libraries or
	// create bindings for APIs not native to the scripting language.
	// It's laughable to think Powder will ever be useful to anyone, but as
	// a matter of completeness, some kind of mechanism to extend the language
	// should be provided.  It is, of course, easy to embed Powder in an existing
	// application, and the interperter is the prime example of that.
	class ExtensionFunction
	{
	public:
		ExtensionFunction()
		{
		}

		virtual ~ExtensionFunction()
		{
		}

		virtual Value* Call(const std::vector<const Value*>& argValuesArray, std::string& errorMsg) = 0;
	};
}