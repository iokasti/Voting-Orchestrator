all: main.o votersRegistry.o checkerFunctions.o voter.o
	g++ -pthread -o ../orchestrator main.o votersRegistry.o checkerFunctions.o voter.o


main.o: main.cpp
	g++ -c main.cpp


votersRegistry.o: VotersRegistry/votersRegistry.cpp VotersRegistry/votersRegistry.hpp
	g++ -c VotersRegistry/votersRegistry.cpp


checkerFunctions.o: CheckerFunctions/checkerFunctions.cpp CheckerFunctions/checkerFunctions.hpp
	g++ -c CheckerFunctions/checkerFunctions.cpp


voter.o: Voter/voter.cpp Voter/voter.hpp
	g++ -c Voter/voter.cpp


clean:
	rm -f *.o

