#include <iostream>
#include "GarbageCollector.h"
#include "GCCollectable.h"
#include "GCReference.hpp"
#include "Assembler.h"
#include "ScopeInstruction.h"
#include "PushInstruction.h"
#include "MathInstruction.h"
#include "StoreInstruction.h"
#include "JumpInstruction.h"
#include "SysCallInstruction.h"
#include "VirtualMachine.h"

class Blah : public Powder::GCCollectable
{
public:
    Blah()
    {
        uint64_t address = uint64_t(this);
        std::cout << "Blah constructed!  (" << address << ")" << std::endl;
    }

    virtual ~Blah()
    {
        uint64_t address = uint64_t(this);
        std::cout << "Blah destructed!  (" << address << ")" << std::endl;
    }
};

class Blorp : public Powder::GCCollectable
{
public:
    Blorp()
    {
        this->blorp = nullptr;

        uint64_t address = uint64_t(this);
        std::cout << "Blorp constructed!  (" << address << ")" << std::endl;
    }

    virtual ~Blorp()
    {
        uint64_t address = uint64_t(this);
        std::cout << "Blorp destructed!  (" << address << ")" << std::endl;
    }

    void SetBlorp(Blorp* blorp)
    {
        if (this->blorp)
            this->DisownObject(this->blorp);

        this->blorp = blorp;

        if (this->blorp)
            this->OwnObject(this->blorp);
    }

    Blorp* GetBlorp()
    {
        return this->blorp;
    }

private:

    Blorp* blorp;
};

void GCTest(void)
{
    std::cout << "Begin GC test..." << std::endl;

    {
        {
            Powder::GCReference<Blah> blah1(new Blah());
            Powder::GCReference<Blah> blah2 = blah1;
        }

        Powder::GCReference<Blorp> blorp1(new Blorp());
        Powder::GCReference<Blorp> blorp2(new Blorp());

        blorp1->SetBlorp(blorp2);
        blorp2->SetBlorp(blorp1);

        Powder::GarbageCollector::GC()->FullPass();
    }

    Powder::GarbageCollector::GC()->FullPass();

    std::cout << "End GC test!" << std::endl;
}

void AssemblyTest(void)
{
    Powder::Assembler assembler;

    std::list<Powder::Instruction*> instructionList;
    Powder::AssemblyData::Entry entry;

    // This illustrates the calling convention...

    Powder::Instruction* instruction = new Powder::ScopeInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "push";
    instruction->assemblyData->configMap.Insert("scopeOp", entry);
    instructionList.push_back(instruction);

    Powder::PushInstruction* pushReturnAddressInstruction = new Powder::PushInstruction();
    instruction = pushReturnAddressInstruction;
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "address";
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.push_back(instruction);

    instruction = new Powder::StoreInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "__return_address__";
    instruction->assemblyData->configMap.Insert("name", entry);
    instructionList.push_back(instruction);

    //...now this is where we would load all the arguments to the procedure call if it had any...

    Powder::JumpInstruction* procCallInstruction = new Powder::JumpInstruction();
    instruction = procCallInstruction;
    instruction->assemblyData = new Powder::AssemblyData();
    entry.string = "jump_given_address";
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.push_back(instruction);

    instruction = new Powder::SysCallInstruction();
    instruction->assemblyData = new Powder::AssemblyData();
    entry.string = "halt";
    instruction->assemblyData->configMap.Insert("sys_call", entry);
    instructionList.push_back(instruction);

    entry.instruction = instruction;
    instruction->assemblyData->configMap.Insert("data", entry);
    pushReturnAddressInstruction->assemblyData->configMap.Insert("data", entry);

    //----------- Start of Procedure -----------

    instruction = new Powder::PushInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "string";
    instruction->assemblyData->configMap.Insert("type", entry);
    entry.string = "Hello, ";
    instruction->assemblyData->configMap.Insert("data", entry);
    instructionList.push_back(instruction);

    entry.instruction = instruction;
    procCallInstruction->assemblyData->configMap.Insert("jump", entry);

    instruction = new Powder::PushInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "string";
    instruction->assemblyData->configMap.Insert("type", entry);
    entry.string = "World!";
    instruction->assemblyData->configMap.Insert("data", entry);
    instructionList.push_back(instruction);

    instruction = new Powder::MathInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "add";
    instruction->assemblyData->configMap.Insert("mathOp", entry);
    instructionList.push_back(instruction);

    instruction = new Powder::SysCallInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "output";
    instruction->assemblyData->configMap.Insert("sys-call", entry);
    instructionList.push_back(instruction);

    instruction = new Powder::JumpInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "jump_return_address";
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.push_back(instruction);

    // ------------End of Procesure ------------

    uint64_t programBufferSize = 0L;
    uint8_t* programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);

    Powder::VirtualMachine vm;
    vm.Execute(programBuffer, programBufferSize);

    delete[] programBuffer;

    for (std::list<Powder::Instruction*>::iterator iter = instructionList.begin(); iter != instructionList.end(); iter++)
    {
        instruction = *iter;
        delete instruction;
    }
}

int main()
{
    //GCTest();

    AssemblyTest();

    return 0;
}