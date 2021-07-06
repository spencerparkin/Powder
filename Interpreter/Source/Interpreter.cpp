#include <iostream>
#include "Compiler.h"
#include "Value.h"
#include "RunTime.h"
#include "Exceptions.hpp"

int main(int argc, char** argv)
{
    using namespace Powder;

	Compiler compiler;
	RunTime runTime(&compiler);

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
				runTime.ExecuteSourceCode(sourceCode, "");
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
			runTime.ExecuteSourceCodeFile(programSourceCodePath);
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