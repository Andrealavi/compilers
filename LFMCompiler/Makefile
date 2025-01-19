.PHONY: clean all

all: astgen

astgen:	astdriver.o astparser.o astscanner.o astgen.o
	clang++ -o astgen astdriver.o astparser.o astscanner.o astgen.o

astgen.o:	astgen.cpp
	clang++ -c -std=c++17 astgen.cpp
	
astparser.o: astparser.cpp
	clang++ -c -std=c++17 astparser.cpp
	
astscanner.o: astscanner.cpp astparser.hpp
	clang++ -c -std=c++17 astscanner.cpp
	
astdriver.o: astdriver.cpp astparser.hpp astdriver.hpp
	clang++ -c -std=c++17 astdriver.cpp

astparser.cpp astparser.hpp: 	astparser.yy 
	bison -o astparser.cpp astparser.yy

astscanner.cpp: astscanner.ll
	flex -o astscanner.cpp astscanner.ll

clean:
	rm -f *~ *.o astgen astscanner.cpp astparser.cpp astparser.hpp
