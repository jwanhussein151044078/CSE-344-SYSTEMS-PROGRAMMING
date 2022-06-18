
#ifndef utilities_H_
#define utilities_H_


#define N 16


char* usage_information();
int parsing_commandline(int argc, char *argv[] ,char **inputFN ,char **outputFN );
int convertline(char** line , char* buf , int bufsize);
void LSM(int *X , int *Y , int sumx , int sumy ,double meanX ,double meamY , double* slop ,double* constant);
int last_line(int temp_fd , int *start );
int find_line_errors(char* line,double* mea,double* mse,double* rmse);
double find_mea(int x[10] ,int y[10] ,double a , double b );
double find_mse(int x[10] ,int y[10] ,double a , double b );
double find_rmse(double mse);
void find_SD(double* mae ,double* mse ,double* rmse ,int linenum ,double* smae ,double* smse ,
				double* srmse ,double* sdmae ,double* sdmse ,double* sdrmse  );

#endif /* utilities_H_ */

