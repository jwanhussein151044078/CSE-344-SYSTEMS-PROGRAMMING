# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <semaphore.h>
# include <signal.h>
# include <limits.h>
# include <string.h>
# include <sys/time.h>
# include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <netdb.h>
#include <netinet/in.h>

# include "utilities.h"
# include "LLIST.h"
# include "GRAPH.h"
# include "QUEUE.h"
# include "cache.h"

# define semaphore_name "mysemaphoreX2X0X2X0X6X"

struct thread_arg{
	int id ;
	int fd ;
	int logfd ;
	int state ;		/* if -1 thread is available , else is occupied */
	pthread_cond_t cond_var ;
	struct thread_arg *next;
};
struct info{
	int size;
	struct thread_arg* head ;
	struct thread_arg** pointers ;
};



void handler(int signo);
int server(char* , int ,int , int , int );
int mywrite(int fd , char* buffer);
void print_parameter(int fdl,char* graphfile,char* logfile,int PORT , int S , int X);
void* fthread(void* arg);
void* resizer_thread(void* arg);

int interupt = 0 ;
struct mygraph g ;
sem_t *_semaphore ;
pthread_t *_pool ;
int pool_size;

struct info thread_info ;
int number_of_occupied_thread = 0;
int min;
int max;

struct mycache cache ;

int help ;

pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t server_cond  = PTHREAD_COND_INITIALIZER;
pthread_cond_t resizer_cond = PTHREAD_COND_INITIALIZER;


int AR = 0 ; 
int AW = 0 ; 
int WR = 0 ; 
int WW = 0 ; 

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t oktoread  = PTHREAD_COND_INITIALIZER;
pthread_cond_t oktowrite = PTHREAD_COND_INITIALIZER;





