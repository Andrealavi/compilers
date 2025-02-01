#ifndef DRIVER_HH
#define DRIVER_HH
#include "parser.hpp"

/************************* IR specific modules ***************************/
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

/**************** C++ data structures used by the compiler *******************/
#include <string>
#include <map>
#include <variant>
#include <algorithm>
#include <vector>
#include <set>
#include <iterator>

/******* "Lexical value" data type for numbers and identifiers ********/
typedef std::variant<std::string,int> lexval;
const lexval NONE = 0;

/********** Driver class to manage the compilation process ***********/
class driver {
    public:
    	driver();                    // Constructor
    	void scan_begin();           // Implemented in the scanner
    	void scan_end();             // Implemented in the scanner
    	int parse(const std::string& f); // Initializes and executes the parsing process
    	void codegen();              // Produces intermediate code by visiting the Abstract
    								// Syntax Forest (ASF)
        void addConstant(std::string constantName);
        bool isConstant(std::string identifier);

    	std::map<std::string, AllocaInst*> NamedValues;
    								// Associative table to implement scope mechanisms and semantic analysis
                                    // Values are added when generating a function or a letexpr binding
        std::vector<std::string> forwardDeclarations = {};
        std::vector<std::set<std::string>> constantsScopes = {std::set<std::string>()};
        std::vector<LoopExprAST*> loopStack = {};
        std::vector<DefAST*> root;   // Vector of ASTs, one for each definition in the source file
    	yy::location location;       // Used by the scanner to locate tokens
    	std::string file;            // Source file
    	std::ostream* outputTarget;  // Output stream for ASTs in Latex
    	bool toLatex;                // Enables writing ASTs to file in Latex
    	std::string opening, closing;// Parentheses for AST visualization. In case
    								// a latex file is written, they are respectively
    								// followed and preceded by the $ symbol
    	bool trace_parsing;          // If true, enables debug traces in the parser
    	bool trace_scanning;         // If true, enables debug traces in the scanner
};

/***************************************************************************/
/****************** Hierarchy of classes that define *************************/
/************************ language constructs *******************************/
/***************************************************************************/

/// RootAST - Base class of the hierarchy
class RootAST {
    public:
    	virtual ~RootAST() {};
    	virtual void visit() {};
    	virtual Value *codegen(driver& drv) { return nullptr; };
};

/// DefAST - Base class for all definition nodes
class DefAST : public RootAST {
    public:
    	virtual ~DefAST() {};
};

/// ExprAST - Base class for all expression nodes
class ExprAST : public RootAST {
    public:
    	virtual ~ExprAST() {};
};

/// ExprAST - Base class for all loop nodes
class LoopExprAST : public ExprAST{
    public:
        virtual ~LoopExprAST() {};
};

/// NumberExprAST - Class for representing numeric constants
class NumberExprAST : public ExprAST {
    private:
    	int Val;

    public:
    	NumberExprAST(int Val);
    	void visit() override;
    	virtual lexval getLexVal() const;  // Getter, included for completeness
    	Constant *codegen(driver& drv) override;
};

/// ArrayExprAST - Class for representing arrays
class ArrayExprAST : public ExprAST {
    private:
        std::string name;
    	std::vector<ExprAST*> Values;
        int numElements;
        bool isComprehension = false;

    public:
    	ArrayExprAST(std::string name, std::vector<ExprAST*> Values);
        ArrayExprAST(std::string name, ExprAST* comprehensionValue);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// BoolConstAST - Class for representing boolean constants
class BoolConstAST : public ExprAST {
    private:
    	int boolVal;  // Val must be 0 or 1

    public:
    	BoolConstAST(int Val);
    	void visit() override;
    	lexval getLexVal() const;  // Getter, included for completeness
    	Constant *codegen(driver& drv) override;
};

/// IdeExprAST - Class for representing identifier references
class IdeExprAST : public ExprAST {
    private:
    	std::string Name;
        int index = -1;

    public:
    	IdeExprAST(std::string &Name);
        IdeExprAST(std::string &Name, int index);
    	lexval getLexVal() const;
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// AssignmentExprAST - Class for representing assignments
class AssignmentExprAST : public ExprAST {
    private:
        std::pair<std::string, ExprAST*> binding;
        bool isConst;
        int index = -1;

    public:
        AssignmentExprAST(std::pair<std::string, ExprAST*> binding);
        AssignmentExprAST(std::pair<std::string, ExprAST*> binding, bool isConst);
        AssignmentExprAST(std::pair<std::string, ExprAST*> binding, int index);
        lexval getLexVal() const;
        void visit() override;
        Value *codegen(driver& drv) override;
};

/// RetExprAST - Class that represents the return instruction
class RetExprAST : public ExprAST {
    private:
        ExprAST* returnExpr;

    public:
        RetExprAST(ExprAST* returnExpr);
        void visit() override;
        Value *codegen(driver& drv) override;
};

/// BinaryExprAST - Class for representing binary operators
class BinaryExprAST : public ExprAST {
    private:
    	std::string Op;
    	ExprAST* LHS;
    	ExprAST* RHS;

    public:
    	BinaryExprAST(std::string Op, ExprAST* LHS, ExprAST* RHS);
        ExprAST* getLHS();
        ExprAST* getRHS();
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// ExponentiationExprAST - Class for representing exponentiation operator
class ExponentiationExprAST : public ExprAST {
    private:
    	ExprAST* Base;
    	ExprAST* Exponent;

