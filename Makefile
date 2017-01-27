all:
	cd Checker; make
	cd Collector; make
	cd Orchestrator; make
	cd Voter; make

clean:
	cd Checker; make clean
	cd Collector; make clean
	cd Orchestrator; make clean
	cd Voter; make clean

