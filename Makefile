CC=g++
PKLIBS=OGRE OIS
OGINCL= $(shell pkg-config --cflags $(PKLIBS))
CFLAGS=-Wall -g -pthread -I/usr/include/jsoncpp $(OGINCL)
JSONLD=-ljsoncpp
JSONHD=-I/usr/include/jsoncpp
LIBS= $(shell pkg-config --libs $(PKLIBS)) -ljsoncpp
EXENAME=crowdsim
ENHANCED_EXENAME=enhanced_crowdsim


all: Agent.o CrowdObject.o Vector.o Wall.o CrowdWorld.o Render.o
	$(CC) $(CFLAGS) $(OGINCL) main.cpp *.o $(LIBS) -o $(EXENAME)

enhanced: Agent.o ORCAAgent.o CrowdObject.o Vector.o Wall.o CrowdWorld.o EnhancedCrowdWorld.o DatasetLoader.o Render.o
	$(CC) $(CFLAGS) $(OGINCL) enhanced_main.cpp *.o $(LIBS) -o $(ENHANCED_EXENAME)

orca_demo: Agent.o ORCAAgent.o CrowdObject.o Vector.o Wall.o CrowdWorld.o Render.o
	$(CC) $(CFLAGS) $(OGINCL) simple_orca_demo.cpp *.o $(LIBS) -o orca_demo

Agent.o: Agent.cpp
	$(CC) $(CFLAGS) -I. -c Agent.cpp

ORCAAgent.o: ORCAAgent.cpp
	$(CC) $(CFLAGS) -I. -c ORCAAgent.cpp

CrowdObject.o: CrowdObject.cpp
	$(CC) $(CFLAGS) -I. -c CrowdObject.cpp

CrowdWorld.o : CrowdWorld.cpp
	$(CC) $(CFLAGS) -I. -c CrowdWorld.cpp

EnhancedCrowdWorld.o : EnhancedCrowdWorld.cpp
	$(CC) $(CFLAGS) -I. -c EnhancedCrowdWorld.cpp

DatasetLoader.o : DatasetLoader.cpp
	$(CC) $(CFLAGS) -I. -c DatasetLoader.cpp

Vector.o : vector.cpp
	$(CC) $(CFLAGS) -I. -c vector.cpp

Render.o : Render.cpp
	$(CC) $(CFLAGS) $(OGINCL) -I. -c Render.cpp

Wall.o : Wall.cpp
	$(CC) $(CFLAGS) -I. -c Wall.cpp

clean: 
	rm -f *.o *~ *.out $(EXENAME) $(ENHANCED_EXENAME) orca_demo

.PHONY: all enhanced orca_demo clean