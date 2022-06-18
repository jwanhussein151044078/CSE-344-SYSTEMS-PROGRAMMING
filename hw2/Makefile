exe: utilities.o programs.o program.o
	gcc -o program.exe program.o programs.o utilities.o -lm

utilities.o : utilities.c
	gcc -c utilities.c -lm 

programs.o : programs.c
	gcc -c programs.c

program.o : program.c
	gcc -c program.c

clean: 
	rm *.o *.exe 