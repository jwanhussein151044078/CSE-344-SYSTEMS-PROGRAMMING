
#include <unistd.h>  
#include <string.h>
#include <math.h>
#include "utilities.h"

#include <stdio.h>

# define PI 3.14


char* usage_information(){
	return "invalid input :\nexpected input \" -i path/to/input_file -o path/to/output_file -t time[1,50]\n";
}
/*
 * function to parsing the input
 * return 0 if success , -1 in case of invalid input .  	
*/
int parsing_commandline(int argc, char *argv[] ,char **inputFN ,char **outputFN ,int *Time){
	int opt;
	while((opt = getopt(argc, argv, ":i:o:t:")) != -1){	
    	
    	if(opt == 'i' && optind == 3){
    		*inputFN = optarg ;
    	}else if (opt == 'o' && optind == 5){
    		*outputFN = optarg ; 
    	}else if (opt == 't' && optind == 7){
    		*Time = string_to_int(optarg) ;
    		if (*Time == 0){
    			return -1 ;
    		} 
    	}else{
     		return -1 ;
    	}   
    }
    if (strcmp(*inputFN,*outputFN) == 0){
    	return -1;
    }        
    if(optind < 7){
    	return -1;
    }
    return 0 ;
}

/**
 * function to convert a string of two digit to integer 
 * returns the value of the integer if the value is bigger than zero and less or equal to 50 , otherwise returns zero .
*/
int string_to_int(char* Time){
	int d1 , d2 , t ;
	if (Time[0] >= '0' && Time[0] <= '9'){
		d1 = Time[0]-48;
	}else{
		return 0 ;
	}
	
	if (Time[1] >= '0' && Time[1] <= '9'){
		d2 = Time[1]-48;
	}else if (Time[1] == '\0'){
		return d1 ;
	}
	
	if(Time[2] != '\0'){
		return 0 ;
	}
	t = (d1*10)+d2;
	return (t>50)?0:t ; 
}

void convert_StoI(int x[N] ,int y[N] , char str[160] ){
	int i = 0 , j = 0 , flag;
	int re = 0 , im = 1 ;
	int count1 = 0, count2 = 0 ; 
	char temp[4] ;
	while(str[i] != '\0' && str[i] != '\n'  && i < 160){
		j = 0 ;
		flag = 0 ;
		while(str[i] >= '0' && str[i] <= '9'){
			temp[j] = str[i];
			i++;
			j++;
			flag = 1 ;
		}
		temp[j] = '\0';
		if(flag == 1 ){
			if (re == 0){
				re = 1 ;
				x[count1++] = StoI(temp);
				im = 0 ;
			}
			else{
				im = 1 ;
				y[count2++] = StoI(temp);
				re = 0 ;
			}
		}
		i++;		
	}

}

int StoI(char src[4]){
	int i = 0 ;
	while(src[i] != '\0' ){i++;}
	if(i==1){
		return ((int)src[0]-48);
	}
	else if(i == 2){
		return (((int)src[0]-48)*10)+((int)src[1]-48);	
	}else{
		return (((int)src[0]-48)*100)+(((int)src[1]-48)*10)+((int)src[2]-48);
	}

}

void DFT(int inreal[N] , int inimag[N] ,int outreal[N], int outimag[N]) {
    /*int n = inreal.length;*/
    for (int k = 0; k < N ; k++) {  // For each output element
        int sumreal = 0;
        int sumimag = 0;
        for (int t = 0 ; t < N ; t++) {  // For each input element
            int angle = 2 * PI * t * k / N ;
            sumreal +=  (inreal[t] * cos(angle)) + (inimag[t] * sin(angle));
            sumimag += (-inreal[t] * sin(angle)) + (inimag[t] * cos(angle));
        }
        outreal[k] = sumreal;
        outimag[k] = sumimag;
    }
}

int DFT_TO_STR(char strout[250] , int Xre[N] , int Xim[N]){
	int i = 0 , j = 0;
	char temp1[6] , temp2[6] ;
	int length1,length2;
	int pos = 0 ;
	for(i = 0 ; i < N ; i++){
		
		length1 = itos(Xre[i],temp1);
		length2 = itos(Xim[i],temp2);
		for (j = 0; j < length1 ; ++j){
			strout[pos++] = temp1[j];
		}
		j = 0 ;
		strout[pos++] = ' ';
		if(temp2[0] != '-'){
			strout[pos++] = '+';
		}else{
			strout[pos++] = '-';
			j++;
		}
		strout[pos++] = 'i';
		for (j ; j < length2 ; ++j){
			strout[pos++] = temp2[j];
		}
		if(i < (N-1)){
			strout[pos++] = ',';
		}
	}

	strout[pos++] = '\0';
	return pos ;
}


int itos(int in , char out[6] ){
	int i = 0 , digit = 1 ;
	int temp = in ;
	int _return = 0 ;
	if(in < 0){
		out[0] = '-';
		i++;
		in = in * -1 ;
	}

	while(temp/10 != 0){
		temp /= 10;
		digit++ ;
	}
	out[digit+i] = '\0' ;
	_return = digit+i ;
	while(digit > 0){
		out[digit+i-1] = (char)((in%10)+48);
		digit--;
		in = in/10 ;
	}
	return _return ;
}