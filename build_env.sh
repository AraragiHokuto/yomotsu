#!/bin/sh

#
# --- Configurations ---
# Adjust those according to your setup
#

## Command name of BSD Make
export BSD_MAKE=bmake

## Root of repository
export REPO_ROOT=`git rev-parse --show-toplevel`

## Distributable system root location
export SYSTEM_ROOT=${REPO_ROOT}/proto

## Working dir for build process
export BUILD_DIR=${REPO_ROOT}/build

## Build type: "Debug" or "Release"
export BUILD_TYPE=Debug

## Target architecture
export BUILD_ARCH=amd64

## Toolchain definitions
### LLVM root
export LLVM_ROOT=`realpath ${REPO_ROOT}/../llvm-renzan-root`

### C Compiler
export CC=${LLVM_ROOT}/bin/clang

### Assembler
export AS=${LLVM_ROOT}/bin/clang

### Linker
export LD=${LLVM_ROOT}/bin/ld.lld

