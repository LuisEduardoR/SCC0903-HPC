OUTPUT = atv2.exe
OUTPUT_SEQ = atv2-seq.exe
WARNING_FLAGS = -Wall -Wno-comment
DEBUG_FLAGS = -g

all:
	gcc atividade2.h atividade2.c -o $(OUTPUT) -fopenmp -lm $(WARNING_FLAGS)
	gcc atividade2.h atividade2-seq.c -o $(OUTPUT_SEQ) -fopenmp -lm $(WARNING_FLAGS)

debug:
	gcc atividade2.c -o $(OUTPUT) -fopenmp -lm $(DEBUG_FLAGS) $(WARNING_FLAGS)
	gcc atividade2-seq.c -o $(OUTPUT) -fopenmp -lm $(WARNING_FLAGS)

run:
	./$(OUTPUT)