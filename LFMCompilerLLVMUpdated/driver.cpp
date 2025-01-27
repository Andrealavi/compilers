#include <cmath>
#include <csignal>
#include <cstdlib>
#include "driver.hpp"
#include "parser.hpp"

// Generation of an instance for each of the LLVMContext,
// Module and IRBuilder classes. In the case of a single module, this is sufficient
LLVMContext *context = new LLVMContext;
Module *module = new Module("LFMCompiler", *context);
IRBuilder<> *builder = new IRBuilder(*context);

/************************ Utility Functions ***************************/
Value *LogErrorV(const std::string Str) {
    std::cerr << Str << std::endl;
    return nullptr;
}

static AllocaInst *MakeAlloca(Function *fun, StringRef ide, Type* T = IntegerType::get(*context,32)) {
    /*
        This C++ function is not simple to understand at first.
        It defines a utility with two parameters:
        1) the representation of an LLVM IR function, and
        2) the name for an SSA register
        The call to this utility returns an IR instruction that allocates an i32 (only allowed type) in
        memory (stack) and stores its pointer in an SSA register which is given
        the name passed as the second parameter. The instruction will be written at the beginning
        of the entry block of the function passed as the first parameter.
        Remember that instructions are generated by a builder. To avoid
        interfering with the global builder, the generation is therefore performed
        with a temporary builder TmpB
    */
    IRBuilder<> TmpB(&fun->getEntryBlock(), fun->getEntryBlock().begin());
    return TmpB.CreateAlloca(T, nullptr, ide);
}

/************ Implementation of driver class methods ************/
driver::driver(): trace_parsing(false), trace_scanning(false),
	              toLatex(false), opening("["), closing("]") {};

int driver::parse (const std::string &f)
{
    file = f;
    location.initialize(&file);

    scan_begin();

    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();

    scan_end();

    return res;
}

void driver::codegen() {
    // The codegen method performs a "simple" call to the
    // homonymous method present in the root node generated by the parser.
    fprintf(stderr, "target triple = \"x86_64-pc-linux-gnu\"\n\n");

    for (DefAST* tree: root) {
        tree->codegen(*this);
    }
};

extern driver drv;

/************* Implementation of AST class methods *************/

/// NumberExprAST
NumberExprAST::NumberExprAST(int Val): Val(Val) {};
void NumberExprAST::visit() {
    *drv.outputTarget << "[" << Val << "]";
};

lexval NumberExprAST::getLexVal() const {
    lexval lval = Val;
    return lval;
};

Constant *NumberExprAST::codegen(driver& drv) {
    return ConstantInt::get(*context, APInt(32,Val));
};

/// BoolConstAST
/*
    Differs from NumberExprAST because it's limited to constants 0 and 1
    whose representation is an int1
*/
BoolConstAST::BoolConstAST(int Val): boolVal(Val) {
    if (Val != 0) boolVal = 1;
};

void BoolConstAST::visit() {
    *drv.outputTarget << "[" << boolVal << "]";
};

lexval BoolConstAST::getLexVal() const {
    lexval lval = boolVal;
    return lval;
};

Constant *BoolConstAST::codegen(driver& drv) {
    return ConstantInt::get(*context, APInt(1,boolVal));
};

/// IdeExprAST
IdeExprAST::IdeExprAST(std::string &Name): Name(Name) {};

lexval IdeExprAST::getLexVal() const {
    lexval lval = Name;
    return lval;
};

void IdeExprAST::visit() {
    *drv.outputTarget << drv.opening << Name << drv.closing;
};

Value *IdeExprAST::codegen(driver& drv) {
    // The value is loaded from memory since it is an identifier expression (e.g. x)

    AllocaInst *L = drv.NamedValues[Name];

    if (L) {
        Value *V = builder->CreateLoad(Type::getInt32Ty(*context),
                                        L, Name);
        return V;
    } else {
        GlobalVariable* G = module->getNamedGlobal(Name);

        if (G) {
            return builder->CreateLoad(G->getValueType(), G, Name);
        }
    }

    return LogErrorV("Variable "+Name+" not defined");
};

