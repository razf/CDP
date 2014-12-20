#include <mpi.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define ABS(x) (x>0?x:-x)
#define MAX(a,b) (a>b?a:b)
#define INFINTY INT_MAX

double calc_bound(int** edgeGarph, char** auxMatrix, int citiesNum);
int** calc_edge_matrix(int citiesNum, int xCoord[], int yCoord[]);
int bnb_serial(int shortestPath[], int** edgeGarph, char** auxMatrix , int bestBound, int index, int citiesNum, int workPath[]);
// The static parellel algorithm main function.
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
	printf("bnb!");
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
		printf("%d", edge);
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
				before_change[i]=auxMatrix[last_node_in_path][i];
				if(auxMatrix[last_node_in_path][i] !=0) {
					continue;
				} else {
					if(i=edge){ 
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
			
			bestBound = MAX(bestBound,actual_bound_with_edge);
			if(best_bound_without_edge < bestBound){//otherwise, no need to run!
				auxMatrix[last_node_in_path][edge] = -1;
				auxMatrix[edge][last_node_in_path] = -1;
				int actual_bound_without_edge = bnb_serial(shortestPath,edgeGarph,auxMatrix,bestBound,index,citiesNum,workPath);
				auxMatrix[last_node_in_path][edge] = 0;
				auxMatrix[edge][last_node_in_path] = 0;
				bestBound = MAX(bestBound,actual_bound_without_edge);
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
			bestBound = MAX(bestBound,actual_bound_without_edge);
			if (best_bound_with_edge < bestBound) {
				char before_change[citiesNum];
				for(int i=0; i<citiesNum; i++){
					before_change[i]=auxMatrix[last_node_in_path][i];
					if(auxMatrix[last_node_in_path][i] !=0) {
						continue;
					} else {
						if(i=edge){
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
				bestBound = MAX(bestBound,actual_bound_with_edge);
			}
			return bestBound;
		}
	}
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
			edges[i][j] =  ABS(xCoord[i] - xCoord[j])+ABS(yCoord[i]-yCoord[j]);

	return edges;
}

