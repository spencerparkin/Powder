# Powder

This is my attempt to impliment a basic programming language featuring a simple syntax,
virtual machine, and a compiler targeting that virtual machine.  Run-time features include
cooperative, single-threaded concurrency, and automatic memory management using garbage
collection.  A major design goal is to try to be as small and simple as possible, not
just in implimentation, but also in the number of VM instructions needed to provide a
reasonably useful and complete programmable machine.  The VM lends itself to
procedural-style programming languages, supports basic system calls for console input
and output, provides list containers, and basic math operations.