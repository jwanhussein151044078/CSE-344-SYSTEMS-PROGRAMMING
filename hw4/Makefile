exe: program.o 
	gcc -o program.exe program.o -pthread -Wall -Wextra

program.o: program.c
	gcc -c program.c -pthread -Wall -Wextra

clean: 
	rm *.o *.exe 
