# Compiler settings
CC=gcc
# Header files in current directory
CFLAGS=-I.

# List of object files to be created from source files
OBJ=unpack.o b010_editor.o

# Rule for creating object files from C source files
# For each .c file, compile it into an .o file
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	#  gcc -c -o unpack.o unpack.c -I.
	#  -c : compile without linking
	#  -o : output file name
	#  <  : input file name
	#  CFLAGS : additional flags

# Link objects into single executable
unpack: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	# gcc -o unpack unpack.o editor_emulation.o -I.

test:
OBJ=test.o b010_editor.o 
# Rule for creating object files from C source files
# For each .c file, compile it into an .o file
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	#  gcc -c -o test.o test.c -I.
	#  -c : compile without linking
	#  -o : output file name
	#  <  : input file name
	#  CFLAGS : additional flags

test: test.o b010_editor.o
		$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o unpack