/// AssignmentExprAST
AssignmentExprAST::AssignmentExprAST(std::pair<std::string, ExprAST*> binding)
    : binding(binding) {};

lexval AssignmentExprAST::getLexVal() const {
    lexval lval = binding.first;
    return lval;
};

void AssignmentExprAST::visit() {
    *drv.outputTarget << "[= " << drv.opening << binding.first << drv.closing;
    binding.second->visit();
    *drv.outputTarget << "]";
};

Value *AssignmentExprAST::codegen(driver& drv) {
    AllocaInst *BInst;
    std::map<std::string,AllocaInst*>::iterator it;

    it = drv.NamedValues.find(binding.first);


    if (it != drv.NamedValues.end()) {
        BInst = drv.NamedValues[binding.first];
    } else {
        Function *function = builder->GetInsertBlock()->getParent();

        BInst = MakeAlloca(function, binding.first);
        drv.NamedValues[binding.first] = BInst;
    }

    Value *boundval = binding.second->codegen(drv);

    builder->CreateStore(boundval, BInst);

    return boundval;
};

/// RetExprAST
RetExprAST::RetExprAST(ExprAST* returnExpr) : returnExpr(returnExpr) {};

void RetExprAST::visit() {
    *drv.outputTarget << "[return ";

    returnExpr->visit();

    *drv.outputTarget << "]";
};

Value* RetExprAST::codegen(driver& drv) {
    Value* returnValue = returnExpr->codegen(drv);

    return builder->CreateRet(returnValue);
};


/// BinaryExprAST
BinaryExprAST::BinaryExprAST(std::string Op, ExprAST* LHS, ExprAST* RHS):
	                         Op(Op), LHS(LHS), RHS(RHS) {};

void BinaryExprAST::visit() {
    *drv.outputTarget << drv.opening << Op;

    if (drv.toLatex) {
        *drv.outputTarget << "$ ";
    } else{
        *drv.outputTarget << " ";
    }

    LHS->visit();
    RHS->visit();
    *drv.outputTarget << "]";
};

Value *BinaryExprAST::codegen(driver& drv) {
    /*
        This is perhaps the simplest code to understand.
        The code for the LHS and RHS of the operator is recursively generated
        and then the code for the specified operation is generated.
    */
    Value *L = LHS->codegen(drv);
    Value *R = RHS->codegen(drv);

    if (!L || !R) {
        return nullptr;
    }

    // The third argument is the name that the operation will have in the generated IR
    if (Op=="+") return builder->CreateNSWAdd(L,R,"sum");
    else if (Op=="-") return builder->CreateNSWSub(L,R,"diff");
    else if (Op=="*") return builder->CreateNSWMul(L,R,"prod");
    else if (Op=="/") return builder->CreateSDiv(L,R,"quot");
    else if (Op=="%") return builder->CreateSRem(L,R,"rem");
    else if (Op=="<") return builder->CreateICmpSLT(L,R,"cmplt");
    else if (Op=="<=") return builder->CreateICmpSLE(L,R,"cmple");
    else if (Op==">") return builder->CreateICmpSGT(L,R,"cmpgt");
    else if (Op==">=") return builder->CreateICmpSGE(L,R,"cmpge");
    else if (Op=="==") return builder->CreateICmpEQ(L,R,"cmpeq");
    else if (Op=="<>") return builder->CreateICmpNE(L,R,"cmpne");
    else if (Op=="and") return builder->CreateAnd(L,R,"and");
    else if (Op=="or") return builder->CreateOr(L,R,"or");
    else {
        return LogErrorV("Binary operator "+Op+" not supported");
    }
};

