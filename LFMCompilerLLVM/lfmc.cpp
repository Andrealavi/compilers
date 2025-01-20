#include <iostream>
#include "driver.hpp"
#include <fstream>
#include <string>

// External variables that are declared in driver.cpp
// Are used to generate LLVM IR code
extern LLVMContext *context;
extern Module *module;
extern IRBuilder<> *builder;

driver drv;

int main(int argc, char *argv[]) {
	bool verbose = false;
	bool latex = false;
	bool gencode = false;
	static std::ofstream outfile;

	// C++ Raw string literal
	// R"delimiter(...content...)delimiter"
	// The raw string literal is used to write strings
	// without using escape chars
	//
	// The string defines the setup
	// for the latex generated code
	std::string latex_preamble = R"PREAMBLE(
        \documentclass[12pt]{article}
        \usepackage{synttree}
        \begin{document}
        )PREAMBLE";

	std::cout << argv[0] << std::endl;

	for (int i = 1; i < argc; ++i) {
		if (argv[i] == std::string("-p")) {
			drv.trace_parsing = true;
		} else if (argv[i] == std::string("-s")) {
			drv.trace_scanning = true;
		} else if (argv[i] == std::string("-v")) {
			verbose = true;
		} else if (argv[i] == std::string("-l")) {
			latex = true; // Enables latex code generation
		} else if (argv[i] == std::string("-c")) {
			gencode = true; // Enabels LLVM IR code generation
		} else if (!drv.parse(argv[i])) {
			std::cout << "Parse successful\n";

			if (latex || verbose) {
			    // Creates latex file and configures the output stream accordingly
				if (latex) {
					drv.toLatex = true;
					drv.opening = "[$";
					drv.closing = "$]";

					outfile.open(argv[i] + std::string(".tex"));

					drv.outputTarget = &outfile;

					*drv.outputTarget << latex_preamble << std::endl;
					*drv.outputTarget << "{\\bf\\LARGE AST Forest of " << argv[i] << "}\n";
					*drv.outputTarget << "\\vspace{1cm}\n\n";
				} else {
					drv.outputTarget = &std::cout;
				}

				// Visit the AST
				for (DefAST *tree : drv.root) { // A for is used since there can be a forest of different ASTs
					if (latex) {
						*drv.outputTarget << "\\synttree";
					}

					tree->visit();

					if (latex) {
						*drv.outputTarget << "\n\\par\\vspace{1cm}\n";
					} else {
						std::cout << std::endl;
					}
				}

				if (latex) {
					*drv.outputTarget << "\\end{document}";
				}
				outfile.close();
			}

			// Generates LLVM IR code
			if (gencode) {
				drv.codegen();
			}
		} else {
			return 1;
		}
	}

	return 0;
}
