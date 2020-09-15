# ORIHIME

## Current Status: Milestone 1

The goal of milestone 1 is to produce a minimum *Operating System* (rather than a just a naked kernel). 

To be more specifically, there should be:

- Working Kernel (Obviously)

- A Working Shell (oksh or dash)

- Orihime-Specific Cross-Compiling Toolchain (binutils, gcc and/or llvm)

- Libc (enough to support components above)

## Build Instruction

+ Build & install llvm-orihime[AraragiHokuto/llvm-orihime] for cross compilation
+ `make LLVM_PREIFX=your-llvm-prefix`

## How to contribute

This branch serves as a snapshot of latest working version. Please submit contribution to component specific branches to keep a clean commit history.