/// ExponentiationExprAST
ExponentiationExprAST::ExponentiationExprAST(ExprAST* Base, ExprAST* Exponent) : Base(Base), Exponent(Exponent) {};

void ExponentiationExprAST::visit() {
    *drv.outputTarget << drv.opening << "^";

    if (drv.toLatex) {
        *drv.outputTarget << "$ ";
    } else{
        *drv.outputTarget << " ";
    }

    Base->visit();
    Exponent->visit();
    *drv.outputTarget << "]";
};

Value *ExponentiationExprAST::codegen(driver& drv) {
    Value *B = Base->codegen(drv);
    Value *E = Exponent->codegen(drv);

    if (!B || !E) {
        return nullptr;
    }

    Value *res = ConstantInt::get(*context, APInt(32,1));

    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *loopBlock = BasicBlock::Create(*context, "loop", function);
    BasicBlock *afterBlock = BasicBlock::Create(*context, "after", function);

    AllocaInst *BaseInst = MakeAlloca(function, "base");
    AllocaInst *ExpInst = MakeAlloca(function, "exp");
    AllocaInst *ResInst = MakeAlloca(function, "res");

    builder->CreateStore(B, BaseInst);
    builder->CreateStore(E, ExpInst);
    builder->CreateStore(res, ResInst);

    Value *loadedExp = builder->CreateLoad(ExpInst->getAllocatedType(), ExpInst, "exp");

    Value *initialCondition = builder->CreateICmpSGT(loadedExp, ConstantInt::get(*context, APInt(32,0)), "cmpgt");

    builder->CreateCondBr(initialCondition, loopBlock, afterBlock);

    builder->SetInsertPoint(loopBlock);

    B = builder->CreateLoad(BaseInst->getAllocatedType(), BaseInst, "base");
    res = builder->CreateLoad(ResInst->getAllocatedType(), ResInst, "res");
    res = builder->CreateNSWMul(res, B, "prod");
    builder->CreateStore(res, ResInst);

    E = builder->CreateLoad(ExpInst->getAllocatedType(), ExpInst, "exp");
    E = builder->CreateNSWSub(E, ConstantInt::get(*context, APInt(32,1)), "diff");
    builder->CreateStore(E, ExpInst);

    loadedExp = builder->CreateLoad(ExpInst->getAllocatedType(), ExpInst, "exp");

    Value *loopCondition = builder->CreateICmpSGT(loadedExp, ConstantInt::get(*context, APInt(32,0)), "cmpgt");
    builder->CreateCondBr(loopCondition, loopBlock, afterBlock);

    builder->SetInsertPoint(afterBlock);

    return res;
};

/// UnaryExprAST
UnaryExprAST::UnaryExprAST(std::string Op, ExprAST* RHS): Op(Op), RHS(RHS) {};

void UnaryExprAST::visit() {
    *drv.outputTarget << drv.opening << Op;

    if (drv.toLatex) {
        *drv.outputTarget << "$ ";
    } else{
        *drv.outputTarget << " ";
    }

    RHS->visit();
    *drv.outputTarget << "]";
};

Value *UnaryExprAST::codegen(driver& drv) {
    /*
        The only operand is stored in RHS. The unary minus
        is "translated" as subtraction where the minuend is 0.
        The logical not is translated as XOR with the logical constant 1
    */
    if (Op=="-") {
        Value *L = ConstantInt::get(*context, APInt(32,0));
        Value *R = RHS->codegen(drv);
        return builder->CreateNSWSub(L,R,"diff");
    } else if (Op=="not") {
        Value *L = ConstantInt::get(*context, APInt(1,1));
        Value *R = RHS->codegen(drv);
        return builder->CreateXor(L,R,"not");
    } else {
        return LogErrorV("Unary operator "+Op+" not supported");
    }
};

/// CallExprAST
CallExprAST::CallExprAST(std::string Callee, std::vector<ExprAST*> Args): Callee(Callee),
	                     Args(std::move(Args)) {};

