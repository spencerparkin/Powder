#include <iostream>
#include "GarbageCollector.h"
#include "GCCollectable.h"
#include "GCReference.hpp"
#include "Assembler.h"
#include "ScopeInstruction.h"
#include "PushInstruction.h"
#include "MathInstruction.h"
#include "StoreInstruction.h"
#include "LoadInstruction.h"
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
    Powder::LinkedList<Powder::Instruction*> instructionList;
    Powder::AssemblyData::Entry entry;
    Powder::Instruction* instruction = nullptr;

    // We should always start a program with scope.

    instruction = new Powder::ScopeInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::ScopeInstruction::ScopeOp::PUSH;
    instruction->assemblyData->configMap.Insert("scopeOp", entry);
    instructionList.AddTail(instruction);

    // This illustrates the calling convention.  First step is to push new scope.

    instruction = new Powder::ScopeInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::ScopeInstruction::ScopeOp::PUSH;
    instruction->assemblyData->configMap.Insert("scopeOp", entry);
    instructionList.AddTail(instruction);

    // Next we assign arguments and the return address to the scope.  These assignments can be done in any order.
    // Here there are no arguments, so we just store the return address.

    Powder::PushInstruction* pushReturnAddressInstruction = new Powder::PushInstruction();
    instruction = pushReturnAddressInstruction;
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::PushInstruction::DataType::ADDRESS;
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.AddTail(instruction);           // We do not yet have the return address instruction.

    instruction = new Powder::StoreInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "__return_address__";
    instruction->assemblyData->configMap.Insert("name", entry);
    instructionList.AddTail(instruction);

    // Again, here is where we would load any arguments into the scope, if we had any, which we don't.
    // Note that as thing stand now, arguments would always be being passed by reference, which might
    // not be bad.  We could require pass-by-value to be the explicit rather than the implicit case.
    // Okay, so now jump to the start of the procedure.

    Powder::JumpInstruction* procCallInstruction = new Powder::JumpInstruction();
    instruction = procCallInstruction;
    instruction->assemblyData = new Powder::AssemblyData();
    entry.code = Powder::JumpInstruction::JUMP_TO_EMBEDDED_ADDRESS;
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.AddTail(instruction);           // We do not yet know where the start of the procedure is.

    // We'll return from the procedure to just after our jump call to the procedure.
    // The first thing we might do is load the return value of the procedure from the scope,
    // if it is something we care about.  The compiler should always produce such a result,
    // but it won't necessarily be something we load upon return.

    instruction = new Powder::LoadInstruction();
    instruction->assemblyData = new Powder::AssemblyData();
    entry.string = "__return_value__";
    instruction->assemblyData->configMap.Insert("name", entry);
    instructionList.AddTail(instruction);

    // Okay, now we know where the return address instruction is, so patch that now.

    entry.instruction = instruction;
    instruction->assemblyData->configMap.Insert("data", entry);
    pushReturnAddressInstruction->assemblyData->configMap.Insert("data", entry);

    // Next we pop the scope of the function we just returned from.

    instruction = new Powder::ScopeInstruction();
    instruction->assemblyData = new Powder::AssemblyData();
    entry.code = Powder::ScopeInstruction::POP;
    instruction->assemblyData->configMap.Insert("scopeOp", entry);
    instructionList.AddTail(instruction);

    // We need to make a system call here to halt the program, otherwise
    // we would fall right into the start of the procedure.

    instruction = new Powder::SysCallInstruction();
    instruction->assemblyData = new Powder::AssemblyData();
    entry.code = Powder::SysCallInstruction::SysCall::EXIT;
    instruction->assemblyData->configMap.Insert("sysCall", entry);
    instructionList.AddTail(instruction);

    // This is the first instruction of the procedure we want to call.
    // This is where we might start loading our arguments, but since
    // we don't have any, we don't care.

    instruction = new Powder::PushInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::PushInstruction::STRING;
    instruction->assemblyData->configMap.Insert("type", entry);
    entry.string = "Hello, ";
    instruction->assemblyData->configMap.Insert("data", entry);
    instructionList.AddTail(instruction);

    // Now that we have the first instruction, patch the procesure call instruction.

    entry.instruction = instruction;
    procCallInstruction->assemblyData->configMap.Insert("jump", entry);

    instruction = new Powder::PushInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::PushInstruction::STRING;
    instruction->assemblyData->configMap.Insert("type", entry);
    entry.string = "World!";
    instruction->assemblyData->configMap.Insert("data", entry);
    instructionList.AddTail(instruction);

    instruction = new Powder::MathInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::MathInstruction::ADD;
    instruction->assemblyData->configMap.Insert("mathOp", entry);
    instructionList.AddTail(instruction);

    instruction = new Powder::SysCallInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::SysCallInstruction::SysCall::OUTPUT;
    instruction->assemblyData->configMap.Insert("sysCall", entry);
    instructionList.AddTail(instruction);

    // Before jumping to the return address, here we should store our return result.
    // We should always store a return result, even if none is given by the way the
    // procedure was written.

    instruction = new Powder::PushInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::PushInstruction::UNDEFINED;
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.AddTail(instruction);

    instruction = new Powder::StoreInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "__return_value__";
    instruction->assemblyData->configMap.Insert("name", entry);
    instructionList.AddTail(instruction);

    // Lastly, jump to the return address.

    instruction = new Powder::LoadInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.string = "__return_address__";
    instruction->assemblyData->configMap.Insert("name", entry);
    instructionList.AddTail(instruction);

    instruction = new Powder::JumpInstruction();
    instruction->assemblyData = new Powder::AssemblyData;
    entry.code = Powder::JumpInstruction::JUMP_TO_LOADED_ADDRESS;
    instruction->assemblyData->configMap.Insert("type", entry);
    instructionList.AddTail(instruction);

    // And that's the last instruction of the program.

    uint64_t programBufferSize = 0L;
    uint8_t* programBuffer = assembler.AssembleExecutable(instructionList, programBufferSize);

    Powder::VirtualMachine vm;
    vm.Execute(programBuffer, programBufferSize);

    delete[] programBuffer;

    Powder::DeleteList<Powder::Instruction*>(instructionList);
}

int main()
{
    //GCTest();

    AssemblyTest();

    return 0;
}