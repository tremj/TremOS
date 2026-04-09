CC=gcc
CFLAGS=-pthread
FMT=indent

mysh: shell.c interpreter.c shellmemory.c pcb.c queue.c scheduler.c pagetable.c lru.c
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c pcb.c queue.c scheduler.c pagetable.c lru.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o pcb.o queue.o scheduler.o pagetable.o lru.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h queue.c queue.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

rebuild: clean mysh

cleanrun: rebuild
	./mysh