#include <iostream>

/*
This program should identify all the palindrome strings, i.e that if reversed they are the same

examples:

bob --> Accept
party --> Reject
*/

int main(int argc, char **argv) {

    if (argc > 1) {
        std::string str = argv[1];

        std::string::reverse_iterator rI = str.rbegin();

        for (std::string::iterator I = str.begin(); I < rI.base(); I++, rI++) {
            if (*I != *rI) {
                std::cout << "Reject" << std::endl;
                return 1;
            }
        }

        std::cout << "Accept" << std::endl;
    }

    return 0;
}
