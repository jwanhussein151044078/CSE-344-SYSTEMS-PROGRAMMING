# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
# include <sys/time.h>




/*

# include "cache.h"
# include "GRAPH.h"
*/
	
# define BUF_SIZE 256

int parsing_commandline(int argc, char *argv[] ,char **IP ,int *PORT , int *S , int *D);


int main(int argc , char* argv[]){
	char* IP;
	int PORT,S,D;
	char buffer[BUF_SIZE];
	int len ,r, byteread = 0;
	char ch ;

	struct timeval  tv1, tv2;
	if(parsing_commandline(argc,argv,&IP,&PORT,&S,&D) == -1){
		printf("invalid command line !!\nEXPECTED :\n\t");
		printf("./client.exe -a IP_ADDR -p PORT -s (unsigned integer) -d (unsigned integer)\n");
		return 0 ;
	}

	int sfd ;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(IP);

	sfd = socket(AF_INET,SOCK_STREAM,0);
	printf("Client (%d) connecting to %s:%d\n",getpid(),IP,PORT );
	if(connect(sfd,(struct sockaddr *)&addr ,sizeof(addr)) < 0){
		printf("client (%d) could not connect to %s:%d",getpid(),IP,PORT );fflush(stdout);
		perror(":");
		return 0 ;
	}
	printf("Client (%d) connected and requesting a path from node %d to %d \n",getpid(),S,D );
	buffer[0] = '\0';
	sprintf(buffer,"%d,%d\n",S,D);
	len = strlen(buffer);
	if(write(sfd,buffer,len) != len ){
		perror("write ERROR :");
		return 0;
	}
	gettimeofday(&tv1, NULL);
	do{
		r = read(sfd,&ch,1);
		if(r == -1){
			perror("read ERROR :");
			return 0;
		}
		else if(r == 0){
			printf("Unexpected EOF form server !!\n");
			return 0 ;
		}else if(ch == '\n'){
			gettimeofday(&tv2, NULL);
			if(byteread == 0){
				printf("Server’s response to (%d): NO PATH, arrived in %f seconds.\n",getpid(),(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec) );
			}else{
				printf("Server’s response to (%d): %s, arrived in %f seconds, shutting down\n",getpid(),buffer,(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec) );
			}
		}else{
			buffer[byteread] = ch ;
			buffer[byteread+1] = '\0' ;
			byteread++;
		}
	}while(ch != '\n');
	return 0 ;
}

/*
int main (){

	struct mycache c = create_cache();
	struct mygraph g = create_graph();
	struct mylinkedlist l ;
	add_edge(&g,0,1);add_edge(&g,0,7);add_edge(&g,0,6);add_edge(&g,0,5);add_edge(&g,0,4);add_edge(&g,0,3);add_edge(&g,0,2);
	add_edge(&g,1,2);add_edge(&g,1,7);add_edge(&g,1,62);add_edge(&g,2,3);add_edge(&g,2,42);add_edge(&g,6,3);add_edge(&g,4,2);
	add_edge(&g,1,1);add_edge(&g,5,7);add_edge(&g,56,101);add_edge(&g,7,56);add_edge(&g,42,4);add_edge(&g,0,3);add_edge(&g,0,2);
	while(1){
	int i,j;
	scanf("%d %d",&i,&j);
	print_graph(g);
	l = bfs(&g,i,j);
	if(l.size == 0){
		printf("no path\n");
	}else{
		add_path(&c,l);
	}

	printf("------------------\n" );
	print_cache(c);
	}
	free_graph(&g);
	free_cache(&c);
	return 0 ;
}
*/
int parsing_commandline(int argc, char *argv[] ,char **IP ,int *PORT , int *S , int *D){
	int opt;
	while((opt = getopt(argc, argv, ":a:p:s:d:")) != -1){	
    	
    	if(opt == 'a' && optind == 3){
    		*IP = optarg ;
    	}else if (opt == 'p' && optind == 5){
    		sscanf(optarg , "%d" , PORT); 
    	}else if (opt == 's' && optind == 7){
    		sscanf(optarg , "%d" , S);
    	}else if (opt == 'd' && optind == 9){
    		sscanf(optarg , "%d" , D); 
    	}else{
     		return -1 ;
    	}   
    }      
    if(optind < 9){
    	return -1;
    }
    return 0 ;
}