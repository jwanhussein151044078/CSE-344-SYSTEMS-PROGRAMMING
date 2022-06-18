#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "utilities.h"

void handler(int num);
void* florists_thread(void* arg);
int init_mc(int size);
int destroy(int size);

int done = 0 ;
int cancel = 0 ;
int interrupt = 0 ;

struct FLORIST *florists ;
int number_of_florists = 0 ;

int finish = 0 ;
pthread_mutex_t S_barrier = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barrier_cond = PTHREAD_COND_INITIALIZER;

struct REQUEST_QUEUE *queue ;

pthread_mutex_t *mutex;
pthread_cond_t *full;
pthread_cond_t *empty;

struct sales *s;

int main(int argc, char *argv[]){
	char *fn ;
	int fd ;
	int thread_num = 0 ;
	pthread_t *threads ;
	struct REQUEST *req;
	int req_num = 0;
	int send_to ;
	

	if(parsing_commandline(argc,argv,&fn) == -1){
		printf("INVAILD COMMAND LINE ARGUMENT::\n");
		printf("expected :\n\t");
		printf("./program.exe -i filename \n");
		return -1;
	}
	
	fd = open(fn,O_RDONLY);
	if(fd == -1){
		printf("open file ERROR !!\n");
		return -1 ;
	}

	///////////////////////////////////// handle SIGINT /////////////////////////////////////////
	signal(SIGINT,handler);
	/////////////////////////////////////////////////////////////////////////////////////////////
	srand(time(0));
	///////////////////////////////////// get the florists //////////////////////////////////////
	florists = get_florists(fd,&number_of_florists);
	if(init_mc(number_of_florists) == -1){
		for (int i = 0; i < number_of_florists ; ++i){
			if(florists[i].kinds > 0){
			free(florists[i].flowers);
			}
		}
		free(florists);
		close(fd);
	}
	///////////////////////////////////////// create the queue  /////////////////////////////////
	
	queue = malloc(sizeof(struct REQUEST_QUEUE)*number_of_florists);
	for (int i = 0; i < number_of_florists; ++i){
		queue[i] = get_QUEUE();
	}
	s = (struct sales*)malloc(sizeof(struct sales)*number_of_florists);
	///////////////////////////////////// create florists threads ///////////////////////////////
	threads = (pthread_t*)malloc(sizeof(pthread_t)*number_of_florists);
	printf("Florist application initializing from file: %s\n",fn );
	
	while(thread_num < number_of_florists ){
		pthread_create(&threads[thread_num],NULL,&florists_thread,(void*)(long)thread_num);
		thread_num++;
	}
	printf("%d florists have been created\n",number_of_florists );
	printf("Processing requests \n");

	///////////////////////// reads the requests and sending to the proper saller ///////////////

	while(!done){
		req = get_REQUEST(fd);
		if(req == NULL){
			done = 1 ;
		}else{
			send_to = get_closest_florist(florists,req,number_of_florists);
			if(send_to == -1){
				printf("unknowun kind of flower (%s) was requested by %s the request was ignored\n",req->flower,req->name );
			}
			else{
				pthread_mutex_lock( &mutex[send_to] );
				while(queue[send_to].used >= QUEUE_SIZE && !interrupt){
					pthread_cond_wait( &empty[send_to] , &mutex[send_to] );
				}
				if(interrupt){
					pthread_mutex_unlock( &mutex[send_to] );
					free(req);
					break ;
				}
				queue[send_to].used++;
				queue[send_to].req[queue[send_to].pointer_Producer%QUEUE_SIZE] = push_REQUEST( *req , florists[send_to].pos );
				queue[send_to].pointer_Producer++; 
				pthread_cond_broadcast( &full[send_to] );
				pthread_mutex_unlock( &mutex[send_to] );			
				req_num++;
			}
			free(req);
		}		
	}	
	///////////////////////////////////// in case of an interrupt ///////////////////////////////
	if(interrupt){
		printf("interrupted !!!!\n");
		thread_num = 0 ;
		while(thread_num < number_of_florists){
			pthread_join(threads[thread_num],NULL);
			thread_num++;
		}

		for (int i = 0; i < number_of_florists ; ++i){
			if(florists[i].kinds > 0)
				free(florists[i].flowers);
		}
		destroy(number_of_florists);
		free(florists);
		free(threads);
		free(queue);
		free(s);
		close(fd);
		return 0 ;
	}
	/////////////////////////////////  wait till all thread finish //////////////////////////////
	pthread_mutex_lock(&S_barrier);
	while(get_req_processed(s,number_of_florists) < req_num && !interrupt){
		pthread_cond_wait(&barrier_cond,&S_barrier);
	}
	cancel = 1 ;
	pthread_cond_broadcast(&barrier_cond);
	for (int i = 0; i < number_of_florists; ++i){
		pthread_cond_broadcast(&full[i]);
	}
	pthread_mutex_unlock(&S_barrier);
	////////////////////////////////////  join all the threads  /////////////////////////////////	
	for(int i = 0 ; i < number_of_florists ; i++){
		pthread_join(threads[i],NULL);
	}
	thread_num = 0 ;
	printf("\nSale statistics for today: \n");
	printf("-------------------------------------------------\n");
	printf("Florist           # of sales           Total time\n");	
	printf("-------------------------------------------------\n");
	while(thread_num < number_of_florists && !interrupt){
		printf("%s\t%20d\t%15fms\n",florists[thread_num].name,s[thread_num].totalsale,s[thread_num].totaltime );
		thread_num++;
	}
	printf("-------------------------------------------------\n");

	////////////////////////////////////  free   ///////////////////////////////////////////
	for (int i = 0; i < number_of_florists ; ++i){
		if(florists[i].kinds > 0){
			free(florists[i].flowers);
		}
	}
	free(florists);
	destroy(number_of_florists);
	free(threads);
	free(queue);
	free(s);
	close(fd);
	return 0;
}