    public:
    	ExponentiationExprAST(ExprAST* Base, ExprAST* Exponent);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// UnaryExprAST - Class for representing unary operators
class UnaryExprAST : public ExprAST {
    private:
    	std::string Op;
    	ExprAST* RHS;

    public:
    	UnaryExprAST(std::string Op, ExprAST* RHS);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// CallExprAST - Class for representing function calls
class CallExprAST : public ExprAST {
    private:
    	std::string Callee;
    	std::vector<ExprAST*> Args;  // ASTs for evaluating arguments

    public:
    	CallExprAST(std::string Callee, std::vector<ExprAST*> Args);
    	lexval getLexVal() const;
        void addArg(ExprAST* arg);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// PipExprAST - Class for representing pipeline function calls
class PipExprAST : public ExprAST {
    private:
        std::vector<ExprAST*> Calls;

    public:
        PipExprAST(std::vector<ExprAST*> Calls);
        void visit() override;
        Value *codegen(driver& drv) override;
};

/// IfExprAST - Class that represents the "conditional" construct
class IfExprAST : public ExprAST {
    private:
    	std::vector<std::pair<ExprAST*, std::vector<ExprAST*>>> IfThenSeq;
    public:
    	IfExprAST(std::vector<std::pair<ExprAST*, std::vector<ExprAST*>>> IfThenSeq);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// TernaryExprAST - Class that represents ternary operator construct
class TernaryExprAST : public ExprAST {
    private:
        ExprAST* boolexpr;
        ExprAST* ifTrueExpr;
        ExprAST* ifFalseExpr;

    public:
        TernaryExprAST(ExprAST* boolexpr, ExprAST* ifTrueExpr, ExprAST* ifFalseExpr);
        void visit() override;
        Value *codegen(driver& drv) override;
};

/// LetExprAST - Class for representing expressions with local
/// environment definition
class LetExprAST : public ExprAST {
    private:
    	std::vector<std::pair<std::string, ExprAST*>> Bindings;
    	std::vector<ExprAST*> Body;
    public:
    	LetExprAST(std::vector<std::pair<std::string, ExprAST*>> Bindings, std::vector<ExprAST*> Body);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// GlobalDefAST - Class for representing global variables
class GlobalDefAST : public DefAST {
    private:
    	std::string name;
        bool initialized = false;
        ExprAST* Val;

    public:
    	GlobalDefAST(std::string name);
        GlobalDefAST(std::string name, ExprAST* Val);
    	void visit() override;
    	Value *codegen(driver& drv) override;
};

/// PrototypeAST - Class for representing function prototypes
/// (name, number and name of parameters; in this case the type is implicit
/// because it's unique)
class PrototypeAST : public DefAST {
    private:
    	std::string Name;
    	bool External;
        bool Forward;
    	std::vector<std::string> Params;

    public:
    	PrototypeAST(std::string Name, std::vector<std::string> Params);
    	lexval getLexVal() const;
    	void setext();
        void setfor();
        bool checkForward();
    	const std::vector<std::string> &getParams() const;
    	void visit() override;
    	int paramssize();
    	Function *codegen(driver& drv) override;
};

/// FunctionAST - Class that represents a function definition
class FunctionAST : public DefAST {
    private:
    	PrototypeAST* Proto;
    	std::vector<ExprAST*> Body;
    	bool external;

    public:
    	FunctionAST(PrototypeAST* Proto, std::vector<ExprAST*> Body);
    	Function *codegen(driver& drv) override;
    	void visit() override;
    	int nparams();
};

/// ForExprAST - Class that represents a for construct
class ForExprAST : public LoopExprAST {
    private:
        std::pair<std::string, ExprAST*> binding;
        ExprAST* condExpr;
        ExprAST* endExpr;
        std::vector<ExprAST*> Body;

    public:
        ForExprAST(std::pair<std::string, ExprAST*> binding, ExprAST* condExpr, ExprAST* endExpr, std::vector<ExprAST*> Body);
        Value *codegen(driver& drv) override;
        void visit() override;
};

/// ComprExprAST - Class that represents array comprehension construct
class ComprExprAST : public LoopExprAST {
    private:
        std::pair<std::string, ExprAST*> binding;
        ExprAST* condExpr;
        ExprAST* endExpr;
        std::string comprehensionName;
        ExprAST* expr;

    public:
        ComprExprAST(std::pair<std::string, ExprAST*> binding, ExprAST* condExpr, ExprAST* endExpr, ExprAST* expr);
        Value *codegen(driver& drv) override;
        void setComprehensionName(std::string name);
        void visit() override;
};

/// DoWhileExprAST - Class that represents a do...while construct
class DoWhileExprAST : public LoopExprAST {
    private:
        ExprAST* condExpr;
        std::vector<ExprAST*> Body;

    public:
        DoWhileExprAST(ExprAST* condExpr, std::vector<ExprAST*> Body);
        Value *codegen(driver& drv) override;
        void visit() override;
};

/// ForRangeExprAST - Class that represents a for range constructs
class ForRangeExprAST : public LoopExprAST {
    private:
        ExprAST* elementExpr;
        ExprAST* arrayExpr;
        std::vector<ExprAST*> Body;

    public:
        ForRangeExprAST(ExprAST* elementExpr, ExprAST* arrayExpr, std::vector<ExprAST*> Body);
        Value *codegen(driver& drv) override;
        void visit() override;
};

/// BreakExprAST - Class that represents a break instruction
class BreakExprAST : public ExprAST {
    public:
        BreakExprAST();
        void visit() override;
        Value *codegen(driver& drv) override;
};

#endif // ! DRIVER_HH
