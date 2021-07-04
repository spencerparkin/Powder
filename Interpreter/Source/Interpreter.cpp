#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include "Compiler.h"
#include "VirtualMachine.h"
#include "Exceptions.hpp"

int main(int argc, char** argv)
{
    using namespace Powder;

    if (argc < 2)
    {
        std::cerr << "Please pass in a fully qualified path to the desired Powder script file." << std::endl;
        return -1;
    }

    std::string powFilePath = argv[1];
    std::fstream fileStream;
    fileStream.open(powFilePath, std::fstream::in);
    if (!fileStream.is_open())
    {
        std::cerr << "Failed to open file: " + powFilePath << std::endl;
        return -1;
    }

    std::stringstream stringStream;
    stringStream << fileStream.rdbuf();
    std::string powFileCode = stringStream.str();
    fileStream.close();

    uint64_t programBufferSize = 0;
    uint8_t* programBuffer = nullptr;

    try
    {
        const char* programCode = powFileCode.c_str();
        Compiler compiler;
        programBuffer = compiler.CompileCode(programCode, programBufferSize);
        if (programBuffer)
        {
            VirtualMachine vm;
            vm.Execute(programBuffer, programBufferSize);
        }
    }
    catch (Exception* exc)
    {
        std::string errorMsg = exc->GetErrorMessage();
        std::cerr << errorMsg << std::endl;
        delete exc;
    }

    delete[] programBuffer;

    return 0;
}