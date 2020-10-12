OUTPUT = atv2.exe
TEST_FILE = input.txt

all:
	gcc *.c -o $(OUTPUT) -fopenmp -lm

run:
	./$(OUTPUT)