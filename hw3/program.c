#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "utilities.h"
#include "child.h"
#include "svd.h"


void HANDLER(int signo);

int main(int argc, char *argv[]){
	char *in1fn , *in2fn , *ns;
	char *data1 , *data2 ;
	char *q1 , *q2 , *q3 , *q4 ;
	int n ;
	int fd1 , fd2 ;
	int **A , **B , **C;
	struct sigaction sa ;
	int l1 , l2 , l3 , l4 ;

	

	memset(&sa , 0 , sizeof(sa));
	sa.sa_handler = &HANDLER;
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD,&sa,NULL)==-1 || sigaction(SIGINT,&sa,NULL)==-1){
		perror("sigaction ERROR :");
		return -1 ;
	}
	if(parsing_commandline(argc,argv,&in1fn,&in2fn,&ns)==-1){
		printf("INVAILD COMMAND LINE ARGUMENT::\n");
		printf("expected :\n\t");
		printf("./programname -i inputfile1 -j inputfile2 -n integer\n");
		return -1;
	}
	
	if((n=stoi(ns))==-1){
		printf("stoi ERROR : cannot convert n to positive integer !!\n");
		return -1 ;
	}
	if(n == 0){
		printf("\n\tn was zero, the program terminated !! \n");
		return -1 ;
	}
	if(( data1 = get_data_from_file(in1fn,n)) == NULL){
		return -1 ;
	}
	if(( data2 = get_data_from_file(in2fn,n)) == NULL){
		free(data1);
		return -1 ;
	}
	
	A = convert_data_to_arr(data1, n);
	if (A == NULL){
		free(data1);
		free(data2);
		return -1 ;
	}
	B = convert_data_to_arr(data2, n);
	if (A == NULL){
		_free(A,n);
		free(data1);
		free(data2);
		return -1 ;
	} 
	free(data1);
	free(data2);

	print_arr(A,n);
	printf("-------------------\n");
	print_arr(B,n);

	////////////////////////////    process data to transfer    //////////////////////////////////////
	int p[8][2];
	int s[2] ;   
	int data_l1 ,data_l2 ,data_l3,data_l4; 
	char* data_to_send1 = data_processor(A,B,n,1,&data_l1);
	char* data_to_send2 = data_processor(A,B,n,2,&data_l2);
	char* data_to_send3 = data_processor(A,B,n,3,&data_l3);
	char* data_to_send4 = data_processor(A,B,n,4,&data_l4);
	if(data_to_send4 == NULL || data_to_send2 == NULL || data_to_send3 == NULL || data_to_send4 == NULL){
		_free(A,n);
		_free(B,n);
		(data_to_send1)? free(data_to_send1):0 ;
		(data_to_send2)? free(data_to_send2):0 ;
		(data_to_send3)? free(data_to_send3):0 ;
		(data_to_send4)? free(data_to_send4):0 ;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////   init pipes  to transfer data /////////////////////////////////////
	if((pipe(p[0])== -1) ||(pipe(p[1])== -1) ||(pipe(p[2])== -1) ||(pipe(p[3])== -1) ||
	   (pipe(p[4])== -1) ||(pipe(p[5])== -1) ||(pipe(p[6])== -1) ||(pipe(p[7])== -1) ){
		perror("pipe create ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		return -1 ;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////// init pipe for synchronization //////////////////////////////////////// 
	if(pipe (s) == -1){
		perror("pipe create ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		return -1 ;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	if(	write(p[0][1],data_to_send1,data_l1+1) != data_l1+1 ||
		write(p[2][1],data_to_send2,data_l2+1) != data_l2+1 ||
		write(p[4][1],data_to_send3,data_l3+1) != data_l3+1 ||
		write(p[6][1],data_to_send4,data_l4+1) != data_l4+1 ){

		perror("write ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		return 0 ;	
	}

	pid_t cid[4] ;
	for(int i = 0 ; i < 4 ; i++){
		cid[i] = fork();
		if(cid[i] == -1){
			perror("fork ERROR:");
			return -1 ;
		}
		if(cid[i] != 0){
			
		}else{			
			if(close(s[0])==-1){
				perror("close pipe ERROR :");
				kill(getppid(),SIGINT);
				exit(0);
			}
			child(i+1,p[i*2],p[(i*2)+1] , n );
			if(close(s[1])==-1){
				perror("close pipe ERROR :");
				kill(getppid(),SIGINT);
				exit(0);
			}
			return 0 ;
		}
	}
	
	if(close(s[1])==-1){
		perror("close pipe ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		exit(0);
	}
	
	if((close(p[0][0])==-1) || (close(p[1][1])==-1) || (close(p[2][0])==-1) || (close(p[3][1])==-1) ||
       (close(p[4][0])==-1) || (close(p[5][1])==-1) || (close(p[6][0])==-1) || (close(p[7][1])==-1) ){
		perror("close pipe ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
		return 0;
	}

	
	char dummy ;
	if(read(s[0],&dummy , 1) != 0){
		perror("sync read ERROR  :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
		return 0;	
	}

	if(close(s[0])==-1){
		perror("close pipe ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
		
	}
	if((close(p[0][1])==-1) || (close(p[2][1])==-1) ||
       (close(p[4][1])==-1) || (close(p[6][1])==-1) ){
		perror("close pipe ERROR :");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
	}

						

	if((q1 = get_data(p[1][0],&l1)) == NULL || (q2 = get_data(p[3][0],&l2)) == NULL || 
	   (q3 = get_data(p[5][0],&l3)) == NULL || (q4 = get_data(p[7][0],&l4)) == NULL){
		printf("Fail to read data from pipe!!\n");
		_free(A,n);
		_free(B,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
	}

	if((close(p[1][0])==-1) || (close(p[3][0])==-1) ||
       (close(p[5][0])==-1) || (close(p[7][0])==-1) ){
		perror("close pipe ERROR :");
		_free(A,n);
		_free(B,n);
		free(q1);
		free(q2);
		free(q3);
		free(q4);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		kill(getpid(),SIGINT);
	}
	C = fill_matrix(q1,q2,q3,q4,n);
	if(C == NULL){
		_free(A,n);
		_free(B,n);
		_free(C,n);
		free(data_to_send1);
		free(data_to_send2);
		free(data_to_send3);
		free(data_to_send4);
		free(q1);
		free(q2);
		free(q3);
		free(q4);
		kill(getpid(),SIGINT);
	}

	printf("\n\tproduct matrix \n");
	printf("----------------------------------------\n");
	print_arr(C,n);
	printf("________________________________________\n\n");
///////////////////////////////// svd ///////////////////////////////////////////////////
	float* w , **v ;
	int r = getpow(n);

	w = (float*)malloc(sizeof(float)*r);
	v = (float**)malloc(sizeof(float*)*r);
	for(int i = 0 ; i < r ; i++){
		v[i] = (float*)malloc(sizeof(float)*r);
	}
	float**CF = convert_to_float(C,n);
	
	dsvd(CF,r,r,w,v);
	int i = 0 ;
	printf("singular value(s) :\n");
	while(w[i]){
		printf("\t%f\n",w[i++]);
	}

	for(i = 0 ; i < r ; i++){
		free(CF[i]);
		free(v[i]);
	}
	free(CF);
	free(v);
	free(w);
	_free(A,n);
	_free(B,n);
	_free(C,n);
	
	free(data_to_send1);
	free(data_to_send2);
	free(data_to_send3);
	free(data_to_send4);
	free(q1);
	free(q2);
	free(q3);
	free(q4);
	
	return 0 ;
}

void HANDLER(int signo){
	if(signo == SIGCHLD){
		wait(NULL);
	}
	if(signo == SIGINT){
		printf("catch SIGINT\n");
		wait(NULL);
		wait(NULL);
		wait(NULL);
		wait(NULL);
		exit(1);
	}
	
}