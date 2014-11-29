#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
//must use an integer variable as input=
#define NumberOfSetBits(i)   ({ i = i - ((i >> 1) & 0x55555555);\
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);\
     (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;})

#define  GetHadamarTile(row,col) ({int Macro_tmp = row&col;\
	Macro_tmp = NumberOfSetBits(Macro_tmp) & 1;\
	 (1-2*Macro_tmp);})

// #define SWAP(a,b)int* Macro_tmp=b; b=a; a=Macro_tmp;

	// Macro_tmp?-1:1______i checked and the mult is faster then branch
/*int NumberOfSetBits(int i)
{
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int GetHadamarTile(int row,int col){
	int tmp = row&col;
	tmp = NumberOfSetBits(tmp) & 1; //check if number of bits is even
	//return tmp?1:-1;
	return 1-2*tmp;
}*/


/*
input is reading vector
output is writing vector
vector_size is size of input and output
working size: number of iterations/ length of upper half we work on
starting index: the index we start work on the ENTIRE vector (i.e., if we start on the bottom half and vector size is 4, starting index would be 2)
*/

// #define ct_phase(input, output, working_size, starting_index) {\
// 		register int upper_bound = working_size+starting_index;\
// 		#pragma omp parallel\
// 		{\
// 			#pragma omp for schedule(static, 1 + ((working_size-1)/omp_get_num_threads()))\
// 			for(register int i=starting_index; i<upper_bound; i++){\
// 				register int working_index = i+working_size;\
// 				output[i]=input[i]+input[working_index];\
// 				output[working_index] = input[i]-input[working_index];\
// 			}\
// 		}\
// 	}





/*
input is reading vector
output is writing vector
vector_size is size of input and output
working size: number of iterations/ length of upper half we work on
starting index: the index we start work on the ENTIRE vector (i.e., if we start on the bottom half and vector size is 4, starting index would be 2)
*/
void ct(int* input, int* output, int vector_size, int working_size, int starting_index) {
	// printf("thread pid(?): %d\n", omp_get_thread_num());
	if(vector_size == 0){
		return;
	}
	register int upper_bound = working_size+starting_index;
	#pragma omp parallel
	{
		#pragma omp for schedule(static, 1 + ((working_size-1)/omp_get_num_threads()))
		for(register int i=starting_index; i<upper_bound; i++){
			register int working_index = i+working_size;//this is the index in the bottom half corresponding to i. (i+vector size/2)
			output[i]=input[i]+input[working_index];
			output[working_index] = input[i]-input[working_index];
		}
	}
		#pragma omp parallel sections 
		{
			#pragma omp section	
			{
				ct(output , input ,vector_size>>1, working_size>>1, 0);
			}
			#pragma omp section
			{
				ct(output + (vector_size>>1), input + (vector_size>>1) ,vector_size>>1, working_size>>1, 0);
			}
		}
		
}




#define wht_bfly(a,b) do {\
	 register int Macro_tmp = *a;\
     *a += *b;\
     *b = Macro_tmp - *b;\
}while(0)
// void wht_bfly (int* a, int* b)
// {
        // register int tmp = *a;
        // *a += *b;
        // *b = tmp - *b;
// }

// #define l2 (x) ({\
 // register int l2;\
        // for (l2 = 0; x > 0; x >>=1)\
        // {\
                // ++ l2;\
        // }\
        // l2;\
// })

// just a integer log2
int l2 (int x)
{
        register int l2;
        for (l2 = 0; x > 0; x >>=1)
        {
                ++ l2;
        }

        return (l2);
}

#define FIVE(a) (a>5? 5 : a)
#define inside_Loop for (register int k = 0; k < (1<<i); ++k)\
			   {\
				register int* a = vector + j + k;\
				register int* b = vector + j + k + (1<<i);\
				register int Macro_tmp = *a;\
				*a += *b;\
				*b = Macro_tmp - *b;\
			   }
////////////////////////////////////////////
// Fast in-place Walsh-Hadamard Transform //
////////////////////////////////////////////

void FWHT (int* vector,int size)
{
  const register int log2 = l2(size) - 1;
  // #pragma omp parallel
  // {
  	register int num_thread_log=l2(omp_get_num_threads());
	for (register int i = 0; i < log2; ++i)
	{
		if(i < 11) {
		#pragma omp parallel for schedule(static, 32768>>FIVE(i))
	    for (register int j = 0; j < (1 << log2); j += 1 << (i+1))
	    {
			inside_Loop;
			}
			   // for (register int k = 0; k < (1<<i); ++k)
			   // {
				// register int* a = vector + j + k;
				// register int* b = vector + j + k + (1<<i);
				// register int Macro_tmp = *a;
				// *a += *b;
				// *b = Macro_tmp - *b;
			   // }
			}	
		else {
			for (register int j = 0; j < (1 << log2); j += 1 << (i+1))
			{
					#pragma omp parallel for schedule(static, 256<<FIVE(i))
					inside_Loop;
			}
		}
	}
	// }
}


