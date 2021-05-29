#include <iostream>
#include "GarbageCollector/GarbageCollector.h"
#include "GarbageCollector/GCCollectable.h"
#include "GarbageCollector/GCReference.hpp"

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

int main()
{
    GCTest();

    // TODO: Write test that sends hand-made program to VM and then runs the program.

    return 0;
}