int main(int argc, char *argv[]){
	char *filename ;
	char *log_filename ;
	int PORT ;
	int S ;
	int X ;
	int maxfd ,fdl;
	char currpath[PATH_MAX];
	char graphfile[PATH_MAX];
	char logfile[PATH_MAX];


	if(parsing_commandline(argc,argv,&filename,&log_filename,&PORT,&S,&X) == -1){
		printf("invalid command line !!\nEXPECTED :\n\t");
		printf("./server.exe -i pathToFile -p PORT -o pathToLogFile -s (int) -x (int)\n");
		return 0 ;
	}
	if(S < 2 || X < S){
		printf("Constraints\n\t");
		printf("X >= S >= 2\n");
		return 0 ;
	}

	if(getcwd(currpath, sizeof(currpath)) == NULL) {
       	printf("error getcwd !!\n");
  		return 0 ;      
  	}
  	if(log_filename[0] != '/'){
		strcpy(graphfile,currpath);
		strcat(graphfile,"/");
		strcat(graphfile,filename);				
	}else{
		strcpy(graphfile,filename);
	}

	switch(fork()){
		case -1 : exit(-1) ;
		case 0  : break;
		default : exit(0) ;
	}
	if(setsid()== -1){
		return -1 ;
	}
	switch(fork()){
		case -1 : return -1 ;
		case 0  : {

			struct sigaction sa ;

			if(log_filename[0] != '/'){
				strcpy(logfile,currpath);
				strcat(logfile,"/");
				strcat(logfile,log_filename);				
			}else{
				strcpy(logfile,log_filename);
			}

			fdl = open(logfile,O_RDWR | O_CREAT | O_APPEND,S_IRWXU | S_IRWXG);
			if(fdl == -1){
				perror("open logfile error :");
				exit(0);
			}

			umask(0);				
			int fd = 0 ;
			chdir("/");
			maxfd = sysconf(_SC_OPEN_MAX);
			if(maxfd == -1){
				maxfd = 1024;
			}
			for(int i = 0 ; i < maxfd ; i++){
				if(fdl != i)close(i);
			}
			close(STDIN_FILENO);
			
			fd = open("/dev/null",O_RDWR);
			if(fd != STDIN_FILENO){
				return 0;
			}
			if(dup2(STDIN_FILENO,STDOUT_FILENO) != STDOUT_FILENO){
				return -1 ;
			}
			if(dup2(STDIN_FILENO,STDERR_FILENO) != STDERR_FILENO){
				return -1 ;
			}
			
			if((_semaphore = sem_open(semaphore_name,O_CREAT | O_EXCL)) == SEM_FAILED){
				mywrite(fdl,"2 attempt to run the server !! \n");
				return 0 ;
			}
			sa.sa_handler = &handler;
			sa.sa_flags = 0;
			sigemptyset( &sa.sa_mask );
			sigaction( SIGINT, &sa , NULL );
			////////////////////////////////////////////////////////////////////////////////////////////////
			print_parameter(fdl,graphfile,logfile,PORT , S ,X);
			
			server(graphfile,fdl,PORT,S,X);
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			close(fdl);
			sem_close(_semaphore);			
			sem_unlink(semaphore_name);
			return 0 ;

		} 
		default : exit(0);
		
	}	
	return 0 ;
}
int server(char* infile , int logfile , int PORT , int S , int X){
	double time_taken ;
	struct sockaddr_in addr;
	struct sockaddr_in Caddr;
	pthread_t resizer ;
	socklen_t C_LEN;
	char print_buff[256];
	struct timeval  tv1, tv2;
	///////////////////////////////////////////////////////////////////
	write(logfile,"Loading graph...\n",17);
	gettimeofday(&tv1, NULL);
	g = load_graph(infile);
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec);
	sprintf(print_buff,"Graph loaded in %.2f seconds with %d nodes and, %d edges.\n",time_taken,g.V,g.E);
	write(logfile,print_buff,strlen(print_buff));
	
	/////////////////////////////////////////////////////////////////
	//////////////////// create  the thread pool ////////////////////	
	cache = create_cache();
	pool_size = S ;
	_pool = (pthread_t*)malloc(sizeof(pthread_t)*pool_size);
	thread_info.size = 0 ;
	thread_info.pointers = (struct thread_arg**)malloc(sizeof(struct thread_arg*)*pool_size);
	
	int counter = 0 ;
	while(counter < pool_size){
		struct thread_arg *temp_info = thread_info.head;
		int i = 0 ;
		if(counter == 0){
			thread_info.head = malloc(sizeof(struct thread_arg));
			thread_info.head->id = counter ;
			thread_info.head->fd = -1 ;
			thread_info.head->logfd = logfile ;
			thread_info.head->state = -1 ;
			thread_info.head->next = NULL ;
			thread_info.pointers[0] = thread_info.head;

			if(pthread_cond_init(&thread_info.pointers[0]->cond_var,NULL)!= 0){
				mywrite(logfile,"init cond ERROR");
				interupt =1 ;
			}
		}else{
			while(temp_info->next != NULL){i++;temp_info = temp_info->next;}
			temp_info->next = malloc(sizeof(struct thread_arg));
			temp_info = temp_info->next ;
			temp_info->id = counter ;
			temp_info->fd = -1 ;
			temp_info->logfd = logfile ;
			temp_info->state = -1 ;
			temp_info->next = NULL ;
			thread_info.pointers[counter] = temp_info;
			if(pthread_cond_init(&thread_info.pointers[counter]->cond_var,NULL)!= 0){
				mywrite(logfile,"init cond ERROR");
				interupt =1 ;
			}
		}
		pthread_create(&_pool[counter],NULL,&fthread,(void*)(long)counter);
		counter++;
	}
	
	pthread_create(&resizer,NULL,&resizer_thread,(void*)(long)logfile);
	sprintf(print_buff,"A pool of %d threads has been created\n",S);
	write(logfile,print_buff,strlen(print_buff));
	/////////////////////////////////////////////////////////////////

	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1 ){
		mywrite(logfile,"ERROR socket()\n");
		/////////////////////////////////////////////////////	
	}
	addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
    addr.sin_port = htons(PORT); 

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      mywrite(logfile,"binding ERROR !! \n");
      /////////////////////////////////////////////////////////
    }
    if(listen(sfd,100) != 0 ){
    	mywrite(logfile,"listen ERROR !! \n");
    }
    min = S ;
    max = X ;
    int CFD ;
    do{
    	
    	pthread_mutex_lock( &server_mutex );
    	while(number_of_occupied_thread >= (double)(min*0.75) && !interupt && min != max ){
    		help = 1 ;
    		pthread_cond_broadcast(&resizer_cond);
    		pthread_cond_wait( &server_cond , &server_mutex );
    	}
    	if(number_of_occupied_thread == max && !interupt){
    		mywrite(logfile,"No thread is available! Waiting for one.\n");
    		while(number_of_occupied_thread >= max && !interupt){
    			pthread_cond_wait( &server_cond , &server_mutex );		
    		}
    	} 
    	if(interupt){
    		pthread_mutex_unlock( &server_mutex );
    		break;
    	}   	
    	pthread_mutex_unlock( &server_mutex );
    	C_LEN = sizeof( struct sockaddr_in);
    	CFD = accept(sfd, (struct sockaddr *)&Caddr, &C_LEN);
    	pthread_mutex_lock( &server_mutex );
    	
    	if(CFD == -1){
    		if(errno == EINTR){
    			pthread_mutex_unlock( &server_mutex );
    			break;
    		}else{
    			mywrite(logfile,"ERROR accept()\n");
    		}
    		pthread_mutex_unlock( &server_mutex );
    	}else{
    		///////////////////////////////////////////////////////////
    		for(int i = 0 ; i < min ; i++){
	    	
	    		if(thread_info.pointers[i]->state == -1 ){
	    			thread_info.pointers[i]->fd = CFD;
	    			thread_info.pointers[i]->state = 1;
	    			
	    			
	    			sprintf(print_buff,"A connection has been delegated to thread id #%d , system load %.2f%c \n",i,(100.0*(double)number_of_occupied_thread/(double)min),37);
					write(logfile,print_buff,strlen(print_buff));
	    			number_of_occupied_thread++;
	    			pthread_cond_broadcast( &thread_info.pointers[i]->cond_var );
	    			pthread_mutex_unlock( &server_mutex );
	    			
	    			break;
	    		}
    		
    		}

    		///////////////////////////////////////////////////////////	
    	}
    	
    }while(!interupt);
    mywrite(logfile,"Termination signal received, waiting for ongoing threads to complete.\n");
	/////////////////////////////////////////////////////////////////
	for(int i = 0 ; i < pool_size ; i++){
		pthread_join(_pool[i],NULL);
	}
	pthread_join(resizer,NULL);
	
	mywrite(logfile,"All threads have terminated, server shutting down.\n");
	close(sfd);
	free(_pool);
	free_cache(&cache);
	//free(thread_info);
	for(int i = 0 ; i < pool_size ; i++){
		free(thread_info.pointers[i]);
	}
	free(thread_info.pointers);
	free_graph(&g);
	return 0 ;
}

