#include <iostream>

/*
This program should identify the language Lp which is made of all the string with an even number of 1

examples:

110011 --> Accept
001110 --> Reject
*/

int main(int argc, char **argv) {

    if (argc > 1) {
        std::string str = argv[1];

        int c = 0;

        for (std::string::iterator I = str.begin(); I != str.end(); I++) {
            if (*I != '0' && *I != '1') {
                std::cout << "Reject" << std::endl;
            }

            if (*I == '1') {
                c++;
            }
        }

        /*
            if (c % 2 == 0) {
                std::cout << "Accept" << std::endl;
            } else {
                std::cout << "Reject" << std::endl;
            }
        */

        // It is possible to use bit a bit AND since the number will be even if the last bit is set to 0
        if ((c & 1) == 0) {
            std::cout << "Accept" << std::endl;
        } else {
            std::cout << "Reject" << std::endl;
        }
    }

    return 0;
}
