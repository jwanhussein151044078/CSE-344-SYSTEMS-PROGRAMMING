exe : utilities.o programA.o programB.o
	gcc -o programA.exe programA.o utilities.o -lm
	gcc -o programB.exe programB.o utilities.o -lm

utilities.o : utilities.c
	gcc -c utilities.c -lm 

programA.o : programA.c
	gcc -c programA.c

programB.o : programB.c
	gcc -c programB.c


clean: 
	rm *.o *.exe 