void* fthread(void* arg){
	const int thread_id = (long)arg;
	char id[10];
	sprintf(id,"%d",thread_id);	
	int readbyte ,r;
	char ch ;
	char buffer[256];
	int src,dest;
	char print_buff[256];

	while(!interupt){
		sprintf(print_buff,"Thread #%d: waiting for connection\n",thread_id);
		write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));
		pthread_mutex_lock( &server_mutex );
		while( thread_info.pointers[thread_id]->state == -1 && !interupt){
			pthread_cond_wait( &thread_info.pointers[thread_id]->cond_var , &server_mutex );
		}
		pthread_mutex_unlock( &server_mutex );
		
		if(interupt)break;
		
		//////////////////////////////////////////////////
		readbyte = 0;
		do{
			r = read(thread_info.pointers[thread_id]->fd , &ch , 1);
			if(r == -1){
				mywrite(thread_info.pointers[thread_id]->logfd,"read system call return -1\n");
			}else if(r == 0){
				mywrite(thread_info.pointers[thread_id]->logfd,"unexpected EOF \n");
			}else if(ch == '\n'){
				buffer[readbyte]= '\0' ;
				sscanf(buffer,"%d,%d",&src,&dest);
				sprintf(print_buff,"Thread #%d: searching database for a path from node %d to node %d\n",thread_id,src,dest);
				write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));
				///////////////////////////////////////////  reader   ///////////////////////////////////////////////////////
				pthread_mutex_lock( &cache_mutex );
				while((AW+WW) > 0 && !interupt){
					WR++;
					pthread_cond_wait(&oktoread,&cache_mutex);
					WR--;
				}
				AR++;
				pthread_mutex_unlock( &cache_mutex );
				if(interupt)break;
				struct mylinkedlist list = is_path_in_cache(cache,src,dest);
				
				pthread_mutex_lock( &cache_mutex );
				AR--;
				if(AR == 0 && WW > 0){
					pthread_cond_broadcast(&oktowrite);
				}else if(WR > 0 && WW == 0){
					pthread_cond_broadcast(&oktoread);
				}
				pthread_mutex_unlock( &cache_mutex );
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				if(list.size == 0){
					sprintf(print_buff,"Thread #%d: no path in database, calculating  %d -> %d\n",thread_id,src,dest);
					write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));
					////////////////////////////calculate the path //////////////////////////
					
					struct mylinkedlist list2 = bfs(&g,src,dest);
					if(list2.size == 0){
						sprintf(print_buff,"Thread #%d: path not possible from node  %d to %d\n",thread_id,src,dest);
						write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));
						
						if(write(thread_info.pointers[thread_id]->fd , "\n" , 1) != 1){
							mywrite(thread_info.pointers[thread_id]->logfd,"could not send respond to client write ERROR !!\n");
						}
					}else{
						char* path = convert_to_string(list2);
						sprintf(print_buff,"Thread #%d: path calculated: %s \n",thread_id,path);
						write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));

						sprintf(print_buff,"Thread #%d: responding to client and adding path to database \n",thread_id);
						write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));

						
						strcat(path,"\n");
						int len = strlen(path);
						if(write(thread_info.pointers[thread_id]->fd , path, len )!= len){
							mywrite(thread_info.pointers[thread_id]->logfd,"could not send respond to client write ERROR !!\n");
						}
						/////////////////////////////////// writer ///////////////////////////////////
						pthread_mutex_lock(&cache_mutex);
						while((AW+AR) > 0 && !interupt){
							WW++;
							pthread_cond_wait(&oktowrite,&cache_mutex);
							WW--;
						}
						AW++;
						pthread_mutex_unlock(&cache_mutex);
						if(interupt)break;
						struct mylinkedlist list3 = is_path_in_cache(cache,src,dest);
						if(list3.size == 0){
							add_path(&cache,list2);        //// writer
						}else{
							free_list(&list3);
						}
						pthread_mutex_lock(&cache_mutex);
						AW--;
						if(WW>0){
							pthread_cond_broadcast(&oktowrite);
						}else if(WR > 0){
							pthread_cond_broadcast(&oktoread);
						}
						pthread_mutex_unlock(&cache_mutex);
						
						//////////////////////////////////////////////////////////////////////////////
						free(path);
						free_list(&list2);
					}
					


					////////////////////////////////////////////////////////////////////////
				}else{
					char* path = convert_to_string(list);
					sprintf(print_buff,"Thread #%d: path found in database: %s \n",thread_id,path);
					write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));

					sprintf(print_buff,"Thread #%d: responding to client\n",thread_id);
					write(thread_info.pointers[thread_id]->logfd,print_buff,strlen(print_buff));

					strcat(path,"\n");
					int len = strlen(path);
					if(write(thread_info.pointers[thread_id]->fd , path , len) != len){
						mywrite(thread_info.pointers[thread_id]->logfd,"could not send respond to client write ERROR !!\n");
					}		
					free(path);
					free_list(&list);
				}
				break ;
			}else{
				buffer[readbyte]= ch ;
				readbyte++;
			}
		}while(ch != '\n');

		//sleep(1);
		//////////////////////////////////////////////////
		pthread_mutex_lock( &server_mutex );
		thread_info.pointers[thread_id]->state = -1;
		close(thread_info.pointers[thread_id]->fd);
		thread_info.pointers[thread_id]->fd = -1 ;
		number_of_occupied_thread--;
		pthread_cond_broadcast(&server_cond );
		pthread_mutex_unlock( &server_mutex );
	}
	return NULL ;
}

