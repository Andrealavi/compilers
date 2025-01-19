/*
    The following macros are header guards.
    They are used in order to prevent multiple definitions of the same code, avoiding compilation errors.
*/
#ifndef DRIVER_HH
# define DRIVER_HH
# include <string>
# include <map>
# include <variant>
# include <vector>
# include "astparser.hpp"

typedef std::variant<std::string,int> lexval;
const lexval NONE = 0;

// Base class of the entire hierarchy of classes that represent
// the program elements
class RootAST {
public:
	virtual ~RootAST() {};
	virtual RootAST *left() {return nullptr;};
	virtual RootAST *right() {return nullptr;};
	virtual lexval getLexVal() {return NONE;};
	virtual void visit() {};
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

/// NumberExprAST - Class for representing numeric constants
class NumberExprAST : public ExprAST {
private:
	int Val;

public:
	NumberExprAST(int Val);
	void visit();
	lexval getLexVal() const;
};

/// IdeExprAST - Class for representing identifier references
class IdeExprAST : public ExprAST {
private:
	std::string Name;

public:
	IdeExprAST(std::string &Name);
	lexval getLexVal() const;
	void visit();
};

/// BinaryExprAST - Class for representing binary operators
class BinaryExprAST : public ExprAST {
private:
	std::string Op;
	ExprAST* LHS;
	ExprAST* RHS;

public:
	BinaryExprAST(std::string Op, ExprAST* LHS, ExprAST* RHS);
	void visit();
};

/// UnaryExprAST - Class for representing unary operators
class UnaryExprAST : public ExprAST {
private:
	std::string Op;
	ExprAST* RHS;

public:
	UnaryExprAST(std::string Op, ExprAST* RHS);
	void visit();
};

/// CallExprAST - Class for representing function calls
class CallExprAST : public ExprAST {
private:
	std::string Callee;
	std::vector<ExprAST*> Args;  // ASTs for evaluating arguments

public:
	CallExprAST(std::string Callee, std::vector<ExprAST*> Args);
	lexval getLexVal() const;
	void visit();
};

/// IfExprAST - Class that represents the "conditional" construct
class IfExprAST : public ExprAST {
private:
	std::vector<std::pair<ExprAST*, ExprAST*>> IfThenSeq;
public:
	IfExprAST(std::vector<std::pair<ExprAST*, ExprAST*>> IfThenSeq);
	void visit();
};

/// LetExprAST - Class for representing expressions with local environment definition
class LetExprAST : public ExprAST {
private:
	std::vector<std::pair<std::string, ExprAST*>> Bindings;
	ExprAST* Body;
public:
	LetExprAST(std::vector<std::pair<std::string, ExprAST*>> Bindings, ExprAST* Body);
	void visit();
};

/// PrototypeAST - Class for representing function prototypes
/// (name, number and name of parameters; in this case the type is implicit
/// because it's unique)
class PrototypeAST : public DefAST {
private:
	std::string Name;
	bool External = false;
	std::vector<std::string> Args;

public:
	PrototypeAST(std::string Name, std::vector<std::string> Args);
	PrototypeAST(std::string Name, bool External, std::vector<std::string> Args);
	lexval getLexVal() const;
	const std::vector<std::string> &getArgs() const;
	void visit();
	int argsize();
};

/// FunctionAST - Class that represents the definition of a function
class FunctionAST : public DefAST {
private:
	PrototypeAST* Proto;
	ExprAST* Body;
	bool external;

public:
	FunctionAST(PrototypeAST* Proto, ExprAST* Body);
	void visit();
	int nparams();
};

// Class that organizes and manages the compilation process
class driver
{
public:
	driver();
	void scan_begin();     // Implemented in scanner
	void scan_end();       // Implemented in scanner
	int parse (const std::string& f);

	std::vector<DefAST*> root;         // At the end of parsing "points" to the AST root
	yy::location location; // Used by scanner to locate tokens
	std::string file;      // Source file

	bool trace_parsing;    // Enables debug traces in parser
	bool trace_scanning;   // Enables debug traces in scanner
};

#endif // ! DRIVER_HH
