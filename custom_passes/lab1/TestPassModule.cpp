//=============================================================================
// FILE:
//    TestPassModule.cpp
//
// DESCRIPTION:
//    Visits all functions in a module and prints their names. Strictly speaking,
//    this is an analysis pass (i.e. //    the functions are not modified). However,
//    in order to keep things simple there's no 'print' method here (every analysis
//    pass should implement it).
//
// USAGE:
//    New PM
//      opt -load-pass-plugin=<path-to>libTestPassModule.so -passes="test-pass" `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// TestPassModule implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {


// New PM implementation
struct TestPassModule: PassInfoMixin<TestPassModule> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {

  	errs() << "Module name: " << M.getName() << "\n";
    errs() << "Number of Functions: " << M.size() << "\n";

    int bbCount = 0;
    for (Function &F: M) {
        bbCount += F.size();
    }

    errs() << "Number of Basic Blocks: " << bbCount << "\n";

    int counter = 0;
    for (Function &F : M) {
        for (BasicBlock &BB: F) {
            for (Instruction &I: BB) {
                counter++;
            }
        }
    }

    errs() << "Instruction Number: " << counter << "\n";

  	return PreservedAnalyses::all();
}


  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getTestPassModulePluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "TestPassModule", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "test-pass-module") {
                    MPM.addPass(TestPassModule());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize TestPassModule when added to the pass pipeline on the
// command line, i.e. via '-passes=test-pass-module'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getTestPassModulePluginInfo();
}