void* resizer_thread(void* arg){
	const int fd = (long)arg;
	char print_buff[256];

	while(!interupt){
		pthread_mutex_lock( &server_mutex );
		while(help == 0 && !interupt){
			pthread_cond_wait(&resizer_cond ,&server_mutex);
		}
		if(interupt){
			pthread_mutex_unlock( &server_mutex );
			return NULL;
		}
		if(min == max){
			help = 0;
			pthread_cond_broadcast(&server_cond );
			pthread_mutex_unlock( &server_mutex );
		}else{
			int temp = pool_size ;
			min += (double)min*0.25;
			if(min > max){
				min -= min-max ;
			}
			pool_size = min ;
			
			sprintf(print_buff,"System load 75%c  pool extended to %d threads \n",37,min);
			write(fd,print_buff,strlen(print_buff));
			_pool = (pthread_t*)realloc(_pool,sizeof(pthread_t)*pool_size);
			thread_info.pointers = (struct thread_arg**)realloc(thread_info.pointers ,pool_size * sizeof(struct thread_arg*));
			struct thread_arg* info = thread_info.head ;
			while(temp < pool_size){
				int i = 0 ;
				while(info->next){
					i++;
					info = info->next ;
				}
				info->next = (struct thread_arg*)malloc(sizeof(struct thread_arg));
				info = info->next ;
				info->id = temp ;
				info->fd = -1 ;
				info->logfd = fd ;
				info->state = -1 ;
				info->next = NULL ;
				thread_info.pointers[temp] = info;

				if(pthread_cond_init(&thread_info.pointers[temp]->cond_var,NULL)!= 0){
					mywrite(fd,"init cond ERROR");
					interupt =1 ;
				}


				pthread_create(&_pool[temp],NULL,&fthread,(void*)(long)temp);


				temp++;
			}
			



			help = 0 ;
			pthread_cond_broadcast(&server_cond );
			pthread_mutex_unlock( &server_mutex );
		}
	}
	
	return NULL;
}