lexval CallExprAST::getLexVal() const {
    lexval lval = Callee;
    return lval;
};

void CallExprAST::addArg(ExprAST* arg) {
    Args.insert(Args.begin(), arg);
};

void CallExprAST::visit() {
    *drv.outputTarget << drv.opening << Callee;

    if (drv.toLatex) {
        *drv.outputTarget << "$ ";
    } else {
        *drv.outputTarget << " ";
    }

    for (ExprAST* arg : Args) {
        arg->visit();
    };

    *drv.outputTarget << "]";
};

Value *CallExprAST::codegen(driver& drv) {
    // The generation of code corresponding to a function call
    // begins by searching in the current module (the only one, in our case) for a function
    // whose name matches the name stored in the AST node
    // If the function is not found (and therefore has not been previously defined)
    // an error is generated. Semantic check!
    Function *CalleeF = module->getFunction(Callee);

    if (!CalleeF) {
        return LogErrorV("Function not defined");
    }

    // The second semantic check is that the retrieved function has
    // as many parameters as there are arguments provided in the AST node
    if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect number of arguments");

    // Having passed the second check successfully, the recursive
    // evaluation of the arguments present in the call is prepared
    // (remember that arguments can be arbitrary expressions)
    // The results of the argument evaluations (SSA registers, as always)
    // are inserted into a vector, where the CreateCall method of the builder
    // expects them, which is called immediately after to generate the IR call instruction
    std::vector<Value*> ArgsV;

    for (auto arg : Args) {
        ArgsV.push_back(arg->codegen(drv));
    }

    return builder->CreateCall(CalleeF, ArgsV, "callfun");
};

/// PipExprAST
PipExprAST::PipExprAST(std::vector<ExprAST*> Calls) : Calls(Calls) {};

void PipExprAST::visit() {
    *drv.outputTarget << "[pipe ";

    for (ExprAST* call : Calls) {
        call->visit();
    }

    *drv.outputTarget << "]";
};

Value* PipExprAST::codegen(driver& drv) {
    for (std::vector<ExprAST*>::iterator it = std::next(Calls.begin()); it != Calls.end() ; it++) {
        CallExprAST* call = dynamic_cast<CallExprAST*>(*it);

        if (!call) {
            return LogErrorV("Pipeline operator requires function calls");
        }

        call->addArg(*(it - 1));
    }

    return Calls.back()->codegen(drv);
};

/// IfExprAST
IfExprAST::IfExprAST(std::vector<std::pair<ExprAST*, std::vector<ExprAST*>>> IfThenSeq):
    IfThenSeq(std::move(IfThenSeq)) {};

void IfExprAST::visit() {
    *drv.outputTarget << "[if ";

    for (unsigned i=0, e=IfThenSeq.size(); i<e; i++) {
        *drv.outputTarget << "[alt ";
        IfThenSeq[i].first->visit();

        for (ExprAST* expr: IfThenSeq[i].second) {
            expr->visit();
        }

        *drv.outputTarget << "]";
    }

    *drv.outputTarget << "]";
};

