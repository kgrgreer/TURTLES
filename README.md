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
1. GC-aware data-structures.
2. Flyweight objects.
3. Extensibility through FOAM-like axioms.

Targets

1. RP2040 microcontroller.
2. The KAOS operating system (which doesn't exist yet).

Higher-Level Languages

Currently, the following higher-level languages are (partially) supported:

1. JS - a subset of a Javascript-like language.
2. SOM - a subset of the Smalltalk language.

The intention is to create T1 as a derivative of SOM and and then T2 as higher-level
axiom-based language on top of T1.

An ARM Cortex M0+ assembler is also planned, as this is the controller used by the RP2040.
