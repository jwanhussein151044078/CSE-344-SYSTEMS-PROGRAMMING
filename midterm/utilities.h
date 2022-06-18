
#ifndef utilities_H_
#define utilities_H_

#include <semaphore.h>

#define shmname "mysharedmem"

struct shm{
	int KP ; // counter to hold the number of soup in the kitchen
	int KC ; // counter to hold the number of main course in the kitchen 
	int KD ; // counter to hold the number of dessert in the kitchen
	int CP ; // counter to hold the number of soup on the counter
	int CC ; // counter to hold the number of mian course on the counter
	int CD ; // counter to hold the number of dessert on the counter

	int KSP ; // counter to hold the total number of soup was delivered to the kitchen 
	int KSC ; // counter to hold the total number of main course was delivered to the kitchen 
	int KSD ; // counter to hold the total number of dessert was delivered to the kitchen 

	int total_served_P ; // counter to hold the number of soup plates has been served by the cooks 
	int total_served_C ; // counter to hold the number of main course plates has been served by the cooks
	int total_served_D ; // counter to hold the number of dessert plates has been served by the cooks

	int tables    ;       // counter to hold the number of empty tables 
	int Ustudentq ;       // counter to hold the number of Ustudent in the queue
	int Gstudentq ;       // counter to hold the number of Gstudent in the queue

	sem_t Kempty ; // semaphore to keep tracking empty spaces in the kitchen , initial value = K 
	sem_t Cempty ; // semaphore to keep tracking empty spaces in the counter , initial value = S
	sem_t Tempty ; // semaphore to keep tracking empty table , initial value = T
	

	sem_t Kmutex ; // binary semaphore to lock and unlock kitchen counters KP , KC and KD , initial value = 1 
	sem_t Cmutex ; // binary semaphore to lock and unlock counter counters CP , CC and CD , initial value = 1
	sem_t Tmutex ; // mutex to lock and unlock the tables counter , initial value = 1 ;

	sem_t TSmutex; // mutex to lock and unlock counter counters total_served_P , total_served_C , total_served_D , initial value = 1

	sem_t cook1 ;   // semaphore to signal the cook who is responsible for soup , initial value = 0
	
	sem_t C1mutex ;   // binary semaphore to signal the cook who is responsible for main course to continue can be posted only from the cook who is resposible for soup , initial value = 0
	sem_t C2mutex ;   // binary semaphore to signal the cook who is responsible for dessert to continue can be posted only from the cook who is resposible for main course , initial value = 0 
	sem_t C3mutex ;   // binary semaphore to signal the cook who is responsible for soup to continue can be posted only from the cook who is resposible for dessert , initial value = 1
	
	sem_t Ssemaph  ;   // sempafore whose value indicates the number of students that can be served , initial value = 0
	sem_t SQ       ;   // student queue mutex , initial value = 1
	sem_t SU       ;   // Ustudent mutex (gives permission to Ustudent to take food from the counter ) , initial value = 0
	sem_t SG       ;   // GStudent mutex (gives permission to Gstudent to take food from the counter ) , initial value = 0

	sem_t cont     ;   // mutex for the organizer
	sem_t SQsem    ;   // sempafore whose value indicates the number of students in the queue , initial value = 0
	

};




int parsing_commandline(int argc, char *argv[] , int* N ,int* T , int* S , int* L ,int* U, int* G , char** fn);
int check_constraints(int N,int M,int T,int S,int L);
int minimum(int a, int b, int c);
int isfilevalide(int fd,int lm3);

#endif /* utilities_H_ */

