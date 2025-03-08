//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include <cstdint>
#include <llvm-18/llvm/IR/Constant.h>
#include <llvm-18/llvm/IR/Constants.h>
#include <llvm-18/llvm/IR/Instruction.h>
#include <llvm-18/llvm/Support/Casting.h>
// L'include seguente va in LocalOpts.h

using namespace llvm;

bool runOnBasicBlock(BasicBlock &B) {

    // Preleviamo le prime due istruzioni del BB
    Instruction &Inst1st = *B.begin(), &Inst2nd = *(++B.begin());

    // L'indirizzo della prima istruzione deve essere uguale a quello del
    // primo operando della seconda istruzione (per costruzione dell'esempio)
    assert(&Inst1st == Inst2nd.getOperand(0));

    // Stampa la prima istruzione
    outs() << "PRIMA ISTRUZIONE: " << Inst1st << "\n";
    // Stampa la prima istruzione come operando
    outs() << "COME OPERANDO: ";
    Inst1st.printAsOperand(outs(), false);
    outs() << "\n";

    // User-->Use-->Value
    outs() << "I MIEI OPERANDI SONO:\n";
    for (auto *Iter = Inst1st.op_begin(); Iter != Inst1st.op_end(); ++Iter) {
      Value *Operand = *Iter;

      if (Argument *Arg = dyn_cast<Argument>(Operand)) {
        outs() << "\t" << *Arg << ": SONO L'ARGOMENTO N. " << Arg->getArgNo()
	       <<" DELLA FUNZIONE " << Arg->getParent()->getName()
               << "\n";
      }
      if (ConstantInt *C = dyn_cast<ConstantInt>(Operand)) {
        outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE " << C->getValue()
               << "\n";
      }
    }

    outs() << "LA LISTA DEI MIEI USERS:\n";
    for (auto Iter = Inst1st.user_begin(); Iter != Inst1st.user_end(); ++Iter) {
      outs() << "\t" << *(dyn_cast<Instruction>(*Iter)) << "\n";
    }

    outs() << "E DEI MIEI USI (CHE E' LA STESSA):\n";
    for (auto Iter = Inst1st.use_begin(); Iter != Inst1st.use_end(); ++Iter) {
      outs() << "\t" << *(dyn_cast<Instruction>(Iter->getUser())) << "\n";
    }

    // Manipolazione delle istruzioni
    Instruction *NewInst = BinaryOperator::Create(
        Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

    NewInst->insertAfter(&Inst1st);
    // Si possono aggiornare le singole references separatamente?
    // Controlla la documentazione e prova a rispondere.
    Inst1st.replaceAllUsesWith(NewInst);

    return true;
}

bool strenghtReduction(BasicBlock &BB) {
    for (Instruction &Inst : BB) {
        if (Inst.getOpcode() == Instruction::Mul) {
            Value *op0 = Inst.getOperand(0);
            Value *op1 = Inst.getOperand(1);

            bool canBeOptimized = false;
            bool isZero = false;

            int64_t opValue = 0;
            ConstantInt *c;

            if (ConstantInt *CI = dyn_cast<ConstantInt>(op0)) {
                int64_t opValue = CI->getSExtValue();

                if (opValue % 2 == 0) {
                    canBeOptimized = true;
                    isZero = true;
                };

                c = CI;
            } else if (ConstantInt *CI = dyn_cast<ConstantInt>(op1)) {
                int64_t opValue = CI->getSExtValue();

                if (opValue % 2 == 0) canBeOptimized = true;

                c = CI;
            }

            if (canBeOptimized) {
                Instruction *newInst = BinaryOperator::Create(
                    Instruction::Shl, !isZero ? op0 : op1, ConstantInt::get(c->getType(), opValue / 2)
                );

                newInst->insertAfter(&Inst);
                // Si possono aggiornare le singole references separatamente?
                // Controlla la documentazione e prova a rispondere.
                Inst.replaceAllUsesWith(newInst);

            }
        }
    }

    return true;
}

bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (strenghtReduction(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M,
                                      ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getLocalOptsPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LocalOpts", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (Name == "localopts") {
                    MPM.addPass(LocalOpts());
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
  return getLocalOptsPluginInfo();
}