Value *IfExprAST::codegen(driver& drv) {
    /*
        This is the most complex code, partly due to the design choice
        of providing the instruction with an arbitrary number of alternatives.
        Clearly, therefore, a significant part of the code generation
        will have to be executed within a loop.
        Before entering the loop, we generate, in the "current block", the code
        for the first test and create the block with the corresponding expression.
        After the loop ends, we will still need to generate the default alternative (executed
        only if the programmer specifies non-exhaustive alternatives). Inside
        the loop we must therefore create (n-1) pairs (if n is the number of alternatives)
        test/expression: T2/E2,T3/E3,...,Tn/En.
        The writing of blocks (rather than creation) follows a slightly
        different order. Note that to complete the writing of
        test block Ti, you need to have "created" both block Ei and block
        Ti+1 (which are the two possible destinations of the conditional jump that
        closes block Ti. However, if pair Ti+1/Ei+1 is generated in iteration
        (i+1), it is evident that at the end of iteration i this jump
        cannot yet be generated and therefore the builder is still positioned on
        block Ti and (thus, still) the writing of block Ei has not even
        started.
        It follows that in cycle (i+1):
        1) the writing of block Ti is completed.
        2) all of block Ei is written
        3) all of block Ti+1 is written except for the final branch
    */
    int numpairs = IfThenSeq.size();
    Value* CondV = IfThenSeq.at(0).first->codegen(drv); //First test

    if (!CondV){
        return nullptr;
    }

    Function *function =
        builder->GetInsertBlock()->getParent();
    BasicBlock *ExprBB =
            BasicBlock::Create(*context,"expr1");
    BasicBlock *ExitBB =
            BasicBlock::Create(*context,"exitblock");
    BasicBlock *CondBB;  // Defined (but not yet created) here
                        // only for visibility reasons (scope)
                        //

    std::vector<std::pair<Value*,BasicBlock*>> phipairs;

    for (int j=1; j<numpairs; j++) {
        CondBB = BasicBlock::Create(*context,
                            "test"+std::to_string(j+1));
        builder->CreateCondBr(CondV, ExprBB, CondBB);
        function->insert(function->end(), ExprBB);
        builder->SetInsertPoint(ExprBB);

        Value *ExprV;

        bool hasReturn = false;

        for (ExprAST* expr : IfThenSeq.at(j-1).second) {
            Value *exprVal = expr->codegen(drv);

            if (dynamic_cast<RetExprAST*>(expr)) {
                hasReturn = true;
            }

            if (!exprVal) {
                return nullptr;
            }

            if (expr == IfThenSeq.at(j-1).second.back()) {
                ExprV = exprVal;
            }
        }

        if (!hasReturn) {
            builder->CreateBr(ExitBB);
        }

        ExprBB = builder->GetInsertBlock(); //Because it might have
                                            //changed

        if (!hasReturn) {
            phipairs.insert(phipairs.end(),{ExprV,ExprBB});
        }

        function->insert(function->end(), CondBB);
        builder->SetInsertPoint(CondBB);

        CondV = IfThenSeq.at(j).first->codegen(drv);

        if (!CondV) {
            return nullptr;
        }

        ExprBB = BasicBlock::Create(*context,
                                "expr"+std::to_string(j+1));
    }

    builder->CreateCondBr(CondV, ExprBB, ExitBB);
    function->insert(function->end(), ExprBB);
    builder->SetInsertPoint(ExprBB);

    Value *ExprV;

    bool hasReturn = false;

    for (ExprAST* expr : IfThenSeq.at(numpairs-1).second) {
        Value *exprVal = expr->codegen(drv);

        if (!ExprV) {
            return nullptr;
        }

        if (dynamic_cast<RetExprAST*>(expr)) {
            hasReturn = true;
        }

        if (expr == IfThenSeq.at(numpairs-1).second.back()) {
            ExprV = exprVal;
        }
    }

    if (!hasReturn) {
        builder->CreateBr(ExitBB);
    }

    ExprBB = builder->GetInsertBlock();

    if (!hasReturn) {
        phipairs.insert(phipairs.end(),{ExprV,ExprBB});
    }

    function->insert(function->end(),ExitBB);
    builder->SetInsertPoint(ExitBB);

    PHINode *PN = builder->CreatePHI(Type::getInt32Ty(*context),
                            numpairs, "condval");

    for (std::vector<std::pair<Value*,BasicBlock*>>::iterator it = phipairs.begin(); it != phipairs.end(); it++) {
        Value* val = (*it).first;
        BasicBlock* incoming = (*it).second;
        PN->addIncoming(val,incoming);
    }

    PN->addIncoming(ConstantInt::get(*context,
                                    APInt(32,0)),CondBB);

    return PN;
};

