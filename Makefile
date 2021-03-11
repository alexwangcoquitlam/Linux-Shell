cshell: cshell.o
	gcc -g -Wall -o cshell cshell.o

cshell.o: cshell.c
	gcc -g -Wall -c cshell.c

clean:
	$(RM) cshell *.o
