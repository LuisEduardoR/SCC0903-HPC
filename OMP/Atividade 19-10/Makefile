OUTPUT = atv.exe
WARNING_FLAGS = -Wall -Wno-comment
DEBUG_FLAGS = -g

all:
	gcc atividade-aula.c -o $(OUTPUT) -fopenmp -lm $(WARNING_FLAGS)

debug:
	gcc atividade-aula.c -o $(OUTPUT) -fopenmp -lm $(DEBUG_FLAGS) $(WARNING_FLAGS)

run:
	./$(OUTPUT)