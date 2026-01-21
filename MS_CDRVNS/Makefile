CC = g++ -O3 -Wno-deprecated -ftree-vectorize

all: CDRVNSforLOP

CDRVNSforLOP: main.o LOP.o Tools.o PSolution.o random.o SFMT.o Archive.o
	$(CC) main.o LOP.o Tools.o PSolution.o random.o SFMT.o Archive.o -lstdc++ -lm -o CDRVNSforLOP

main.o: main.cpp Tools.hpp ParameterSetting.h LOP.h PSolution.h random.h
	$(CC) -c main.cpp

LOP.o: LOP.cpp LOP.h Tools.hpp
	$(CC) -c LOP.cpp

Tools.o: Tools.cpp Tools.hpp LOP.h
	$(CC) -c Tools.cpp

PSolution.o: PSolution.cpp PSolution.h PSolution_util.cpp random.h
	$(CC) -c PSolution.cpp

random.o: random.cpp random.h SFMT.h
	$(CC) -c random.cpp

SFMT.o: SFMT.c SFMT.h SFMT-params.h SFMT-common.h SFMT-params19937.h
	$(CC) -c SFMT.c

Archive.o: Archive.cpp Archive.h
	$(CC) -c Archive.cpp

clean:
	rm -f core *.o CDRVNSforLOP

