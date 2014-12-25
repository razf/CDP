#include <mpi.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define ABS(x) (x>0?x:-x)
#define MIN(a,b) (a<b?a:b)
#define INFINTY INT_MAX

typedef struct
{
	int citiesNum;
	int xCoord;
	int yCoord;
	int jobs;
	int jobs_array;
} dataToProcesses;

void build_derived_static_type(INDATA_TYPE* indata, MPI_Datatype* message_type_ptr, int first_array_len, int second_array_len){
// Build a derived datatype consisting of two ints and three int arrays.
	int block_lengths[5];
	MPI_Aint displacements[5];
	MPI_Aint addresses[6];
	MPI_Datatype typelist[5];
	// First specify the types
	// typelist[0] = x,y arrays length, typelist[1] = xCoord, typelist[2] = yCoord, typelist[3] = job num, typelist[4] = jobs array 
	typelist[0] = typelist[1]= typelist[2] = typelist[3] = typelist[4] = MPI_INT;
	// Specify the number of elements of each type
	block_lengths[1] = block_lengths[2] = first_array_len;
	block_lengths[0] = block_lengths[3]=1;
	block_lengths[4] = second_array_len;
	// Calculate the displacements of the members
	//relative to indata
	MPI_Address(indata, &addresses[0]);
	MPI_Address(&(indata->citiesNum), &addresses[1]);
	MPI_Address(&(indata->xCoord), &addresses[2]);
	MPI_Address(&(indata->yCoord), &addresses[3]);
	MPI_Address(&(indata->jobs), &addresses[4]);
	MPI_Address(&(indata->jobs_array), &addresses[5]);
	displacements[0] = addresses[1] - addresses[0];
	displacements[1] = addresses[2] - addresses[0];
	displacements[2] = addresses[3] - addresses[0];
	displacements[3] = addresses[4] - addresses[0];
	displacements[4] = addresses[5] - addresses[0];
	// Create the derived type
	MPI_Type_struct(5, block_lengths,
	displacements, typelist, message_type_ptr);
	// Commit it so that it can be used
	MPI_Type_commit(message_type_ptr);
} // Build—derived—type 


int* IntMatrixToArray(int** matrix, int n){
	int* a = malloc(sizeof(int)*n*n);
		for(int i=0; i<n; i++) 
			for(int j=0; j<n; j++)
				a[i*n+j] = matrix[i][j];
	return a;
}

char* CharMatrixToArray(char** matrix, int n){
	char* a = malloc(sizeof(char)*n*n);
		for(int i=0; i<n; i++) 
			for(int j=0; j<n; j++)
				a[i*n+j] = matrix[i][j];
	return a;
}

double calc_bound(int** edgeGarph, char** auxMatrix, int citiesNum);
int** calc_edge_matrix(int citiesNum, int xCoord[], int yCoord[]);
int bnb_serial(int shortestPath[], int** edgeGarph, char** auxMatrix , int bestBound, int index, int citiesNum, int workPath[]);
// The static parellel algorithm main function.

int tsp_static(int citiesNum, int xCoord[], int yCoord[], int shortestPath[]) {
	//build jobs
	int numtasks;
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	int max_job=(citiesNum-2)*(citiesNum-1);

}
int tsp_main(int citiesNum, int xCoord[], int yCoord[], int shortestPath[])
{
	char** auxMatrix = malloc(sizeof(char*)*citiesNum);
	int** edgeGarph = calc_edge_matrix(citiesNum, xCoord, yCoord);
	if(!auxMatrix) {
		printf("NULL!!!1");
		exit(0);
	}
	for(int i=0; i<citiesNum; i++) {
		auxMatrix[i] = calloc(citiesNum,sizeof(char));
		if(!auxMatrix[i]) {
			printf("NULL!!!2");
			exit(0);
		}
	}

	for(int i=0; i<citiesNum; i++) {
		auxMatrix[i][i] = -1;
	}

	int* workPath = malloc(sizeof(int) * citiesNum);
	workPath[0] = 0; 

	//print the matricies!
	for(int i=0; i<citiesNum; i++) {
		for(int j=0; j<citiesNum; j++) {
			printf("%d ", edgeGarph[i][j]);
		}
		printf("\n");
	}

	for(int i=0; i<citiesNum; i++) {
		for(int j=0; j<citiesNum; j++) {
			printf("%d ", auxMatrix[i][j]);
		}
		printf("\n");
	}
	int best = bnb_serial(shortestPath, edgeGarph, auxMatrix, INFINTY,0,citiesNum,workPath);
	free(workPath);
	for(int i=0; i<citiesNum; i++){
		free(edgeGarph[i]);
		free(auxMatrix[i]);
	}
	free(edgeGarph);
	free(auxMatrix);
	return best;//TODO
}

