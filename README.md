# Powder

*Powder*, the name inspired by my yearly quest for glorious turns, is my attempt to impliment a scripting language that compiles down to instructions interpreted by a stack-based virtual machine.  The project consists of a compiler (lexer, parser and instruction generator) along with a VM targeted by that compiler.  Some sample code follows.

```
greeting_maker = func(name) {
    return func[name]() {
        output("Hello, " + name + "!\n");
    };
};

greeting_func = greeting_maker("world");
greeting_func();
```

As you can see, it looks a lot like C, and that's on purpose.  You'll find many Python, Javascript and Lua influences in it as well, along with a few unique language features.

Powder is a dynamically typed and garbage collected language.  Built-in types include numbers, strings, lists, sets, maps and functions, to name a few.  Types can be added to the language using extension modules.  A primitive IDE and debugger is provided with the whole project.

I have made a You-Tube video series about Powder, which critics are raving about.  People are throwing money at me to see the videos, but I keep trying to remind them that the videos are free.  All proceeds have gone to help fund my mortgage.  The links are as follows.  (You'll laugh, you'll cry.  It's an emotional roller coaster, so get the pop-corn, the tissue-box, and some drugs.  Enjoy!)

* Part 1: [Introduction](https://www.youtube.com/watch?v=awjfhq5j2EI)
* Part 2: [Types](https://www.youtube.com/watch?v=Aj3svD-MCU4)
* Part 3: [Functions](https://www.youtube.com/watch?v=PjEWqetVlVQ)
* Part 4: [Objects](https://www.youtube.com/watch?v=jUgLFbKQV-0)
* Part 5: [Mazes](https://www.youtube.com/watch?v=8jjUrTNph-E&t=51s)
* Part 6: [Extensions](https://www.youtube.com/watch?v=8JldO1F1YA8)
* Part 7: [Comprehensions](https://www.youtube.com/watch?v=duKToQgUcys)
* Part 8: [Forking](https://www.youtube.com/watch?v=pIRvbT4MgnQ)

## Under the Hood

Here I'm going to blab on with some of my thoughts on the implimentation of the project.

### The Parser

Parsing is an interesting problem to me, and one that I'm still learning about all the time.  Rather than cheat and just use *LLVM*, (which is what you should do if you really want to build an industrial-strength programming language), I wanted to write my own parser from scratch.  This means learning something about grammars.  I wrote the whole parser as a separate [project](https://github.com/spencerparkin/ParseParty), which is included with *Powder* as a submodule, and represents a complete re-write of the original parser I came up with.

Given a grammar, it's not hard to randomly generate a sentence from the language of the grammar.  But determining if and how a given sentence belongs to a given grammar, the inverse problem, if you will, is much harder, and it's the very definition of what parsing is.

The parser that I came up with takes a geometric approach, and requires some backtracking.  (I'm not sure how well it scales with the size of the input file, but I also tried to speed things up with a cache to prevent reparsing anything encountered more than once during back-tracking.)  Geometrically, the idea is to "fit" the sentence to various grammar rules, just like how you might write a recursive algorithm to layout a tree of controls in a GUI application framework.  In contrast with an *LL(k)* parser, which only needs to know the first few tokens as it parses, this requires keeping track of the beginning and end of each region to be parsed as the grammar tree is descended.  Undoubtedly, the time-complexity of this parse algorithm won't be as good as *LL(k)*.

There are some match rules in the grammar of *Powder*, however, that must be matched right-to-left, instead of left-to-right, so I'm not yet sure how to use an *LL(k)* parser to compile *Powder*.

### The Garbage Collector

Garbage collection is also an interesting problem, and not a trivial one either.  Again, I wrote *Powder's* GC as a separate [project](https://github.com/spencerparkin/GarbageCollector), included as a submodule, and is yet another re-write of my initial attempt at writing such a thing.

What I did here is start with the simple idea of reference counted objects.  Reference objects (tracked by the GC, but not freed by it) are managed by the host application, and referenced objects (tracked and freed by the GC), also called collectables, are managed by the GC.  As usual, when the last reference to a collectable goes out of scope, the collectable is freed immediately without any GC intervention.  This is nice, because it means that memory is often freed immediately when it's no longer needed.  The role of the GC, then, is simply to solve the problem of when this whole scheme doesn't work, which is simply when cycles are introduced.

My first mistake in trying to write the GC was in thinking that I needed to detect these cycles.  Not so.  The role of the GC is simply to determine if any group of collectables can or can't be traced back to a reference that is purely managed by the host application.  The other class of references are those that are simply owned by collectables.  I refer to these as critical and non-critical references, respectively.  When the GC is run, it builds a graph of all GC objects (references and collectables alike), determines which collectables are no longer visible to the host application, and then simply frees them.  One GC pass is all it takes to clean up everything that can and should be cleaned up.

My GC code asserts if any script exits without everything being cleaned-up as expected, and so far, despite writing many complicated scripts, it's been exiting with an assurance that all memory is accounted for, and Visual Studio hasn't been complaining about leaks either, so I have a fair degree of confidence that *Powder* isn't leaking any memory.

### The Virtual Machine

Again, an industrial-strength language would probably use *LLVM*, which has many back-ends for real machines.  But part of the fun of a project like this is to try to make your *own* VM.  My goal with *Powder* was to make a VM that was as simple as possible, with a minimal instruction set.  What I found is that all I really needed was an eval-stack, and a scope-stack.  I didn't see any need for registered.  (Not yet, anyway.)  Scopes are what they sound like, and store variables in a kind of namespace, if you will, and can be nested in a tree-like fasion, which happens only when you make a function call.  (This departs from a stack, and becomes a tree if you consider multiple threads of execution.)  Thus, functions are the way to control scope, and the scopes help facilitate the "stack frame" and return addresses.  The eval-stack is just for expression evaluation, and it's up to the instruction generator of the compiler to make sure that nothing is ever leaked on the eval-stack.

Interestingly, when I first started this project, I thought that instruction generation as a function of the abstract syntax tree was going to be the hardest part.  Not so!  Parsing turns out to be a far more difficult problem.  Using the AST to generate the code is fairly straight-forward.  Unlike the mighty *LLVM*, however, I'm not doing anything to try to optimize the machine code that is generated.

Machine code, in the case of *Powder*, is just an op-code (a single byte), followed by a variable-length number of bytes understood by the instruction assigned to that op-code.  There's not much more to it than that.  For example, a jump instruction will encode or embed the address of the place (or offset) into the executable where execution should proceed.  Typically, an instruction executes, then the next adjacent instruction follows.  It's whole fetch-execute-cycle.  *Powder's* VM is probably Turing complete, though I don't know how to rigourously or formaly prove such a thing.

The *Powder* VM also has the concept of threads, but they're more like fibers, because they don't run concurrently, just cooperatively.  I'm still playing with this idea and may refactor the code to make it better.  For now, fibers execute using a *FIFO* scheme to determine which fiber hasn't had a turn to run for the longest time.  A better schedular would probably let fibers yeild to specific fibers, facilitate communication between fibers in a way provided by the machine internals, and maybe offer some other forms of synchronization.  Again, since they're just fibers, I don't have to worry about all the dangers of true parallelism, and what consequences that may have on the VM or the GC.  Still, fibers are a neat feature, and can provide some interesting program features, like co-routines or maybe the async/await stuff.  It's all still a work in progress.

### Possible Improvements

For one, I'd like to ditch Visual Studio's build system and use something else, maybe *cmake*.  Property pages are a drag.  Many improvements could be made to the IDE and debugger.  The VM is also quite slow, maybe because of all the reference counting of the GC, and the pedantic error checking all over the VM.  Perhaps all error checking should just go away in a release build.  One improvement I already made was to completely get rid of all exception throwing.  When an exception is thrown, all bets are off, and memory is leaked.  That is no longer the case.  Every internal error, whether a problem with the VM or the script, is handled cleanly and everything exits as gracefully as possible.

## Conclusion

This is just a fun side-project, driven by a desire to expand my own horizons, and to keep my sanity.  You see, when I'm not doing a side-project, I'm usually wading through horribly written legacy code, abandoned by those that came before me at my job.  Sometimes it's nice to take a break and work something that you can take a little bit of pride in.  I'm not under any delusions, of course, that *Powder* is anything useful or good.  It's probably a piece of crap.  But it's my piece of crap, and I enjoy tinkering with it and trying to make it better.