
#ifndef utilities_H_
#define utilities_H_


#define N 16


char* usage_information();
int parsing_commandline(int argc, char *argv[] ,char **inputFN ,char **outputFN ,int *Time);
int string_to_int(char* Time);
void convert_StoI(int x[16] ,int y[16] , char str[160] );
int StoI(char src[4]);
void DFT(int x[N] ,int y[N] ,int outx[N] ,int outy[N]);
int DFT_TO_STR(char strout[210] , int Xre[N] , int Xim[N]);
int itos(int in , char out[6]);

#endif /* utilities_H_ */
