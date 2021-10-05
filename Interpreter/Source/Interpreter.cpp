#include <iostream>
#include "VirtualMachine.h"
#include "Exceptions.hpp"
#include "GarbageCollector.h"

int main(int argc, char** argv)
{
    using namespace Powder;

	GarbageCollector::GC()->Startup();

	// Enter scope for the VM so that we can run the GC once the VM goes out of scope.
	// This ensures that the VM has no excuse for leaking anything.
	{
		VirtualMachine vm;

		if (argc < 2)
		{
			while (true)
			{
				std::cout << "Powder: ";
				std::string sourceCode;
				std::getline(std::cin, sourceCode);
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

	GarbageCollector::GC()->Shutdown();

    return 0;
}