int bnb_serial(int shortestPath[], int** edgeGarph, char** auxMatrix , int bestBound, int index, int citiesNum, int workPath[])
{
	//index is the index of the last vertex in the path (i.e, in [1,64,53,63], index would be 3)
	int last_node_in_path = workPath[index];
	int path_length = 0;
	if(index == citiesNum-1) {
		for(int i=0; i<citiesNum-1; i++){
			path_length += edgeGarph[workPath[i]][workPath[i+1]];
		}
		path_length += edgeGarph[workPath[0]][workPath[citiesNum-1]];
		if(path_length<bestBound) {
			for(int i=0; i<citiesNum; i++) {
				shortestPath[i] = workPath[i];
			}
		}
		printf("found a path of length %d! ",path_length);
		for(int i=0; i<citiesNum; i++)
			printf("%d ", workPath[i]);
		printf("\n");
		fflush(stdout);
		return path_length;
	} else {
		//find the first available edge:
		int edge = -1;
		for(int i=0; i<citiesNum; i++) {
			if(auxMatrix[last_node_in_path][i] == 0) {//we can use this edge!
				edge = i;
				break;
			}
		}
		if (edge==-1){
			return INFINTY;
		}
		auxMatrix[last_node_in_path][edge] = 1;
		auxMatrix[edge][last_node_in_path] = 1;
		double best_bound_with_edge = calc_bound(edgeGarph, auxMatrix, citiesNum);
		auxMatrix[last_node_in_path][edge] = -1;
		auxMatrix[edge][last_node_in_path] = -1;
		double best_bound_without_edge = calc_bound(edgeGarph, auxMatrix, citiesNum);
		auxMatrix[last_node_in_path][edge] = 0;
		auxMatrix[edge][last_node_in_path] = 0;

		if(best_bound_with_edge <= best_bound_without_edge) {
			if(best_bound_with_edge > bestBound) {
				return INFINTY;
			}
			//back up the array, and change it so the edge will be taken.
			char before_change[citiesNum];
			for(int i=0; i<citiesNum; i++){
				// printf("backing up %d in %d,%d edge is %d, cn is %d\n",auxMatrix[last_node_in_path][i], last_node_in_path, i, edge, citiesNum);
				fflush(stdout);
				before_change[i]=auxMatrix[last_node_in_path][i];
				if(auxMatrix[last_node_in_path][i] !=0) {
					continue;
				} else {
					if(i==edge){ 
						auxMatrix[last_node_in_path][i]=1;
						auxMatrix[i][last_node_in_path]=1;
					}
					else {
						auxMatrix[last_node_in_path][i]=-1;
						auxMatrix[i][last_node_in_path]=-1;
					}
				}
			}
			workPath[index+1]=edge;
			int actual_bound_with_edge = bnb_serial(shortestPath,edgeGarph,auxMatrix,bestBound,index+1,citiesNum,workPath);
			for(int i=0; i<citiesNum; i++) {
				auxMatrix[last_node_in_path][i] = before_change[i];
				auxMatrix[i][last_node_in_path] = before_change[i];
			}
			
			bestBound = MIN((bestBound),(actual_bound_with_edge));
			if(best_bound_without_edge < bestBound){//otherwise, no need to run!
				auxMatrix[last_node_in_path][edge] = -1;
				auxMatrix[edge][last_node_in_path] = -1;
				int actual_bound_without_edge = bnb_serial(shortestPath,edgeGarph,auxMatrix,bestBound,index,citiesNum,workPath);
				auxMatrix[last_node_in_path][edge] = 0;
				auxMatrix[edge][last_node_in_path] = 0;
				bestBound = MIN((bestBound),(actual_bound_without_edge));
			}
		} else {
			if(best_bound_without_edge > bestBound) {
				return bestBound;
			}
			auxMatrix[last_node_in_path][edge] = -1;
			auxMatrix[edge][last_node_in_path] = -1;
			int actual_bound_without_edge = bnb_serial(shortestPath,edgeGarph,auxMatrix,bestBound,index,citiesNum,workPath);
			auxMatrix[last_node_in_path][edge] = 0;
			auxMatrix[edge][last_node_in_path] = 0;
			bestBound = MIN((bestBound),(actual_bound_without_edge));
			if (best_bound_with_edge < bestBound) {
				char before_change[citiesNum];
				for(int i=0; i<citiesNum; i++){
					before_change[i]=auxMatrix[last_node_in_path][i];
					if(auxMatrix[last_node_in_path][i] !=0) {
						continue;
					} else {
						if(i==edge){
							auxMatrix[last_node_in_path][i]=1;
							auxMatrix[i][last_node_in_path]=1;
						}
						else{
							auxMatrix[last_node_in_path][i]=-1;
							auxMatrix[i][last_node_in_path]=-1;
						}
					}
				}
				workPath[index+1]=edge;
				int actual_bound_with_edge = bnb_serial(shortestPath,edgeGarph,auxMatrix,bestBound,index+1,citiesNum,workPath);
				for(int i=0; i<citiesNum; i++) {
					auxMatrix[last_node_in_path][i] = before_change[i];
					auxMatrix[i][last_node_in_path] = before_change[i];
				}
				bestBound = MIN((bestBound),(actual_bound_with_edge));
			}
			
		}
	}
	return bestBound;
}

