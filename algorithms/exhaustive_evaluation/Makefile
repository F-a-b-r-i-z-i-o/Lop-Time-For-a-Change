CPP = g++
CC = gcc
CPPFLAGS = -O2 -Wall
CCFLAGS = -Wall
INCLUDES = -I"/usr/include"
LFLAGS = -L"/usr/lib"
LIBS =
#LIBS = -lm

all: exhaustive_evaluation

clean:
	rm *.o exhaustive_evaluation

lop.o: lop.cpp lop.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c lop.cpp $(LFLAGS) $(LIBS)

timer.o: timer.cpp timer.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c timer.cpp $(LFLAGS) $(LIBS)

exhaustive_evaluation.o: exhaustive_evaluation.cpp exhaustive_evaluation.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c exhaustive_evaluation.cpp $(LFLAGS) $(LIBS)

OBJECTS_exhaustive_evaluation = lop.o timer.o exhaustive_evaluation.o

exhaustive_evaluation: main_exhaustive_evaluation.cpp $(OBJECTS_exhaustive_evaluation)
	$(CPP) $(CPPFLAGS) $(INCLUDES) main_exhaustive_evaluation.cpp $(OBJECTS_exhaustive_evaluation) $(LFLAGS) $(LIBS) -o exhaustive_evaluation
