all: main.o collectorFunctions.o resultList.o votersRegistry.o
	g++ -pthread -o ../collector main.o collectorFunctions.o resultList.o votersRegistry.o


main.o: main.cpp
	g++ -c main.cpp


collectorFunctions.o: CollectorFunctions/collectorFunctions.cpp CollectorFunctions/collectorFunctions.hpp
	g++ -c CollectorFunctions/collectorFunctions.cpp 


resultList.o: ResultList/resultList.cpp ResultList/resultList.hpp
	g++ -c ResultList/resultList.cpp 


votersRegistry.o: VotersRegistry/votersRegistry.cpp VotersRegistry/votersRegistry.hpp
	g++ -c VotersRegistry/votersRegistry.cpp 


clean:
	rm -f *.o

