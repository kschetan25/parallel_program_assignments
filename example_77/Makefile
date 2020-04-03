CC = gcc
LIB = -pthread
PROG1 = example_77.exe

%.exe : %.o
	$(CC) $(LIB) -o $@ $^

%.o : %.c
	$(CC) $(LIB) -c -o $@ $^

prog: $(PROG1)

clean:
	rm -f *.exe *.o