/// LetExprAST
LetExprAST::LetExprAST(std::vector<std::pair<std::string, ExprAST*>> Bindings, std::vector<ExprAST*> Body):
    Bindings(std::move(Bindings)), Body(Body) {};

void LetExprAST::visit() {
    *drv.outputTarget << "[let [bindings ";

    for (unsigned i=0, e=Bindings.size(); i<e; i++) {
        *drv.outputTarget << "[= " << drv.opening << Bindings[i].first << drv.closing;
        Bindings[i].second->visit();
        *drv.outputTarget << "]";
    };

    *drv.outputTarget << "][in ";

    for (ExprAST* expr: Body) {
        expr->visit();
    }

    *drv.outputTarget << "]]";
};

Value *LetExprAST::codegen(driver& drv) {
    Function *function = builder->GetInsertBlock()->getParent();
    std::map<std::string,AllocaInst*> AllocaTmp;
    std::map<std::string,AllocaInst*>::iterator it;

    for (int j=0, e=Bindings.size(); j<e; j++) {
        std::string ide = Bindings.at(j).first;

        Value *boundval = Bindings.at(j).second->codegen(drv);

        if (!boundval) {
            return nullptr;
        }

        AllocaInst *BInst = MakeAlloca(function, ide);
        builder->CreateStore(boundval, BInst);

        //AllocaInst *BInst = static_cast<AllocaInst *>(boundval);
        it = drv.NamedValues.find(ide);

        if(it != drv.NamedValues.end()) {
            AllocaTmp[ide] = drv.NamedValues[ide];
        }

        drv.NamedValues[ide] = BInst;
    }

    Value *letVal;

    for (ExprAST* expr : Body) {
        Value *exprVal = expr->codegen(drv);

        if (expr == Body.back()) {
            letVal = exprVal;
        }
    }

    for (int j=0, e=Bindings.size(); j<e; j++) {
        std::string ide = Bindings.at(j).first;
        it = AllocaTmp.find(ide);

        if (it != AllocaTmp.end()) {
            drv.NamedValues[ide] = AllocaTmp[ide];
        }
    }
   	return letVal;
};

/// GlobalDefAST
GlobalDefAST::GlobalDefAST(std::string name): name(name) {
   Val = new NumberExprAST(0);
};
GlobalDefAST::GlobalDefAST(std::string name, ExprAST* Val):
         name(name), Val(Val) {initialized = true;};

void GlobalDefAST::visit() {
    *drv.outputTarget << "[global " << drv.opening << name << drv.closing;
    Val->visit();
    *drv.outputTarget << "]";
};

Value *GlobalDefAST::codegen(driver& drv) {
    GlobalVariable* G = module->getNamedGlobal(name);

    if (G) {
        LogErrorV("Variabile globale "+name+" già definita");
        return nullptr;
    };

    ConstantInt* V = static_cast<ConstantInt*>(Val->codegen(drv));

    if (V) {
        G = new GlobalVariable(*module,
                                IntegerType::get(*context,32),
                                true,
                                GlobalValue::WeakAnyLinkage,
                                V,
                                name);
        G->print(errs());
        fprintf(stderr, "\n");
        return G;
    }
    return nullptr;
};

/// PrototypeAST
PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Params):
                          Name(Name), Params(std::move(Params)) {External=false;};

lexval PrototypeAST::getLexVal() const {
   	lexval lval = Name;
   	return lval;
};
void PrototypeAST::setext() { External = true; };

const std::vector<std::string>& PrototypeAST::getParams() const {
    return Params;
};

void PrototypeAST::visit() {
    if (External) *drv.outputTarget << "[extern ";
    *drv.outputTarget << drv.opening << Name << drv.closing << "[params ";

    for (auto it=Params.begin(); it!=Params.end(); ++it) {
        *drv.outputTarget << drv.opening << *it << drv.closing;
    };

    *drv.outputTarget << "]";

    if (External) *drv.outputTarget << "]";
};

