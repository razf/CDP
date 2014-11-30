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

// just a integer log2
int l2 (register int x)
{
        register int l2;
        for (l2 = 0; x > 0; x >>=1)
        {
                ++ l2;
        }

        return (l2);
}
// need to define max_k before use
#define inside_Loop for (register int k = 0; k < max_k; ++k)\
			   {\
				register int* const a = reg_vector + j + k;\
				register int* const b = a + max_k;\
				const register int Macro_tmp = *a;\
				*a += *b;\
				*b = Macro_tmp - *b;\
			   }


void fast_parallel_walsh(int* vector, int size)
{	
	const register int log2 = l2(size) - 1;
	const register int switch_loop_parallel=log2>>1;
	const register int max_j=1<<log2;
	register int* const reg_vector=vector;
	const register int num_thread_log=l2(omp_get_max_threads())-1;
	for (register int i = 0; i < log2; ++i)
	{
		const register int max_k=1<<i;
		if(i <= switch_loop_parallel) {
		register int chunk=max_j>>(num_thread_log+i+1);
		chunk= (chunk>256 ? chunk : 256); // chunks lower then 512 are too small
		// max_j>>(num_thread_log+i+1) 32768>>FIVE(i)
		#pragma omp parallel for schedule(static,chunk)
			for (register int j = 0; j < max_j; j += 1 << (i+1)) // putting the jump size in register and using it here make a slowdown
			{
			
				inside_Loop;// need to define max_k before use
			}
		}	
		else {
			register int j=0;
			register int chunk=max_k>>(num_thread_log);
			chunk= (chunk>256 ? chunk : 256);// chunks lower then 512 are too small
			// before some of the optimization the paragma parallel here insted of parallel inside the loop 
			//made measurable improvement now it does not make a measurable speedup/slowdown 
			#pragma omp parallel // if we create the threads here and not in the pragma for we reduce overhead of creating threads each iteration
			{
				for (; j < max_j;)
				{
					
					//max_k>>(num_thread_log) 256<<FIVE(i-11)
					#pragma omp  for schedule(static, chunk)
					inside_Loop;// need to define max_k before use
					#pragma omp barrier // it worked without barrier but i think without barrier there could be errors because the inside_Loop use j
					#pragma omp single 
					{
						 j += 1 << (i+1);
					}
				}
			}
		}
	}

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

