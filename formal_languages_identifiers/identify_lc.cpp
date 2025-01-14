#include <iostream>
#include <set>

/*
This program should identify the language Lc = {x \in A* : exist y \in A*, a \in A, x = yca}

A = {a, b, c}
*/

// Remember that char is not part of the standard library but is the default so it does not need std::
std::set<char> A = {'a', 'b', 'c'};

int main(int argc, char** argv) {

    if (argc > 1) {
        std::string str = argv[1];

        char second_to_last = str.rbegin()[1];

        if (second_to_last != 'c') {
            std::cout << "Reject" << std::endl;
            return 1;
        }

        std::string::iterator I = str.begin();

        for (;I != str.end(); I++) {
            if (A.count(*I) != 1) {
                std::cout << "Reject" << std::endl;
                return 1;
            }
        }

        std::cout << "Accept" << std::endl;
    }

    return 0;
}
