# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/sem.h>
# include <pthread.h>
# include <semaphore.h>
# include <string.h>
# include <stdlib.h>

# define SLEEP_T 1

int parsing_commandline(int argc, char *argv[] , char** fn);
int fvalid(int fd);
int lvalid(char str[3]);
void* chefs(void* arg);
void* helper(void *arg);

sem_t mutex1 ; /* the helper thread wait for, and posted by wholesaler when new line is read */
sem_t mutex2 ; /* the wholesaler thread wait for , posted by chef when dessert is delivered  */
sem_t mutexMF; /* the chef who has endless supply of ( W S ) wait for , posted by helper when line read by wholesaler is MF (chef[5] wait for) */
sem_t mutexMS; /* the chef who has endless supply of ( W F ) wait for , posted by helper when line read by wholesaler is MS (chef[4] wait for) */
sem_t mutexMW; /* the chef who has endless supply of ( S F ) wait for , posted by helper when line read by wholesaler is MW (chef[3] wait for) */
sem_t mutexFS; /* the chef who has endless supply of ( W M ) wait for , posted by helper when line read by wholesaler is FS (chef[2] wait for) */
sem_t mutexFW; /* the chef who has endless supply of ( S M ) wait for , posted by helper when line read by wholesaler is FW (chef[1] wait for) */
sem_t mutexSW; /* the chef who has endless supply of ( M F ) wait for , posted by helper when line read by wholesaler is SW (chef[0] wait for) */
sem_t F;       /* flour mutex */
sem_t M;	   /* milk mutex */
sem_t S;	   /* sugar mutex*/
sem_t W;       /* walnuts mutex */

