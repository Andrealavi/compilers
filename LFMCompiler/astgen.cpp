#include <iostream>
#include "astdriver.hpp"
#include <string>

driver drv;

int main (int argc, char *argv[]) {
	bool verbose = false;
	std::cout << argv[0] << std::endl;

	for (int i = 1; i < argc; ++i) {
    	if (argv[i] == std::string ("-p")) {
    		drv.trace_parsing = true;
    	} else if (argv[i] == std::string ("-s")) {
    		drv.trace_scanning = true;
    	} else if (argv[i] == std::string("-v")) {
    		verbose = true;
    	} else if (!drv.parse (argv[i])) {
    		std::cout << "Parse successful\n";

    		if (verbose) {
    		    for (DefAST* tree: drv.root) {
    		        tree->visit();
    		        std::cout << std::endl;
    		    }
    		}

    	} else {
    		return 1;
    	}
	}

	return 0;
}
