
#ifndef utilities_H_
#define utilities_H_


#define N 16



int parsing_commandline(int argc, char *argv[] ,char **input1FN ,char **input2FN,char** n);
int stoi(char* str);
char* get_data_from_file(char* fn , int n);
int** convert_data_to_arr(char* data , int n);
void print_arr(int** a , int s);
void _free(int **a , int s);
char* data_processor(int** A,int** B,int n,int quarter , int *data_len);
char* get_data(int fd , int *length);
int** fill_matrix(char* q1,char* q2,char* q3,char* q4,int n);
float** convert_to_float(int **C,int n);
int getpow(int n);

#endif /* utilities_H_ */