void* florists_thread(void* arg){
	const int id = (long)arg;
	double ms ;
	s[id].totalsale = 0 ;
	s[id].totaltime = 0 ;
	
	while(1){
		pthread_mutex_lock(&mutex[id]);
		while(!interrupt && queue[id].used <= 0 && !cancel){
			pthread_cond_wait(&full[id],&mutex[id]);
			if(interrupt || cancel){
				break;
			}
			
		}
		if(interrupt){
			printf("%s was interrupted !!\n",florists[id].name );
			pthread_mutex_unlock(&mutex[id]);
			break;						
		}
		if(cancel){
			pthread_mutex_unlock(&mutex[id]);
			break;	
		}
		queue[id].used--;
		ms = (rand()%250)+1;
		ms += queue[id].req[queue[id].pointer_Consumer%QUEUE_SIZE].dis/florists[id].speed;
		s[id].totaltime += ms;
		s[id].totalsale += 1;
		pthread_cond_broadcast(&empty[id]);
		pthread_mutex_unlock(&mutex[id]);		
		usleep(ms*1000);
		
		printf("florist %s has has delivered a %s to %s in %fms \n",florists[id].name,queue[id].req[(queue[id].pointer_Consumer)%QUEUE_SIZE].flower, 
																	queue[id].req[(queue[id].pointer_Consumer)%QUEUE_SIZE].name,ms);	
		
		pthread_mutex_lock(&S_barrier);
		queue[id].pointer_Consumer++;
		pthread_cond_broadcast(&barrier_cond);
		pthread_mutex_unlock(&S_barrier);

	}
	pthread_mutex_lock(&S_barrier);
	finish++;
	if(finish < number_of_florists && !interrupt){
		pthread_cond_wait(&barrier_cond,&S_barrier);
		printf("%s closing shop . \n",florists[id].name );
	}else if(!interrupt){
		printf("All requests processed .\n\n");
		pthread_cond_broadcast(&barrier_cond);
		printf("%s closing shop . \n",florists[id].name );
	}else{
		pthread_cond_broadcast(&barrier_cond);	
	}
	pthread_mutex_unlock(&S_barrier);
	return NULL ;
	
}

void handler(int num){
	if(num == SIGINT){
		done =1 ;
		interrupt = 1;
		cancel =1 ;
		for(int i = 0 ; i < number_of_florists ; i++){
			pthread_cond_broadcast( &full[i] );
			pthread_cond_broadcast( &empty[i] );
		}
		pthread_cond_broadcast(&barrier_cond);
	}
}

int init_mc(int size){
	int flag = 0 ;
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*size);
	full = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)*size);
	empty = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)*size);
	for(int i = 0 ; i < size ; i++){
		if(pthread_mutex_init(&mutex[i],NULL)!= 0){
			flag = 1 ;break;
		}
		if(pthread_cond_init(&full[i],NULL)!= 0){
			flag = 1 ;break;
		}
		if(pthread_cond_init(&empty[i],NULL)!= 0){
			flag = 1 ;break;
		}
	}
	if(flag){
		free(mutex);
		free(empty);
		free(full);
		return -1 ;
	}
	return 1 ;
}

int destroy(int size){
	int flag = 0 ;
	for (int i = 0; i < size; ++i){
		if(pthread_mutex_destroy(&mutex[i]) != 0){
			printf("destroing mutex was failed !!\n");
			flag = 1;
		}	
		if(pthread_cond_destroy(&full[i]) != 0){
			printf("destroing full cond was failed !!\n");
			flag = 1;
		}
		if(pthread_cond_destroy(&empty[i]) != 0){
			printf("destroing empty cond was failed !!\n");
			flag = 1;
		}
	}
	free(mutex);
	free(full);
	free(empty);
	if(flag){
		return -1;
	}
	return 1 ;	
}