#include <iostream>
#include "VirtualMachine.h"
#include "GarbageCollector.h"

int main(int argc, char** argv)
{
    using namespace Powder;

	GC::GarbageCollector::Set(new GC::GarbageCollector());

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

				Error error;
				if (!vm.ExecuteSourceCode(sourceCode, "", error))
				{
					std::string errorMsg(error);
					std::cerr << errorMsg << std::endl;
				}

				GC::GarbageCollector::Get()->Collect();
			}
		}
		else
		{
			std::string programSourceCodePath = argv[1];

			Error error;
			if (!vm.ExecuteSourceCodeFile(programSourceCodePath, error))
			{
				std::string errorMsg(error);
				std::cerr << errorMsg << std::endl;
			}
		}
	}

	delete GC::GarbageCollector::Get();
	GC::GarbageCollector::Set(nullptr);

    return 0;
}