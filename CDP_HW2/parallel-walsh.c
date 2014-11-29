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

#define SWAP(a,b)({int* Macro_tmp=b; b=a; a=Macro_tmp;})

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



void ct_phase(int* input, int* output, int vector_size, int working_size, int starting_index) {
	register int upper_bound = working_size+starting_index;
	for(register int i=starting_index; i<upper_bound; i++){
		register int working_index = i+(vector_size>>1);
		output[i]=input[i]+input[working_index];
		output[working_index] = input[i]-input[working_index];
	}
}

void fast_parallel_walsh(int* vector, int size)
{
	register int* temp_out_vector = malloc(sizeof(int) * size);
	if(!temp_out_vector) {//failed to malloc
		printf("Failed to allocate memory for input!");
		exit(-1);
	}
	register int* temp_inp_vector = vector;
	register int log_times = 0;
	register int working_size;
	for(register int i = size; i>1; i>>1){
		log_times++;
		working_size = i>>1;
		for(register int j = 0; j<log_times; j++){
			ct_phase(temp_inp_vector, temp_out_vector, size, working_size , j*working_size);
			SWAP(temp_inp_vector, temp_out_vector);
		}
	}
	if (vector==temp_out_vector){
		free(temp_inp_vector);
		return;
	}
	memcpy(vector,temp_out_vector , sizeof(int) * size);
	free(temp_out_vector);

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

