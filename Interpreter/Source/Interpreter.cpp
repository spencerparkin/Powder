#include <iostream>
#include "Compiler.h"
#include "Value.h"
#include "Scope.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"

int main(int argc, char** argv)
{
    using namespace Powder;

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

    return 0;
}