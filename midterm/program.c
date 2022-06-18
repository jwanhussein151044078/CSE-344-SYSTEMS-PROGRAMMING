#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "utilities.h"
#include "actors.h"



int main(int argc, char *argv[]){

	int N,M,T,S,L,K,U,G;
	char* filename ;
	int fd ;
	int input ;
	char* addr;
	struct shm *myshared ;

	if(parsing_commandline(argc,argv,&N,&T,&S,&L,&U,&G,&filename)==-1){
		printf("INVAILD COMMAND LINE ARGUMENT::\n");
		printf("expected :\n\t");
		printf("./program.exe -N num -M num -T num -S num -L num \n");
		return -1;
	}
	M = U + G ;
	if(check_constraints(N,M,T,S,L)==-1){
		printf("constraints imposed \n");
		printf("M > N > 2\n");
		printf("S > 3\n");
		printf("M > T >= 1\n");
		printf("L >= 3\n");
		return -1 ;
	}
	K = (2*L*M)+1 ;	
	/* open a shared mem and set its size*/
	fd = shm_open(shmname , O_CREAT | O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return -1 ;
	}
	if(ftruncate(fd, sizeof(struct shm)) == -1){  /* resize the object to size of int * 10 */
		perror("ftruncate ERROR :");
		shm_unlink(shmname);
		return -1 ;
	}
	if((input = open(filename,O_RDONLY)) == -1){
		perror("open file ERROR :");
		shm_unlink(shmname);
		return -1 ;	
	}
	if(isfilevalide(input,3*L*M) == -1){
		shm_unlink(shmname);
		close(input);
		return -1 ;
	}
	myshared =(struct shm*) mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);

	/* initializing the counters */
	myshared->KP = 0 ;
	myshared->KC = 0 ;
	myshared->KD = 0 ;
	myshared->CP = 0 ;
	myshared->CC = 0 ;
	myshared->CD = 0 ;
	myshared->KSP = 0 ;
	myshared->KSC = 0 ;
	myshared->KSD = 0 ;
	myshared->total_served_P = 0 ;
	myshared->total_served_C = 0 ;
	myshared->total_served_D = 0 ;
	myshared->tables = T ;
	myshared->Ustudentq = 0 ;
	myshared->Gstudentq = 0 ;

	/* initializing the semaphores */
	if(sem_init(&myshared->Kempty ,1,K) == -1 || sem_init(&myshared->Cempty ,1,S) == -1 || sem_init(&myshared->Tempty ,1,T) == -1 ||
	   sem_init(&myshared->Kmutex ,1,1) == -1 || sem_init(&myshared->Cmutex ,1,1) == -1 || sem_init(&myshared->Tmutex ,1,1) == -1 || 
	   sem_init(&myshared->Ssemaph,1,0) == -1 || sem_init(&myshared->cook1  ,1,0) == -1 || sem_init(&myshared->TSmutex,1,1) == -1 || 
	   sem_init(&myshared->SU     ,1,0) == -1 || sem_init(&myshared->C1mutex,1,0) == -1 || sem_init(&myshared->C2mutex,1,0) == -1 || 
	   sem_init(&myshared->C3mutex,1,1) == -1 || sem_init(&myshared->SQ     ,1,1) == -1 || sem_init(&myshared->SG     ,1,0) == -1 ||
	   sem_init(&myshared->cont   ,1,0) == -1 || sem_init(&myshared->SQsem  ,1,0) == -1 ){
		perror("sem_init ERROR :");
		shm_unlink(shmname);
		return -1 ;
	}	

	if(fork()==0){
		suppler(input,L*M*3);
		printf("The supplier finished supplying – GOODBYE!!\n");fflush(stderr);
		return 0;
	}
	
	for (int i = 0; i < N; ++i){
		if(fork()==0){
			cook(i,L*M);			
			return 0 ;
		}
	}
	if(fork()==0){
		Queue_organizer(M,L);
		return 0;
	}

	for (int i = 0; i < U; ++i){
		if(fork()==0){
			Ustudent(i,L);
			return 0 ;
		}
	}
	for (int i = 0; i < G; ++i){
		if(fork()==0){
			Gstudent(i,L);
			return 0 ;
		}
	}

	
	for(int i = 0 ; i < M+N+2; i++){
		wait(NULL);
	}

	if(sem_destroy(&myshared->Kempty ) == -1 || sem_destroy(&myshared->Cempty ) == -1 || sem_destroy(&myshared->Tempty ) == -1 ||
	   sem_destroy(&myshared->Kmutex ) == -1 || sem_destroy(&myshared->Cmutex ) == -1 || sem_destroy(&myshared->Tmutex ) == -1 || 
	   sem_destroy(&myshared->Ssemaph) == -1 || sem_destroy(&myshared->cook1  ) == -1 || sem_destroy(&myshared->TSmutex) == -1 || 
	   sem_destroy(&myshared->SU     ) == -1 || sem_destroy(&myshared->C1mutex) == -1 || sem_destroy(&myshared->C2mutex) == -1 || 
	   sem_destroy(&myshared->C3mutex) == -1 || sem_destroy(&myshared->SQ     ) == -1 || sem_destroy(&myshared->SG     ) == -1 ||
	   sem_destroy(&myshared->cont   ) == -1 || sem_destroy(&myshared->SQsem  ) == -1 ){
		perror("sem_destroy ERROR :");
		shm_unlink(shmname);
		return -1 ;
	}	
	if(close(input)==-1){
		perror("close file ERROR :");
	}
	if(shm_unlink(shmname) ==-1){
		perror("close shared mem object ERROR :");
		return -1 ;
	}
}