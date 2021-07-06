#include <iostream>
#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"
#include "GarbageCollector.h"

int main(int argc, char** argv)
{
    using namespace Powder;

	// Enter scope for the VM so that we can run the GC once the VM goes out of scope.
	{
		Compiler compiler;
		VirtualMachine vm(&compiler);

		if (argc < 2)
		{
			while (true)
			{
				std::cout << "Powder: ";
				std::string sourceCode;
				std::cin >> sourceCode;
				if (sourceCode == "exit")
					break;

				try
				{
					vm.ExecuteSourceCode(sourceCode, "");
				}
				catch (Exception* exc)
				{
					std::string errorMsg = exc->GetErrorMessage();
					std::cerr << errorMsg << std::endl;
					delete exc;
				}
			}
		}
		else
		{
			std::string programSourceCodePath = argv[1];

			try
			{
				vm.ExecuteSourceCodeFile(programSourceCodePath);
			}
			catch (Exception* exc)
			{
				std::string errorMsg = exc->GetErrorMessage();
				std::cerr << errorMsg << std::endl;
				delete exc;
			}
		}
	}

	GarbageCollector::GC()->FullPass();

    return 0;
}