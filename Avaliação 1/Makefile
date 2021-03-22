OUTPUT = prova.exe
WARNING_FLAGS = -Wall -Wno-comment
DEBUG_FLAGS = -g

all:
	gcc prova.c -o $(OUTPUT) -fopenmp -lm $(WARNING_FLAGS)

debug:
	gcc prova.c -o $(OUTPUT) -fopenmp -lm $(DEBUG_FLAGS) $(WARNING_FLAGS)

run:
	./$(OUTPUT)