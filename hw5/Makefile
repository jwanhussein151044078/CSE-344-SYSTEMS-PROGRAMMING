exe: floristApp.o utilities.o
	gcc -o floristApp.exe utilities.o floristApp.o -pthread -Wall -Wextra

floristApp.o: floristApp.c
	gcc -c floristApp.c -pthread -Wall -Wextra

utilities.o: utilities.c
	gcc -c utilities.c -pthread -Wall -Wextra

clean: 
	rm *.o *.exe 