double calc_bound(int** edgeGarph, char** auxMatrix, int citiesNum) {
	double bound = 0;
	int low_min = -1;
	int high_min = -1;
	int required = 0;
	for(int i=0; i<citiesNum; i++){
		for(int j=0; j<citiesNum; j++){
			if(auxMatrix[i][j]==-1){
				continue;
			}
			if(auxMatrix[i][j]==1){
				bound+=edgeGarph[i][j];
				required++;
				if(required==2)
					break;
			} else {
				if(low_min == -1) {
					low_min = edgeGarph[i][j];
					continue;
				}
				if(high_min == -1) {
					if(edgeGarph[i][j] < low_min) {
						high_min = low_min;
						low_min = edgeGarph[i][j];
						continue;
					} else {
						high_min = edgeGarph[i][j];
						continue;
					}
				}
				if(edgeGarph[i][j] > high_min) {
					continue;
				}
				if(edgeGarph[i][j] < low_min) {
					high_min = low_min;
					low_min = edgeGarph[i][j];
					continue;
				} else {
					high_min = edgeGarph[i][j];
				}
			}
		}
		if(required == 1) {
			bound += low_min;
		} else if (required == 0) {
			bound += low_min + high_min;
		}
		required = 0;
		low_min = -1;
		high_min = -1;
	}
	return bound/2;
}

int** calc_edge_matrix(int citiesNum, int xCoord[], int yCoord[]) {
	int** edges = malloc(sizeof(int*) * citiesNum);
	if(!edges) {
		printf("Error while allocating matrix!");
		exit(-1);
	}
	for(int i=0; i<citiesNum; i++) {
		edges[i] = malloc(sizeof(int) * citiesNum);
		if(!edges[i]) {
			printf("Error while allocating matrix arrays!");
			exit(-1);
		}
	}
	for (int i = 0; i<citiesNum; i++)
		for(int j=0; j<citiesNum; j++)
			edges[i][j] =  ABS((xCoord[i] - xCoord[j]))+ABS((yCoord[i]-yCoord[j]));

	return edges;
}