int PrototypeAST::paramssize() {
    return Params.size();
};

Function *PrototypeAST::codegen(driver& drv) {
    /* Generates code for a function prototype.
        As a first step, it must create an appropriate structure
        that defines the "complete type" of the function,
        i.e., the return type and parameter types.
        In our case, the only base type is integers
    */
    std::vector<Type*> intarray(Params.size(), IntegerType::get(*context,32));
    FunctionType *FT = FunctionType::get(IntegerType::get(*context,32),
                                        intarray, false);

    // Now we can define the function
    Function *F = Function::Create(FT, Function::ExternalLinkage, Name, *module);

    // For each parameter of function F (which, it's good to remember, is the LLVM
    // representation of a function, not a C++ function) we now assign the name specified
    // by the programmer and present in the AST node related to the prototype
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(Params[Idx++]);

        // The code is emitted only if it's the prototype of an external
        // function. If instead the prototype is part of the definition of an internal
        // function (prototype+body) then the emission is done by the function's codegen
        if (External) {
            F->print(errs());
            fprintf(stderr, "\n");
        };
    }

    return F;
}

/// FunctionAST
FunctionAST::FunctionAST(PrototypeAST* Proto, std::vector<ExprAST*> Body): Proto(Proto), Body(Body) {};

void FunctionAST::visit() {
  	 *drv.outputTarget << "[function ";
  	 Proto->visit();

    for (ExprAST* expr: Body) {
        expr->visit();
    }

    *drv.outputTarget << "]";
};

int FunctionAST::nparams() {
  	 return Proto->paramssize();
};

Function *FunctionAST::codegen(driver& drv) {
    // Verify that the function is not already present in the module and thus that
    // we are not "attempting" a redefinition

    std::string FunName = std::get<std::string>(Proto->getLexVal());
    Function *function = module->getFunction(FunName);

    if (function) {
        // If the function is already defined, an error message is given and we exit
        LogErrorV("Function " + FunName + " already defined");
        return nullptr;
    }

    // The function is not defined.
    // First we generate the prototype code, which will be emitted after
    // generating the rest as well.
    function = Proto->codegen(drv);

    // If for some reason the prototype generation fails
    // we give an error message and "bounce" the failure back to the caller
    if (!function) {
        LogErrorV("Definition of function " + FunName + " failed");
        return nullptr;
    }

    // The function has been defined and, at the moment, "contains" only the code
    // to generate its prototype. It's therefore time to generate
    // the rest of the function
    // First, we define a Basic Block in which to insert the code
    BasicBlock *BB = BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(BB);

    // Second, we need to deal with the formal parameters which will be
    // referenced in the body (otherwise they would be useless).
    // The parameters are inserted in a symbol table. The access key
    // will be the identifier used by the programmer.
    // The value instead will be the memory area where the argument
    // will be stored at the time of the call.
    for (auto &Arg : function->args()) {
        // For each formal parameter we allocate space on the stack
        AllocaInst *Alloca = MakeAlloca(function, Arg.getName());
        // ... and create an instruction that stores in a register the
        // pointer to the allocated area
        builder->CreateStore(&Arg, Alloca);
        // ... and register the same address in the symbol table
        drv.NamedValues[std::string(Arg.getName())] = Alloca;
    }

    // Now we can finally generate the code corresponding to the body (which can
    // reference the symbol table)
    //

    Value *LastVal = nullptr;
    bool hasExplicitReturn = false;

    // Generating IR for each expression within the function body
    for (ExprAST* expr: Body) {
        Value *exprVal = expr->codegen(drv);

        if (!exprVal) {
            // If the body caused an error, we delete the function
            // definition from the module.
            function->eraseFromParent();
            return nullptr;
        }

        // Check if the current expression is a return expression
        if (ExprAST* RetExpr = dynamic_cast<RetExprAST*>(expr)) {
            hasExplicitReturn = true;

            LastVal = exprVal;
        }
    }

    // If there is no return expression, 0 is returned
    if (!hasExplicitReturn) {
        builder->CreateRet(ConstantInt::get(*context, APInt(32,0)));
    }

    // finally emit the function code to stderr
    function->print(errs());
    fprintf(stderr, "\n");

    return function;
};

