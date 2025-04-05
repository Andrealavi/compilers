#include "loopWalk.hpp"

using namespace llvm;

PreservedAnalyses LoopWalk::run(
    Loop &L,
    LoopAnalysisManager &LAM,
    LoopStandardAnalysisResults &LAR,
    LPMUpdater &LU
) {
    if (L.isLoopSimplifyForm()) outs() << "The loop is in simplifed form\n";
    else outs() << "The loop is not in simplified form\n";

    BasicBlock *preHeader = L.getLoopPreheader();
    BasicBlock *header = L.getHeader();

    outs() << "This is the preheader: \n\n";
    preHeader->print(outs());
    outs() << "\n\n";

    outs() << "This is the header: \n\n";
    header->print(outs());
    outs() << "\n\n";

    for (BasicBlock *BB : L.getBlocks()) {
        outs() << "This is the a basic block of the loop: \n\n";
        BB->print(outs());
        outs() << "\n";
    }

    return PreservedAnalyses::all();
};

PassPluginLibraryInfo getLoopWalkPluginInfo() {
return {LLVM_PLUGIN_API_VERSION, "LoopWalk", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, LoopPassManager &LPM,
                    ArrayRef<PassBuilder::PipelineElement>) -> bool {
                if (Name == "loop-walk") {
                    LPM.addPass(LoopWalk());
                    return true;
                }
                return false;
            });
    }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize TestPass when added to the pass pipeline on the
// command line, i.e. via '-passes=test-pass'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getLoopWalkPluginInfo();
}
