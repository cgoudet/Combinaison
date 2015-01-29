CC = g++ -std=c++0x -lboost_program_options
CLASS = -c
FLAGS = -Wall -Wextra #-Werror #-Wfatal-errors
ROOT = `root-config --libs --cflags --ldflags`
ROOFIT = -lRooFit -lRooFitCore -lRooStats -lHistFactory

EXE = main.exe


all : ${EXE}
	echo "Up to date" 

main.exe : main.o Combine.o CloseCoutSentry.o
	$(CC) $^ -o $@ $(ROOT) $(ROOFIT)


%.o : %.cpp
	$(CC) $(CLASS) $(FLAGS)  $(ROOT) $(ROOFIT) $<