/// ForExprAST
ForExprAST::ForExprAST(std::pair<std::string, ExprAST*> binding, ExprAST* condExpr, ExprAST* endExpr, std::vector<ExprAST*> Body)
    : binding(binding), condExpr(condExpr), endExpr(endExpr), Body(Body) {};

void ForExprAST::visit() {
    *drv.outputTarget << "[for [expressions";

    *drv.outputTarget << "[= " << drv.opening << binding.first << drv.closing;
    binding.second->visit();
    condExpr->visit();
    endExpr->visit();

    *drv.outputTarget << "][in ";

    for (ExprAST* expr: Body) {
        expr->visit();
    }

    *drv.outputTarget << "]]";
};

Value* ForExprAST::codegen(driver& drv) {
    Function *function = builder->GetInsertBlock()->getParent();

    // In order to implement the for in the IR four BB are created:
    // conditionBlock: checks the for condition and operate the related branching
    // loopBlock: executes the loop instructions
    // updateBlock: updates the loop counter variable
    // exitBlock: returns the value computed by the loopBlock
    BasicBlock *conditionBlock = BasicBlock::Create(*context, "condition", function);
    BasicBlock *loopBlock = BasicBlock::Create(*context, "loop", function);
    BasicBlock *updateBlock = BasicBlock::Create(*context, "update", function);
    BasicBlock *exitBlock = BasicBlock::Create(*context, "exit", function);

    // Entry BB
    std::string ide = binding.first;
    Value *counterValue = binding.second->codegen(drv);

    if (!counterValue) {
        return nullptr;
    }

    AllocaInst *counterInst = MakeAlloca(function, ide);
    builder->CreateStore(counterValue, counterInst);

    std::pair<std::string, AllocaInst*> allocaTmp;
    std::map<std::string,AllocaInst*>::iterator it;

    allocaTmp.first = ide;

    // Checks if a variable with the same name given to the counter has already been declared
    // If so, it is saved in a temporary register in order to use the symbol table
    it = drv.NamedValues.find(ide);
    if (it != drv.NamedValues.end()) {
        allocaTmp.second = drv.NamedValues[ide];
    }

    drv.NamedValues[ide] = counterInst;

    builder->CreateBr(conditionBlock);

    // Condition BB
    builder->SetInsertPoint(conditionBlock);

    Value *condExprValue = condExpr->codegen(drv);

    if (!condExprValue) {
        return nullptr;
    }

    builder->CreateCondBr(condExprValue, loopBlock, exitBlock);

    // Loop BB
    builder->SetInsertPoint(loopBlock);

    Value *retVal = ConstantInt::get(*context, APInt(32,0));

    // The value computed within the loop is stored in order to be returned
    for (ExprAST* expr: Body) {
        Value *exprVal = expr->codegen(drv);

        if (dynamic_cast<RetExprAST*>(expr)) {
            retVal = exprVal;
        }
    }

    builder->CreateBr(updateBlock);

    // Update BB
    builder->SetInsertPoint(updateBlock);

    Value *endExprValue = endExpr->codegen(drv);
    builder->CreateStore(endExprValue, counterInst);

    builder->CreateBr(conditionBlock);

    // Exit BB
    builder->SetInsertPoint(exitBlock);

    if (it != drv.NamedValues.find(ide)) {
        drv.NamedValues[ide] = allocaTmp.second;
    }

    return retVal;
};
