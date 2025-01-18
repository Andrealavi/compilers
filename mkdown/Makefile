.PHONY: clean all

all: mk

mk: parser.o scanner.o main.o
	clang++ -o mk parser.o scanner.o main.o

parser.o: parser.cpp
	clang++ -c parser.cpp

scanner.o: scanner.cpp parser.hpp
	clang++ -c scanner.cpp

main.o: main.cpp parser.hpp
	clang++ -c main.cpp

parser.cpp: parser.yy
	bison -o parser.cpp parser.yy

scanner.cpp: scanner.ll
	flex -o scanner.cpp scanner.ll

clean:
	rm -f *~ *.o calc++ scanner.cpp parser.cpp parser.hpp
