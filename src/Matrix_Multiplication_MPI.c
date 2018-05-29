/*
 ============================================================================
 Name        : Matrix_Multiplication_MPI.c
 Author      : Angelo Settembre
 Version     :
 Copyright   : Your copyright notice
 Description : Matrix Multiplication in MPI with Collective communication routines
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define SIZE 3				/*Dimensione righe e colonne matrice*/

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	char message[100];        /* storage for message */
	int **matrixA, **matrixB, **matrixC;					/*MATRICI*/
	int i,j,k;
	MPI_Status status ;   /* return status for receive */

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p); 


	/*ALLOCAZIONE MATRICI*/
	matrixA = (int**) malloc(SIZE * sizeof(int*));
	matrixB = (int**) malloc(SIZE * sizeof(int*));
	matrixC = (int**) malloc(SIZE * sizeof(int*));


	/*RIEMPIMENTO PRIMA MATRICE*/
	for(i = 0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			matrixA[i][j] = (rand() % 8);		//Valori tra 0 e 8 esclusi
		}
	}

	/*RIEMPIMENTO SECONDA MATRICE*/
	for(i = 0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			matrixB[i][j] = (rand() % 8);		//Valori tra 0 e 8 esclusi
		}
	}

	printf("First Matrix \n");
	for(i = 0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			printf("%d\t",matrixA[i][j]);
		}
		printf("\n");
	}

	printf("Second Matrix \n");
	for(i = 0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			printf("%d\t",matrixB[i][j]);
		}
		printf("\n");
	}

	if (my_rank !=0){
		/* create message */
		sprintf(message, "Matrix Multiplication MPI from process %d!", my_rank);
		dest = 0;
		/* use strlen+1 so that '\0' get transmitted */
		MPI_Send(message, strlen(message)+1, MPI_CHAR,
				dest, tag, MPI_COMM_WORLD);
	}
	else{
		printf("Matrix Multiplication MPI From process 0: Num processes: %d\n",p);
		for (source = 1; source < p; source++) {
			MPI_Recv(message, 100, MPI_CHAR, source, tag,
					MPI_COMM_WORLD, &status);
			printf("%s\n",message);
		}
	}
	/* shut down MPI */
	MPI_Finalize(); 


	return 0;
}
