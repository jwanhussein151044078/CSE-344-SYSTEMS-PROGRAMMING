
#include <unistd.h>  
#include <string.h>
#include <math.h>
#include "utilities.h"

#include <stdio.h>




char* usage_information(){
	return "invalid input :\nexpected input \" -i path/to/input_file -o path/to/output_file \n";
}
/*
 * function to parsing the input
 * return 0 if success , -1 in case of invalid input .  	
*/
int parsing_commandline(int argc, char *argv[] ,char **inputFN ,char **outputFN){
	int opt;
	while((opt = getopt(argc, argv, ":i:o:t:")) != -1){	
    	
    	if(opt == 'i' && optind == 3){
    		*inputFN = optarg ;
    	}else if (opt == 'o' && optind == 5){
    		*outputFN = optarg ; 
    	}else{
     		return -1 ;
    	}   
    }
    if (strcmp(*inputFN,*outputFN) == 0){
    	return -1;
    }        
    if(optind < 5){
    	return -1;
    }
    return 0 ;
}

int convertline(char** line , char* buf , int bufsize){
    int X_I[10],Y_I[10] , i , j , sum_X= 0 , sum_Y = 0 ;
    char temp[10];
    int line_length = 54 ;
    double slop , cons ;

    for(i = 0 , j = 0 ; i < bufsize ; i+=2){
        X_I[j] = buf[i];
        Y_I[j] = buf[i+1];
        sum_X += X_I[j];
        sum_Y += Y_I[j];
        j++;    
        strcat(*line,"(");
        line_length += sprintf(temp,"%d",buf[i]);
        strcat(*line,temp);
        strcat(*line,", ");
        line_length += sprintf(temp,"%d",buf[i+1]);
        strcat(*line,temp);
        strcat(*line,"),");        
    }
    LSM(X_I,Y_I,sum_X,sum_Y,(sum_X/10.0),(sum_Y/10.0),&slop,&cons);
    strcat(*line," ");
    line_length += sprintf(temp,"%.3f",slop);
    strcat(*line,temp);
    strcat(*line,"x ");
    line_length += sprintf(temp,"%.3f",cons);
    if(cons > 0 ){
        strcat(*line,"+");
        strcat(*line,temp);
        line_length++;    
    }
    else if(cons < 0) {
        strcat(*line,temp);
    }
    strcat(*line,"\n");
    
    return line_length ;
}

void LSM(int *X , int *Y , int sumx , int sumy ,double meanX ,double meamY , double* slop ,double* constant){
    double X_I2[10] , Y_I[10];
    double sx=0 , sy=0 ;

    for (int i = 0; i < 10; ++i){
        X_I2[i] = (X[i]-meanX)*(X[i]-meanX);
        Y_I[i] = (X[i]-meanX)*(Y[i]-meamY);
        sx += X_I2[i] ;
        sy += Y_I[i] ;
    }

    *slop = sy/sx ;
    *constant = meamY-((sy/sx)*meanX);   
}


int find_line_errors(char* line,double* mea,double* mse,double* rmse){
    int X[10] , Y[10] ;
    double a , b ;

    sscanf(line,"(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d),(%d, %d), %lfx %lf",
                   &X[0],&Y[0],&X[1],&Y[1],&X[2],&Y[2],&X[3],&Y[3],&X[4],&Y[4],&X[5],&Y[5],&X[6],&Y[6],
                   &X[7],&Y[7],&X[8],&Y[8],&X[9],&Y[9],&a,&b);
/*
    for (int i = 0; i < 10; ++i)
    {
        printf("%d , %d\n",X[i],Y[i] );
    }
    printf("a %f\nb = %f\n",a,b );
*/

    *mea  = find_mea(X,Y,a,b);
    *mse  = find_mse(X,Y,a,b);
    *rmse = find_rmse(*mse); 
}


int last_line(int temp_fd , int *start ){
    int fsize , lastline=0 , temp ;
    char ch[1];
    lseek(temp_fd,0,SEEK_SET);
    fsize = lseek(temp_fd,0,SEEK_END);
    if(fsize <= 0){
        return -1;
    }
    lseek(temp_fd,0,SEEK_SET);
    while(read(temp_fd,ch,1) == 1){
        if(ch[0] == '\n'){
            temp = lastline ;
            lastline = lseek(temp_fd,0,SEEK_CUR);
        }
    }
    *start = temp ;
    return fsize - temp ;
}

double find_mea(int x[10] , int y[10] , double a , double b ){
    double sum = 0 ;
    double temp ;
    for (int i = 0; i < 10; ++i){
        temp = (x[i]*a) + b ;
        temp = y[i] - temp ;
        if(temp < 0)
            temp *= -1 ;
        sum += temp ;
    }
    return sum*0.1;
}

double find_mse(int x[10] , int y[10] , double a , double b ){
    double sum = 0 ;
    double temp ;
    for (int i = 0; i < 10; ++i){
        temp = (x[i]*a) + b ;
        temp = y[i] - temp ;
        temp *= temp ;
        sum += temp ;
    }
    return sum*0.1;
}

double find_rmse(double mse ){
    return sqrt(mse);
}

void find_SD(double* mae ,double* mse ,double* rmse ,int linenum ,double* smae ,double* smse ,
                double* srmse ,double* sdmae ,double* sdmse ,double* sdrmse  ){
    
    double sumofmae = 0 , sumofmse = 0 , sumofrmse = 0 ;
    double temp1 , temp2 , temp3 ;
    double temp11=0 , temp21=0 , temp31=0 ;
    for (int i = 0; i < linenum; ++i){
        sumofmae += mae[i];
        sumofmse += mse[i];
        sumofrmse += rmse[i];
    }
    temp1 = sumofmae/linenum ;
    temp2 = sumofmse/linenum ;
    temp3 = sumofrmse/linenum ;
    *smae = temp1;
    *smse = temp2;
    *srmse = temp3;
    
    for (int i = 0; i < linenum; ++i){
        temp11 += (mae[i]-temp1)*(mae[i]-temp1);
        temp21 += (mse[i]-temp2)*(mse[i]-temp2);
        temp31 += (rmse[i]-temp3)*(rmse[i]-temp3);     
    }
    *sdmae = sqrt(temp11/linenum);
    *sdmse = sqrt(temp21/linenum);
    *sdrmse = sqrt(temp31/linenum);
}