void fast_parallel_walsh(int* vector, int size)
{	
	FWHT(vector, size);
	// printf("input vector is: ");
	// for(int i=0; i<size; i++) 
	// 	printf("%d ", vector[i]);
	// printf("\n");
	// register int* temp_out_vector = malloc(sizeof(int) * size);
	// if(!temp_out_vector) {//failed to malloc
	// 	printf("Failed to allocate memory for input!");
	// 	exit(-1);
	// }
	// register int* temp_inp_vector = vector;
	// register int log_times = 0;
	// register int working_size;
	// ct(temp_inp_vector,temp_out_vector, size, size>>1,0);
	// // for(register unsigned int i = size; i>1; i>>=1){
	// // 	// printf("in outer loop, i is: %d\n", i);
		
	// // 	working_size = i>>1;//chunk size that the ct phase will run on (power of 2)
	// // 	// printf("iteration number: %d\n", log_times);
	// // 	// printf("input vector is: ");
	// // 	// for(int i=0; i<size; i++) 
	// // 	// 	printf("%d ", temp_inp_vector[i]);
	// // 	// printf("\n");
	// // 	// printf("output vector is: ");
	// // 	// for(int i=0; i<size; i++) 
	// // 	// 	printf("%d ", temp_out_vector[i]);
	// // 	// printf("\n");
	// // 	register int max_section_num=(1<<log_times);
	// // 	#pragma omp parallel
	// // 	{
	// // 		#pragma omp for schedule(static)
	// // 		for(register int j = 0; j<max_section_num; j++){

	// // 			//this does a CT phase:
	// // 			// ct_phase(temp_inp_vector, temp_out_vector, working_size , j*i);
	// // 			register int starting_index = j*i;
	// // 			register int upper_bound = working_size+starting_index;
	// // 					#pragma omp parallel for schedule(static, 1 + ((working_size-1)/omp_get_num_threads()))
	// // 					for(register int i=starting_index; i<upper_bound; i++){
	// // 						register int working_index = i+working_size;
	// // 						temp_out_vector[i]=temp_inp_vector[i]+temp_inp_vector[working_index];
	// // 						temp_out_vector[working_index] = temp_inp_vector[i]-temp_inp_vector[working_index];
	// // 					}


	// // 			//SWAP(temp_inp_vector, temp_out_vector);
	// // 		}
	// // 	}
	// // 	log_times++;
	// // 	int* x=temp_out_vector;
	// // 	temp_out_vector=temp_inp_vector;
	// // 	temp_inp_vector=x;
	// // }
	
	// // printf("AFTER FINISH");
	// // printf("input vector is: ");
	// // 	for(int i=0; i<size; i++) 
	// // 		printf("%d ", temp_inp_vector[i]);
	// // 	printf("\n");
	// // 	printf("output vector is: ");
	// // 	for(int i=0; i<size; i++) 
	// // 		printf("%d ", temp_out_vector[i]);
	// // 	printf("\n");
	// /*if (vector==temp_out_vector){
	// 	memcpy(vector,temp_inp_vector , sizeof(int) * size);
	// 	free(temp_inp_vector);
	// 	printf("output vector is: ");
	// 	for(int i=0; i<size; i++) 
	// 		printf("%d ", vector[i]);
	// 	printf("\n");
	// 	return;
	// }
	// //memcpy(vector,temp_out_vector , sizeof(int) * size);
	// printf("output vector is: ");
	// for(int i=0; i<size; i++) 
	// 	printf("%d ", vector[i]);
	// printf("\n");
	// free(temp_out_vector);*/
	// memcpy(vector,temp_inp_vector , sizeof(int) * size);

}

void simple_parallel_walsh(int* vector, int size)
{
	register int* input = malloc(sizeof(int) * size);
	if(!input) {//failed to malloc
		printf("Failed to allocate memory for input!");
		exit(-1);
	}
	memcpy(input, vector, sizeof(int) * size);//i checked and memcpy cant change the destination address to null or anything 
	
	
	#pragma omp parallel
	{
		#pragma omp for schedule(static, 1 + ((size-1)/omp_get_num_threads()))
		for(register int col = 0; col<size; col++) {
			register int sum = 0;
			for(register int row = 0; row<size; row++) {
				sum+=GetHadamarTile(row,col)*input[row];
			}
			vector[col] = sum;
			sum = 0;
		}
	}
	free(input);
}

