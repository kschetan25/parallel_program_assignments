CC = gcc
LIB = -pthread
PROG0 = example_73.exe
PROG1 = example_77.exe
PROG2 = gaussian.exe

%.exe : %.o
	$(CC) $(LIB) -o $@ $^

%.o : %.c
	$(CC) $(LIB) -c -o $@ $^

prog: $(PROG0) $(PROG1) $(PROG2)

clean:
	rm -f *.exe *.o
