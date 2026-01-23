# roll_num: MT25017
# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lpthread -lm

# Naming Convention Variables
ROLL_NO = MT25017
PROG_A = $(ROLL_NO)_Part_A_Program_A
PROG_B = $(ROLL_NO)_Part_A_Program_B
WORKER_SRC = $(ROLL_NO)_Part_B_W1.c
WORKER_OBJ = $(ROLL_NO)_Part_B_W1.o

# Default target
all: $(PROG_A) $(PROG_B)

# Compile Program A (Processes)
$(PROG_A): $(ROLL_NO)_Part_A_Program_A.c $(WORKER_OBJ)
	$(CC) $(CFLAGS) -o $(PROG_A) $(ROLL_NO)_Part_A_Program_A.c $(WORKER_OBJ) $(LDFLAGS)

# Compile Program B (Threads)
$(PROG_B): $(ROLL_NO)_Part_A_Program_B.c $(WORKER_OBJ)
	$(CC) $(CFLAGS) -o $(PROG_B) $(ROLL_NO)_Part_A_Program_B.c $(WORKER_OBJ) $(LDFLAGS)

# Compile worker logic into a modular object file
$(WORKER_OBJ): $(WORKER_SRC)
	$(CC) $(CFLAGS) -c $(WORKER_SRC) -o $(WORKER_OBJ)

# Clean target: Mandatory to remove binaries before submission
clean:
	rm -f $(PROG_A) $(PROG_B) $(WORKER_OBJ)