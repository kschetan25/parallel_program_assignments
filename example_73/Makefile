CC = gcc
LIB = -pthread
PROG0 = example_73.exe

%.exe : %.o
	$(CC) $(LIB) -o $@ $^

%.o : %.c
	$(CC) $(LIB) -c -o $@ $^

prog: $(PROG0)

clean:
	rm -f *.exe *.o