void handler(int signo){
	
	if(signo == SIGINT){
		interupt = 1;	
		for(int i = 0 ; i< pool_size ; i++){
			pthread_cond_broadcast(&thread_info.pointers[i]->cond_var);
		}
		pthread_cond_broadcast(&resizer_cond);
		pthread_cond_broadcast(&server_cond);
		pthread_cond_broadcast(&oktowrite);
		pthread_cond_broadcast(&oktoread);
	}
	
}

int mywrite(int fd , char* buffer){
	return write(fd,buffer,strlen(buffer));
}

void print_parameter(int fdl,char* graphfile,char* logfile,int PORT , int S , int X){
	char temp[6];

	mywrite(fdl , "Executing with parameters:\n");
	mywrite(fdl , "-i\t");
	mywrite(fdl , graphfile);
	mywrite(fdl , "\n");
			
	mywrite(fdl , "-p\t");
	sprintf(temp,"%d",PORT);
	mywrite(fdl , temp);
	mywrite(fdl , "\n");
			
	mywrite(fdl , "-o\t");
	mywrite(fdl , logfile);
	mywrite(fdl , "\n");
			
	mywrite(fdl , "-s\t");
	sprintf(temp,"%d",S);
	mywrite(fdl , temp);
	mywrite(fdl , "\n");
			
	mywrite(fdl , "-x\t");
	sprintf(temp,"%d",X);
	mywrite(fdl , temp);
	mywrite(fdl , "\n");
}