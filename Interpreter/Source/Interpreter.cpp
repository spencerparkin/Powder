#include <iostream>
#include "Compiler.h"
#include "Value.h"
#include "RunTime.h"
#include "GarbageCollector.h"
#include "Exceptions.hpp"
#include "SysCallInstruction.h"

int main(int argc, char** argv)
{
    using namespace Powder;

    if (argc < 2)
    {
        std::cerr << "Please pass in a path to the desired Powder script file." << std::endl;
        return -1;
    }

	std::string programSourceCodePath = SysCallInstruction::ResolveScriptPath(argv[1]);

	try
	{
		Compiler compiler;
		RunTime runTime(&compiler);
		runTime.ExecuteSourceCodeFile(programSourceCodePath);
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