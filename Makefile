CC=g++
CFLAGS=-c -g -pg -Wswitch -Wall -Dlinux -D_DEBUG -std=c++20
#CFLAGS=-c -O1 -Wall
LDFLAGS=-g -pg
#LDFLAGS= 

SOURCES=  \
TinyJS.cpp \
TinyJS_Functions.cpp \
TinyJS_MathFunctions.cpp \
ltn_String.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all: run_tests

run_tests: run_tests.o $(OBJECTS)
	$(CC) $(LDFLAGS) run_tests.o $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f run_tests run_tests.o $(OBJECTS)
