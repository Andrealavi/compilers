#ifndef LLVM_TRANSFORMS_TESTPASS_H
#define LLVM_TRANSFORMS_TESTPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {
    class LoopWalk : public PassInfoMixin<LoopWalk> {
        public:
            PreservedAnalyses run(
                Loop &L,
                LoopAnalysisManager &LAM,
                LoopStandardAnalysisResults &LAR,
                LPMUpdater &LU
            );
    };
} // namespace llvm

#endif // LLVM_TRANSFORMS_TESTPASS _H
