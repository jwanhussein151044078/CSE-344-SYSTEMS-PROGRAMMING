exe: program.o utilities.o actors.o
	gcc -o program.exe utilities.o actors.o program.o -lrt -pthread 

utilities.o:utilities.c
	gcc -c utilities.c -lm

program.o: program.c
	gcc -c program.c -lrt -pthread

actors.o: actors.c
	gcc -c actors.c -lrt


clean: 
	rm *.o *.exe 
