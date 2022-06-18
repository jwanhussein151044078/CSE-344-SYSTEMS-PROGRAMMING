exe: program.o utilities.o child.o svd.o
	gcc -o program.exe svd.o utilities.o child.o program.o -lm 

utilities.o:utilities.c
	gcc -c utilities.c -lm

program.o: program.c
	gcc -c program.c

child.o: child.c
	gcc -c child.c -lm

svd.o: svd.c
	gcc -c svd.c


clean: 
	rm *.o *.exe 
