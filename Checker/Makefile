all: main.o votersRegistry.o checkerFunctions.o collectorFunctions.o
	g++ -pthread -o ../checker main.o votersRegistry.o checkerFunctions.o collectorFunctions.o


main.o: main.cpp
	g++ -c main.cpp


votersRegistry.o: VotersRegistry/votersRegistry.cpp VotersRegistry/votersRegistry.hpp
	g++ -c VotersRegistry/votersRegistry.cpp


checkerFunctions.o: CheckerFunctions/checkerFunctions.cpp CheckerFunctions/checkerFunctions.hpp
	g++ -c CheckerFunctions/checkerFunctions.cpp


collectorFunctions.o: CollectorFunctions/collectorFunctions.cpp CollectorFunctions/collectorFunctions.hpp
	g++ -c CollectorFunctions/collectorFunctions.cpp


clean:
	rm -f *.o

