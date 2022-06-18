

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>



#include "utilities.h"
#include "actors.h"

int suppler(int filed,int lm3){
	int fd ;
	struct shm *myshared ;
	int counter = 0 ;
	int detector = 0 , temp = 0 ,r;
	char ch;
	
	fd = shm_open(shmname , O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return -1 ;
	}

	myshared = (struct shm*)mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);
	
	while(1){
		
		sem_wait(&myshared->Kempty);
		sem_wait(&myshared->Kmutex);
		if((r = read(filed,&ch,1)) == 0){
			sem_post(&myshared->Kmutex);			
			return 0 ;
		}
		if(r == -1 ){
			perror("ERROR read file :");
			return -1 ;
		}
		if(ch == 'P'){
			printf("The supplier going to the kitchen to deliver soup :                           kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
			myshared->KP++;
			myshared->KSP++;
			printf("The supplier delivered soup – after delivery:                                 kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);		
		}else if( ch == 'C'){
			printf("The supplier going to the kitchen to deliver main course :                    kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
			myshared->KC++;
			myshared->KSC++;
			printf("The supplier delivered main course – after delivery:                          kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
		}else if( ch == 'D'){
			printf("The supplier going to the kitchen to deliver dessert :                        kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
			myshared->KD++;
			myshared->KSD++;
			printf("The supplier delivered dessert – after delivery:                              kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
								myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
		
		}else{
			printf("unknown kind of meals was found ( %c ) byte number ( %d ) in input file !!!\n",ch,counter+1 );
			return -1 ;
		}
		temp = minimum(myshared->KSP , myshared->KSC , myshared->KSD );
		if(temp != detector){
			sem_post(&myshared->cook1);
			detector = temp ;
		}
		sem_post(&myshared->Kmutex);
		counter++ ;
	}


	
	return 0 ;	
}

int cook(int id , int lm){
	
	int fd ;
	struct shm *myshared ;

	fd = shm_open(shmname , O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return -1 ;
	}
	myshared = (struct shm*)mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);
	
	while(1){
		sem_wait(&myshared->TSmutex);
		if((id%3) == 0){
			if(myshared->total_served_P == lm){
				sem_post(&myshared->TSmutex);
				printf("Cook %d finished serving - items at kitchen: 0 – going home – GOODBYE!!!\n",id );fflush(stderr);
				return 0 ;
			}
			else{
				
				myshared->total_served_P++ ;
				sem_post(&myshared->TSmutex);
				sem_wait(&myshared->cook1);
				sem_wait(&myshared->C3mutex);
			}
		}else if((id%3) == 1){
			if(myshared->total_served_C == lm){
				sem_post(&myshared->TSmutex);
				printf("Cook %d finished serving - items at kitchen: 0 – going home – GOODBYE!!!\n",id );fflush(stderr);
				return 0 ;
			}
			else{
				myshared->total_served_C++ ;
				sem_post(&myshared->TSmutex);
				sem_wait(&myshared->C1mutex);				
			}
		}else{
			if(myshared->total_served_D == lm){
				sem_post(&myshared->TSmutex);
				printf("Cook %d finished serving - items at kitchen: 0 – going home – GOODBYE!!!\n",id );fflush(stderr);
				return 0 ;
			}
			else{
				myshared->total_served_D++ ;
				sem_post(&myshared->TSmutex);
				sem_wait(&myshared->C2mutex);
			}
		}
		sem_wait(&myshared->Kmutex );
		printf("cook %d is going to the kitchen to wait for/get a plate                      - kitchen items  P: %3d  C: %3d  D: %3d  = %3d\n",
										id, myshared->KP,myshared->KC,myshared->KD,myshared->KP+myshared->KC+myshared->KD);fflush(stderr);
		sem_post(&myshared->Kmutex );
		sem_wait(&myshared->Kmutex );
		
		if(id%3 == 0){
			myshared->KP--;		
		}else if(id%3 == 1){
			myshared->KC--;
		}else{
			myshared->KD--;
		}
		sem_post(&myshared->Kempty);
		sem_post(&myshared->Kmutex);
		sem_wait(&myshared->Cempty);
		sem_wait(&myshared->Cmutex);
		if(id%3 == 0){
			printf("cook %d is going to the counter to deliver soup                              - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD );fflush(stderr);
			myshared->CP++;
			sem_post(&myshared->C1mutex);
			printf("cook %d placed soup on the counter                                           - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD);fflush(stderr);		
		}else if(id%3 == 1){
			printf("cook %d is going to the counter to deliver main course                       - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD );fflush(stderr);
			myshared->CC++;
			sem_post(&myshared->C2mutex);
			printf("cook %d placed main course on the counter                                    - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD);fflush(stderr);
		}else{
			printf("cook %d is going to the counter to deliver dessert                           - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD );fflush(stderr);
			myshared->CD++;
			sem_post(&myshared->C3mutex);
			sem_post(&myshared->Ssemaph);
			printf("cook %d placed dessert on the counter                                        - counter items  P: %3d  C: %3d  D: %3d  = %3d\n",id,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD);fflush(stderr);
		}
		sem_post(&myshared->Cmutex);
		
	}
	return 0 ;
}

int Gstudent(int id , int l){
	
	int counter = 0;
	int fd ;
	struct shm *myshared ;


	fd = shm_open(shmname , O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return -1 ;
	}
	myshared = (struct shm*)mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);
	while(counter < l){
		sem_wait(&myshared->SQ);
		myshared->Gstudentq++;
		sem_post(&myshared->SQsem);
		sem_wait(&myshared->Cmutex );
		printf("Gstudent %d going to the counter (round %d) - # of student at counter :%3d and   counter items  P: %3d  C: %3d  D: %3d  = %3d\n"
										,id,counter+1,myshared->Ustudentq+myshared->Gstudentq,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD );fflush(stderr);
		sem_post(&myshared->Cmutex );
		sem_post(&myshared->SQ);
		sem_wait(&myshared->SG);
		sem_wait(&myshared->Cmutex );

		myshared->CP--;
		myshared->CC--;
		myshared->CD--;

		sem_post(&myshared->Cempty );
		sem_post(&myshared->Cempty );
		sem_post(&myshared->Cempty );
		sem_post(&myshared->cont);
		sem_post(&myshared->Cmutex );

		printf("Gstudent %d got food and is going to get a table (round %d) - # of empty tables:%3d \n",
										id , counter+1 , myshared->tables  );fflush(stderr);
		sem_wait(&myshared->Tempty );

		sem_wait(&myshared->Tmutex );		
		myshared->tables--;
		sem_post(&myshared->Tmutex );
		
		printf("Gstudent %d sat at table 1 to eat (round %d) - empty tables:                    %3d \n",
										id , counter+1,myshared->tables);fflush(stderr);
		
		sem_post(&myshared->Tempty );
		sem_wait(&myshared->Tmutex );		
		myshared->tables++;
		sem_post(&myshared->Tmutex );
		
		if(counter+1 < l){
			printf("Gstudent %d left table 1 to eat again (round %d) - empty tables :               %3d \n",
										id , counter+2 ,myshared->tables  );fflush(stderr);
		}else {
			printf("Gstudent %d is done eating L = %d times - going home - GOODBYE!!!\n",
										id , counter+1  );fflush(stderr);
			return 0 ;
		}
				
		counter++;
	}
	
	return 0 ;
}

int Ustudent(int id , int l){
	
	int counter = 0;
	int fd ;
	struct shm *myshared ;


	fd = shm_open(shmname , O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return -1 ;
	}
	myshared = (struct shm*)mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);
	while(counter < l){
		sem_wait(&myshared->SQ);
		myshared->Ustudentq++;
		sem_post(&myshared->SQsem);
		sem_wait(&myshared->Cmutex );
		printf("Ustudent %d going to the counter (round %d) - # of student at counter :%3d and   counter items  P: %3d  C: %3d  D: %3d  = %3d\n"
										,id,counter+1,myshared->Ustudentq+myshared->Gstudentq,myshared->CP,myshared->CC,myshared->CD,myshared->CP+myshared->CC+myshared->CD );fflush(stderr);
		sem_post(&myshared->Cmutex );
		sem_post(&myshared->SQ);
		sem_wait(&myshared->SU);		
		sem_wait(&myshared->Cmutex );

		myshared->CP--;
		myshared->CC--;
		myshared->CD--;

		sem_post(&myshared->Cempty );
		sem_post(&myshared->Cempty );
		sem_post(&myshared->Cempty );
		sem_post(&myshared->cont);
		sem_post(&myshared->Cmutex );

		printf("Ustudent %d got food and is going to get a table (round %d) - # of empty tables:%3d \n",
										id , counter+1 , myshared->tables  );fflush(stderr);
		sem_wait(&myshared->Tempty );
		sem_wait(&myshared->Tmutex );
		myshared->tables--;
		sem_post(&myshared->Tmutex );
		printf("Ustudent %d sat at table 1 to eat (round %d) - empty tables:                    %3d \n",
										id , counter+1,myshared->tables);fflush(stderr);
		sem_post(&myshared->Tempty );
		sem_wait(&myshared->Tmutex );
		myshared->tables++;
		sem_post(&myshared->Tmutex );
		if(counter+1 < l){
			printf("Ustudent %d left table 1 to eat again (round %d) - empty tables :               %3d \n",
										id , counter+2 ,myshared->tables  );fflush(stderr);
		}else {
			printf("Ustudent %d is done eating L = %d times - going home - GOODBYE!!!\n",
										id , counter+1  );fflush(stderr);
			return 0 ;
		}
				
		counter++;
	}
	
	return 0 ;
}

void Queue_organizer(int m , int l){
	int fd ;
	int counter = 0;
	int total = m*l ;
	struct shm *myshared ;


	fd = shm_open(shmname , O_RDWR , S_IRWXU );
	if(fd == -1){
		perror("open shared mem object ERROR :");
		return ;
	}
	myshared = (struct shm*)mmap(NULL , sizeof(struct shm) , PROT_READ | PROT_WRITE , MAP_SHARED , fd ,  0);
	
	while(counter < total){
		sem_wait(&myshared->SQsem);
		sem_wait(&myshared->Ssemaph);
		sem_wait(&myshared->SQ);
		if(myshared->Gstudentq > 0){
			myshared->Gstudentq--;
			sem_post(&myshared->SG);
			sem_post(&myshared->SQ);
			sem_wait(&myshared->cont);
		}else if (myshared->Ustudentq > 0){
			myshared->Ustudentq--;
			sem_post(&myshared->SU);
			sem_post(&myshared->SQ);
			sem_wait(&myshared->cont);
		}
		counter++ ;
	}


}