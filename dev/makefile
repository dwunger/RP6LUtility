# Compiler settings
CC=gcc
# Header files in current directory
CFLAGS=-I.

# List of object files to be created from source files
OBJ=rparse.o 010editor.o

# Rule for creating object files from C source files
# For each .c file, compile it into an .o file
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	#  gcc -c -o rparse.o rparse.c -I.
	#  -c : compile without linking
	#  -o : output file name
	#  <  : input file name
	#  CFLAGS : additional flags

# Link objects into single executable
rparse: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	# gcc -o rparse rparse.o editor_emulation.o -I.

clean:
	rm -f *.o rparse
