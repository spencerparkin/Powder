#include <iostream>
#include "Compiler.h"
#include "RunTime.h"
#include "GarbageCollector.h"
#include "Exceptions.hpp"

int main(int argc, char** argv)
{
    using namespace Powder;

    if (argc < 2)
    {
        std::cerr << "Please pass in a path to the desired Powder script file." << std::endl;
        return -1;
    }

    std::string programSourceCodePath = argv[1];

	try
	{
		Compiler compiler;
		RunTime runTime(&compiler);
		runTime.ExecuteSourceCode(programSourceCodePath);
		GarbageCollector::GC()->FullPass();
	}
	catch (Exception* exc)
	{
		std::string errorMsg = exc->GetErrorMessage();
		std::cerr << errorMsg << std::endl;
		delete exc;
	}

    return 0;
}