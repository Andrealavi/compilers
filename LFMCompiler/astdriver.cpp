#include "astdriver.hpp"
#include "astparser.hpp"

/*
    Explanation of frequent methods:

    - visit: visits the node and, if applicable, its children
    - getLexVal: returns node's lexical value
*/

// driver methods definition

driver::driver(): trace_parsing(false), trace_scanning(false) {};

// Used to make scanning start and end. It also makes the parsing start
int driver::parse (const std::string &f) {
	file = f;
	location.initialize(&file);

	scan_begin();

	yy::parser parser(*this); // parser needs a pointer to driver since %params { *driver } is in the .yy file
	parser.set_debug_level(trace_parsing);
	int res = parser.parse();

	scan_end();

	return res;
}

// NumberExprAST methods definition

NumberExprAST::NumberExprAST(int Val): Val(Val) {};

void NumberExprAST::visit() {
	//std::cout << std::get<int>(getLexVal()) << " ";
	std::cout << "(" << Val << ")";
};

lexval NumberExprAST::getLexVal() const {
	lexval lval = Val;
	return lval;
};

// IdeExprAST methods definition

IdeExprAST::IdeExprAST(std::string &Name): Name(Name) {};

lexval IdeExprAST::getLexVal() const {
	lexval lval = Name;
	return lval;
};

void IdeExprAST::visit() {
	//std::cout << std::get<std::string>(getLexVal()) << " ";
	std::cout << "(" << Name << ")";
};

// BinaryExprAST methods definition

BinaryExprAST::BinaryExprAST(std::string Op, ExprAST* LHS, ExprAST* RHS): Op(Op), LHS(LHS), RHS(RHS) {};

void BinaryExprAST::visit() {
	std::cout << "(" << Op << " ";
	LHS->visit();
	RHS->visit();
	std::cout << ")";
};

// UnaryExprAST methods definition

UnaryExprAST::UnaryExprAST(std::string Op, ExprAST* RHS): Op(Op), RHS(RHS) {};

void UnaryExprAST::visit() {
	std::cout << "(" << Op << " ";
	RHS->visit();
	std::cout << ")";
};

// CallExprAST methods definition

CallExprAST::CallExprAST(std::string Callee, std::vector<ExprAST*> Args): Callee(Callee), Args(std::move(Args)) {};

lexval CallExprAST::getLexVal() const {
	lexval lval = Callee;
	return lval;
};

void CallExprAST::visit() {
	std::cout << std::get<std::string>(getLexVal()) << "( ";
	for (ExprAST* arg : Args) {
	  arg->visit();
	};
	std::cout << ')';
};

// IfExprAST methods definition

IfExprAST::IfExprAST(std::vector<std::pair<ExprAST*, ExprAST*>> IfThenSeq):
	IfThenSeq(std::move(IfThenSeq)) {};

void IfExprAST::visit() {
	std::cout << "(if ";
	for (unsigned i=0, e=IfThenSeq.size(); i<e; i++) {
	   std::cout << "(";
	   IfThenSeq[i].first->visit();
	   std::cout << " | ";
	   IfThenSeq[i].second->visit();
	   std::cout << ")";
	}
};

// LetExprAST methods definition

LetExprAST::LetExprAST(std::vector<std::pair<std::string, ExprAST*>> Bindings, ExprAST* Body):
	Bindings(std::move(Bindings)), Body(Body) {};

void LetExprAST::visit() {
	std::cout << "(let (bindings ";
	for (unsigned i=0, e=Bindings.size(); i<e; i++) {
	  std::cout << "(" << Bindings[i].first << '=';
	  Bindings[i].second->visit();
	  std::cout << ")";
	};
	std::cout << ") in ";
	Body->visit();
	std::cout << ')';
};

// PrototypeExprAST methods definition

PrototypeAST::PrototypeAST(std::string Name, bool External, std::vector<std::string> Args):
	                         Name(Name), External(External), Args(std::move(Args)) {};

PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Args):
	                         Name(Name), Args(std::move(Args)) {External=false;};

lexval PrototypeAST::getLexVal() const {
	lexval lval = Name;
	return lval;
};

const std::vector<std::string>& PrototypeAST::getArgs() const {
	return Args;
};

void PrototypeAST::visit() {
	std::cout << "extern " << Name << "( ";
	for (auto it=Args.begin(); it!=Args.end(); ++it) {
	  std::cout << *it << ' ';
	};
	std::cout << ')';
};

int PrototypeAST::argsize() {
	return Args.size();
};

// FunctionExprAST methods definition

FunctionAST::FunctionAST(PrototypeAST* Proto, ExprAST* Body): Proto(Proto), Body(Body) {
	if (Body == nullptr) external=true;
	else external=false;
};

void FunctionAST::visit() {
	std::cout << std::get<std::string>(Proto->getLexVal()) << "( ";

	for (auto it=Proto->getArgs().begin(); it!= Proto->getArgs().end(); ++it) {
	  std::cout << *it << ' ';
	};

	std::cout << ')';
	Body->visit();
};

int FunctionAST::nparams() {
	return Proto->argsize();
};
