OBJS = src/main.cpp

CC=g++

COMPILER_FLAGS = 

LINKER_FLAGS = 

OBJ_NAME = SudokuSolver.exe

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

clean:
	rm $(OBJ_NAME)