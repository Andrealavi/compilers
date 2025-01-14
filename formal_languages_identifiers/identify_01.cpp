#include <iostream>
#include <set>

/*
This program should identify all the strings in which a 0 is always followed by 1

examples:

010101 --> Accept
00101 --> Reject
*/

std::set<char> B = {'0', '1'};

int main(int argc, char **argv) {

    if (argc > 1) {
        std::string str = argv[1];

        if (str.back() == '0') {
            std::cout << "Reject" << std::endl;
            return 1;
        }

        for (std::string::iterator I = str.begin(); I != str.end() - 1; I++) {
            if (*I == '0' && *(I + 1) != '1') {
                std::cout << "Reject" << std::endl;
                return 1;
            }
        }

        std::cout << "Accept" << std::endl;
    }

    return 0;
}
