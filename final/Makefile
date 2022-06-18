exe : server.o GRAPH.o utilities.o LLIST.o QUEUE.o client.o cache.o
	gcc -o server.exe utilities.o GRAPH.o server.o LLIST.o QUEUE.o cache.o -pthread -Wall -Wextra
	gcc -o client.exe client.o -Wall -Wextra



server.o : server.c
	gcc -c server.c -pthread -Wall -Wextra

GRAPH.o : GRAPH.c
	gcc -c GRAPH.c -Wall -Wextra

utilities.o : utilities.c
	gcc -c utilities.c -Wall -Wextra

LLIST.o : LLIST.c
	gcc -c LLIST.c -Wall -Wextra

QUEUE.o : QUEUE.c
	gcc -c QUEUE.c -Wall -Wextra

cache.o : cache.c
	gcc -c cache.c -Wall -Wextra


client.o : client.c
	gcc -c client.c -Wall -Wextra	 
clean: 
	rm *.o *.exe 