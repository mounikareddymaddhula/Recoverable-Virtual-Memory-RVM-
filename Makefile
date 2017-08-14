all: rvm basic abort multi multi-abort truncate test-case-1 test-case-2 test-case-3 test-case-4 test-case-5

CLEAN := rvm.o librvm.a basic abort multi multi-abort truncate test-case-1 test-case-2 test-case-3 test-case-4 test-case-5

clean:
	rm -f ${CLEAN}   

rvm:
	g++ -c rvm.cpp -o rvm.o
	ar rcs librvm.a rvm.o

basic: basic.c rvm
	g++ -static basic.c -L. -lrvm -o basic

multi: multi.c rvm
	g++ -static multi.c -L. -lrvm -o multi

abort: abort.c rvm
	g++ -static abort.c -L. -lrvm -o abort

multi-abort: multi-abort.c rvm
	g++ -static multi-abort.c -L. -lrvm -o multi-abort

truncate: truncate.c rvm
	g++ -static truncate.c -L. -lrvm -o truncate

test-case-1: test-case-1.c rvm
	g++ -static test-case-1.c -L. -lrvm -o test-case-1

test-case-2: test-case-2.c rvm
	g++ -static test-case-2.c -L. -lrvm -o test-case-2

test-case-3: test-case-3.c rvm
	g++ -static test-case-3.c -L. -lrvm -o test-case-3

test-case-4: test-case-4.c rvm
	g++ -static test-case-4.c -L. -lrvm -o test-case-4

test-case-5: test-case-5.c rvm
	g++ -static test-case-5.c -L. -lrvm -o test-case-5
