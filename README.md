# TURTLES
TURTLES Ultra-Recursive Tool for Language EnvironmentS

An experimental language creation system.

![Turtles](turtles.png)

T0

T0 is the lowest-level language in TURTLES, and is meant to act like a VM for higher-level languages. Like Java's JVM and the Forth programming language, it is stack based.
It is lexically scoped and supports closures.

T0 is currently written in Javascript, as this provides an easy development environment,
but the intent is to also have a C version.

Goals

TURTLES is meant to allow for experimentation in the following areas:
1. GC-aware data-structures. [Copying Garbage Collectors](https://en.wikipedia.org/wiki/Cheney%27s_algorithm#:~:text=Garbage%20collection%20is%20performed%20by,previous%20stop%20and%20copy%20technique.) work by dividing memory into two pools and when one becomes full, then copying all live objects to the other pool and then freeing the whole original pool. But if you're going to copy something like a binary-tree, why would you just blindly copy it as is, when with similar effort, you could instead create a perfectly balanced version in the new pool? Similarly, hashtables, vectors and caches could all be resized to more desirable sizes when copied.
2. Flyweight objects. By separating an object's class from its data (the flyweight pattern), you could remove some of the size and performance overhead often caused by OO languages. For example, if you wanted to create a homogeneous Array of Integer objects, the array could store the class only once, rather than for each instance, and the resulting array would be much smaller. Similarly, when classes are known in methods, the class handling code could be partial-eval-ed away, resulting in procedural code with no dynamic dispatch overhead.  
3. Extensibility through FOAM-like axioms.

Targets

1. RP2040 microcontroller.
2. The KAOS operating system (which doesn't exist yet).

Higher-Level Languages

Currently, the following higher-level languages are (partially) supported:

1. JS - a subset of a Javascript-like language.
2. [SOM](http://som-st.github.io/) - a subset of the Smalltalk language.

The intention is to create T1 as a derivative of SOM and and then T2 as higher-level
axiom-based language on top of T1 (maybe to be named "Axiom").

An ARM Cortex M0+ assembler is also planned, as this is the controller used by the RP2040.
