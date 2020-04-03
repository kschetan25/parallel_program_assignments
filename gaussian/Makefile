CC = gcc
LIB = -pthread
PROG2 = gaussian.exe

%.exe : %.o
	$(CC) $(LIB) -o $@ $^

%.o : %.c
	$(CC) $(LIB) -c -o $@ $^

prog: $(PROG2)

clean:
	rm -f *.exe *.o