int main(int argc, char *argv[]){
	char* filename ;
	char *ing1 , *ing2 ;
	int fd ;
	int lines ;
	int i , r;
	char dummy ;
	char *ingredients ;
	pthread_t phelper  ;    /* helper thread */
	pthread_t chefsth[6] ;  /* chefs threads chef[0] endless supply of M F
							    			 chef[1] endless supply of M S
							    			 chef[2] endless supply of M W
							    			 chef[3] endless supply of F S
							    			 chef[4] endless supply of F W
							    			 chef[5] endless supply of W S */ 



	if(parsing_commandline(argc,argv,&filename)==-1){
		printf("INVAILD COMMAND LINE ARGUMENT::\n");
		printf("expected :\n\t");
		printf("./program.exe -i filename \n");
		return -1;
	}
	if((fd = open(filename,O_RDONLY)) == -1){
		perror("open file ERROR :");
		return -1 ;
	}
	if((lines = fvalid(fd)) == -1){
		close(fd);
		return -1 ;
	}

	if(sem_init(&mutex1 ,0,0) == -1 || sem_init(&mutex2 ,0,0) == -1 || sem_init(&mutexMF,0,0) == -1 || sem_init(&mutexMS,0,0) == -1 || 
	   sem_init(&mutexMW,0,0) == -1 || sem_init(&mutexFS,0,0) == -1 || sem_init(&mutexFW,0,0) == -1 || sem_init(&mutexSW,0,0) == -1 ||
	   sem_init(&F,0,0) == -1 || sem_init(&M,0,0) == -1 || sem_init(&S,0,0) == -1 || sem_init(&W,0,0) == -1){
		perror("sem_init ERROR :");
		close(fd);
		return -1 ;
	}
	ingredients = malloc(sizeof(char)*2);
	
	if(ingredients == NULL){
		close(fd);
		return -1 ;
	}

	lseek(fd,0L,SEEK_SET);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( pthread_create(&phelper , NULL , helper , ingredients) != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[0] , NULL , chefs , "0") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[1] , NULL , chefs , "1") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[2] , NULL , chefs , "2") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[3] , NULL , chefs , "3") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[4] , NULL , chefs , "4") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	if( pthread_create(&chefsth[5] , NULL , chefs , "5") != 0 ){
		printf("ERROR creating a thread\n");
		close(fd);
		return -1 ;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for( i = 0 ; i < lines ; i++){
		if((r = read(fd , ingredients , 2)) != 2){
			if(r == -1){
				printf("read ERROR !!\n");
				for(int i = 0 ; i < 6 ; i++){
					pthread_cancel(chefsth[i]);
				}
				close(fd);
				return -1 ;
			}
			break ;
		}
		if(ingredients[0] == 'F' || ingredients[1] == 'F'){
			sem_post(&F);
			if(ing1 == NULL){
				ing1 = "flour";
			}else{
				ing2 = "flour";
			}
		}
		if(ingredients[0] == 'M' || ingredients[1] == 'M'){
			sem_post(&M);
			if(ing1 == NULL){
				ing1 = "milk";
			}else{
				ing2 = "milk";
			}
		}
		if(ingredients[0] == 'S' || ingredients[1] == 'S'){
			sem_post(&S);
			if(ing1 == NULL){
				ing1 = "sugar";
			}else{
				ing2 = "sugar";
			}
		}
		if(ingredients[0] == 'W' || ingredients[1] == 'W'){
			sem_post(&W);
			if(ing1 == NULL){
				ing1 = "walnuts";
			}else{
				ing2 = "walnuts";
			}
		}	
		printf("the wholesaler delivers %s and %s\n",ing1 , ing2 );
		ing1 = NULL ;
		ing2 = NULL ;
		if((r = read(fd , &dummy , 1))!= 1){
			if(r == -1){
				printf("read ERROR !!\n");
				free(ingredients);
				close(fd);
				return -1 ;

			}
		}
		sem_post(&mutex1);
		printf("the wholesaler is waiting for the dessert\n");
		sem_wait(&mutex2);
		printf("the wholesaler has obtained the dessert and left to sell it\n");
		
	}
	pthread_cancel(phelper);
	
	for(int i = 0 ; i < 6 ; i++){
		pthread_cancel(chefsth[i]);
	}
	
	free(ingredients);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	if(pthread_join(phelper,NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	
	if(pthread_join(chefsth[0],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	if(pthread_join(chefsth[1],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	if(pthread_join(chefsth[2],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	if(pthread_join(chefsth[3],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	if(pthread_join(chefsth[4],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	if(pthread_join(chefsth[5],NULL) != 0){
		printf("ERROR join thread\n");
		close(fd);
		return -1 ;	
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(close(fd) == -1){
		printf("ERROR close file\n");
		return -1 ;
	}
	if(sem_destroy(&mutex1 ) == -1 || sem_destroy(&mutex2 ) == -1 || sem_destroy(&mutexMF) == -1 || sem_destroy(&mutexMS) == -1 || 
	   sem_destroy(&mutexMW) == -1 || sem_destroy(&mutexFS) == -1 || sem_destroy(&mutexFW) == -1 || sem_destroy(&mutexSW) == -1 ||
	   sem_destroy(&F) == -1 || sem_destroy(&M) == -1 || sem_destroy(&S) == -1 || sem_destroy(&W) == -1 ){
		perror("sem_destroy ERROR :");
		return -1 ;
	}

	return 0 ;
}

void* chefs(void* arg){
	int i = ((char*)arg)[0]-48;
	while(1){
		if(i == 0){
			printf("chef0 is waiting for sugar and walnuts\n");
			sem_wait(&mutexSW);
			sem_wait(&S);
			printf("chef0 has taken the sugar\n");
			sem_wait(&W);
			printf("chef0 has taken the walnuts\n");
		}else if(i == 1){
			printf("chef1 is waiting for flour and walnuts\n");
			sem_wait(&mutexFW);
			sem_wait(&F);
			printf("chef1 has taken the flour\n");
			sem_wait(&W);
			printf("chef1 has taken the walnuts\n");
		}else if(i == 2){
			printf("chef2 is waiting for flour and sugar\n");
			sem_wait(&mutexFS);
			sem_wait(&F);
			printf("chef2 has taken the flour\n");
			sem_wait(&S);
			printf("chef2 has taken the sugar\n");
		}else if(i == 3){
			printf("chef3 is waiting for milk and walnuts\n");
			sem_wait(&mutexMW);
			sem_wait(&M);
			printf("chef3 has taken the milk\n");
			sem_wait(&W);
			printf("chef3 has taken the walnuts\n");
		}else if(i == 4){
			printf("chef4 is waiting for milk and sugar\n");
			sem_wait(&mutexMS);
			sem_wait(&M);
			printf("chef4 has taken the milk\n");
			sem_wait(&S);
			printf("chef4 has taken the sugar\n");
		}else if(i == 5){
			printf("chef5 is waiting for milk and flour\n");
			sem_wait(&mutexMF);
			sem_wait(&M);
			printf("chef5 has taken the milk\n");
			sem_wait(&F);
			printf("chef5 has taken the flour\n");
		}
		printf("chef%d is preparing the dessert\n",i );
		sleep(SLEEP_T);
		printf("chef%d has delivered the dessert to the wholesaler\n",i );
		sem_post(&mutex2);
	}
}

void* helper(void *arg){

	while(1){
		sem_wait(&mutex1);
		
		if(strcmp((char*)arg,"MF") == 0 || strcmp((char*)arg,"FM") == 0){
			sem_post(&mutexMF);
		}
		else if(strcmp((char*)arg,"MS") == 0 || strcmp((char*)arg,"SM") == 0){
			sem_post(&mutexMS);
		}
		else if(strcmp((char*)arg,"WS") == 0 || strcmp((char*)arg,"SW") == 0){
			sem_post(&mutexSW);
		}
		else if(strcmp((char*)arg,"WM") == 0 || strcmp((char*)arg,"MW") == 0){
			sem_post(&mutexMW);
		}
		else if(strcmp((char*)arg,"FS") == 0 || strcmp((char*)arg,"SF") == 0){
			sem_post(&mutexFS);
		}
		else if(strcmp((char*)arg,"WF") == 0 || strcmp((char*)arg,"FW") == 0){
			sem_post(&mutexFW);
		}
	}
}

int fvalid(int fd){
	int lines = 0 ;
	int i ;
	char temp[3] ;
	
	while(1){
		i = read(fd , temp , 3);
		if(i == 3){
			if(lvalid(temp) == 1){
				lines++;
			}
			else{
				printf("invalid line %d .\n",lines+1);
				return -1;
			}
		}
		else if(i == 2){
			if(lvalid(temp) == 1){
				lines++;
				if (lines < 10 ){
					printf("the file contains less than 10 lines (%d lines in file)\n", lines);
					return -1 ;
				}
				return lines;
			}
			else{
				printf("invalid line %d .\n",lines+1);
				return -1;
			}
		}
		else if( i == 0 ){
			if (lines < 10 ){
				printf("the file contains less than 10 lines (%d lines in file)\n", lines);
				return -1 ;
			}
			return lines;
		}
		else if( i == -1 ){
			printf("read ERROR !!!\n");
			return -1; 
		}
		else if(i == 1){
			printf("invalid line %d\n",lines);
			return -1 ;
		}
	}
}


int lvalid(char str[3]){
	if(str[2] == EOF || str[2] == '\n'){
		if(str[1] != str[0]){
			if((str[0] == 'F' || str[0] == 'M' || str[0] == 'S' || str[0] == 'W') && 
			   (str[1] == 'F' || str[1] == 'M' || str[1] == 'S' || str[1] == 'W') ){
				return 1 ;
			}
		}
	}	
	return 0 ;
}

int parsing_commandline(int argc, char *argv[] , char** fn){
	int opt;

	while((opt = getopt(argc, argv, ":i:")) != -1){	
        	
    	if(opt == 'i' && optind == 3){
    		*fn = optarg ;
    	}else{
     		return -1 ;
    	}   
    }        
    if(optind < 3){
    	return -1;
    }
    
    return 0 ;
}