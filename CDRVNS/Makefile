CXX = g++
CXXFLAGS = -O3 -Wno-deprecated -ftree-vectorize
LDFLAGS = -lm

debug: CXXFLAGS = -O0 -g -DDEBUG -Wno-deprecated
debug: clean CDRVNSforLOP

release: CXXFLAGS = -O2 -DNDEBUG -Wno-deprecated -ftree-vectorize
release: clean CDRVNSforLOP

all: CDRVNSforLOP

CDRVNSforLOP: main.o LOP.o Tools.o PSolution.o random.o SFMT.o Archive.o
	$(CXX) $(CXXFLAGS) main.o LOP.o Tools.o PSolution.o random.o SFMT.o Archive.o $(LDFLAGS) -o CDRVNSforLOP

main.o: main.cpp Tools.hpp ParameterSetting.h LOP.h PSolution.h random.h
	$(CXX) $(CXXFLAGS) -c main.cpp

LOP.o: LOP.cpp LOP.h Tools.hpp
	$(CXX) $(CXXFLAGS) -c LOP.cpp

Tools.o: Tools.cpp Tools.hpp LOP.h
	$(CXX) $(CXXFLAGS) -c Tools.cpp

PSolution.o: PSolution.cpp PSolution.h PSolution_util.cpp random.h
	$(CXX) $(CXXFLAGS) -c PSolution.cpp

random.o: random.cpp random.h SFMT.h
	$(CXX) $(CXXFLAGS) -c random.cpp

SFMT.o: SFMT.c SFMT.h SFMT-params.h SFMT-common.h SFMT-params19937.h
	gcc -O3 -c SFMT.c   # oppure usa CC/CFLAGS separati se vuoi pulito

Archive.o: Archive.cpp Archive.h
	$(CXX) $(CXXFLAGS) -c Archive.cpp

clean:
	rm -f core *.o CDRVNSforLOP

