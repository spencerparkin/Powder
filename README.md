# Powder

Powder is a programming language and a run-time targeted by a compiler for that
language, all written as an exercise in writing such a thing.  Here is some example
code...

```
output("Hello!\n");
fork {
    i = 0;
    while(i < 4) {
        output("fork A: " + i + "\n");
        yield;
        i += 1;
    }
} else {
    j = 0;
    while(j > -4) {
        output("fork B: " + j + "\n");
        yield;
        j -= 1;
    }
};
output("Good-bye!\n");
```

As you might have guessed, the output of this program is as follows.

```
Hello!
fork A: 0
fork B: 0
fork A: 1
fork B: -1
fork A: 2
fork B: -2
fork A: 3
fork B: -3
Good-bye!
Good-bye!
```

The good-bye message is printed twice, because each fork continues to the
end of the program.  This could be prevented with a system call to exit.

Anyhow, this demonstrates the cooperative concurrency built into the language,
which is useful for increasing the efficiency of programs that utilize
non-blocking I/O operations.

The language itself borrows from C, Python, Lua, and Javascript.  Its
rules for placing semi-colons are somewhat different, but possibly more
consistent; at least, from my point of view.

The atomic, non-mutable data types of the language are numbers, strings
and functions.  The composite, mutable data types are lists and maps,
which can nest any type.  Thus, lists of lists of numbers, maps of lists of functions,
maps of maps of strings, etc., are all possible.  The programmer need
not concern themselves with memory management.  The run-time VM uses a
simple form of garbage collection to occationally reclaim abandoned memory.

Presently, the VM is purely stack-based, and has only 13 instructions.
Some are very simple while others, such as the math instruction, which
handles all math operations on any number of values, are heavily overloaded,
but the code remains quite compact.

Ironically, the parser is the most complicated part of the compiler.  The
code that generates the instructions as a function of the AST (abstarct syntax
tree) is quite simple, and fell out naturally while targetting the stack-based VM.
I would have thought the instruction sequence generating code would have been the
most complicated part of compilation, but the parser is far more so.  Powder uses an algorithm
based on applying grammar production rules to a given tokenized sequence
in order to produce the AST.

Powder is easily embeddable and extensible, and I plan to write a simple
debugger for it, because, why not?  As for the name, it is a skiing reference.
The creator of Python was a fan of Monte-Python, and I'm a fan of skiing.