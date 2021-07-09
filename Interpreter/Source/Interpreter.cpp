#include <iostream>
#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"
#include "GarbageCollector.h"
#include "Debugger.h"

int main(int argc, char** argv)
{
    using namespace Powder;

	// Enter scope for the VM so that we can run the GC once the VM goes out of scope.
	// This ensures that the VM has no excuse for leaking anything.
	{
		Compiler compiler;
		Debugger debugger(1701);
		VirtualMachine vm(&compiler, &debugger);

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

	// At this point, a full purge should bring us to zero outstanding objects.
	GarbageCollector::GC()->FullPurge();
	uint32_t objectCount = GarbageCollector::GC()->RemainingObjectCount();
	if (objectCount > 0)
	{
		std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "GC system did not purge " << objectCount << " object(s)." << std::endl;
		std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

		GarbageCollector::GC()->FullPurge();
	}

	// Even if the GC system thinks it's done, did it leak anything?
	uint32_t honestCollectableCount = GarbageCollector::GC()->HonestCollectableCount();
	if (honestCollectableCount > 0)
	{
		std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "GC system leaked " << honestCollectableCount << " object(s)." << std::endl;
		std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	}

    return 0;
}