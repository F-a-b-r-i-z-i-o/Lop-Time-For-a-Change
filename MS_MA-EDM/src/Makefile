CXXFLAGS = -O2

debug: CXXFLAGS = -O0 -g -DDEBUG
debug: clean main

release: CXXFLAGS = -O2
release: clean main

main: main.cpp crossoverCX.o crossoverOB.o getDistance.o intensify.o print.o utils.o evaluate.o initialize_heuristic.o MA.o Problem.o Individual.o Archive.o
	g++ $(CXXFLAGS) -o $@ $^

Individual.o: Individual.h Individual.cpp
	g++ $(CXXFLAGS) -c -o $@ Individual.cpp 

crossoverCX.o: crossoverCX.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ crossoverCX.cpp

crossoverOB.o: crossoverOB.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ crossoverOB.cpp

getDistance.o: getDistance.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ getDistance.cpp

intensify.o: intensify.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ intensify.cpp

print.o: print.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ print.cpp

evaluate.o: evaluate.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ evaluate.cpp

initialize_heuristic.o: initialize_heuristic.cpp Individual.h 
	g++ $(CXXFLAGS) -c -o $@ initialize_heuristic.cpp

Problem.o: Problem.h
	g++ $(CXXFLAGS) -c -o $@ Problem.cpp

Archive.o: Archive.h Archive.cpp
	g++ $(CXXFLAGS) -c -o $@ Archive.cpp

MA.o: MA.h Individual.h Problem.h MA.cpp
	g++ $(CXXFLAGS) -c -o $@ MA.cpp

utils.o: utils.cpp utils.h
	g++ $(CXXFLAGS) -c -o $@ utils.cpp

clean:
	rm -f main *.o
