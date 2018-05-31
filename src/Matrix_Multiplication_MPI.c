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
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mpi.h"

#define SIZE 4									/*Dimensione righe e colonne di ogni matrice*/

/*PROTOTIPI FUNZIONI*/
void allocateMatrix(int **matrix, int size);
void createMatrix(int **matrix, int size);
void printMatrix(int **matrix, int size);

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int source;   /* rank of sender */
	int dest;     /* rank of receiver */
	int tag=0;    /* tag for messages */
	char message[100];        /* storage for message */
	int **matrixA, **matrixB, **matrixC;					/*MATRICI*/
	int i,j,k;
	int sum = 0;
	MPI_Status status ;   /* return status for receive */

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);


	/*ALLOCAZIONE MATRICI*/
	matrixA = (int**) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE RIGHE
	allocateMatrix(matrixA, SIZE);

	matrixB = (int**) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE RIGHE
	allocateMatrix(matrixB, SIZE);

	matrixC = (int**) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE RIGHE
	allocateMatrix(matrixC, SIZE);
	/*		*/

	srand(time(NULL));										//SEME DELLA FUNZIONE rand()

	//ACCEDERE SIZE*i+j


	if (my_rank !=0){
		/* create message */
		sprintf(message, "Matrix Multiplication MPI from process %d!", my_rank);
		dest = 0;
		/* use strlen+1 so that '\0' get transmitted */
		MPI_Send(message, strlen(message)+1, MPI_CHAR,
				dest, tag, MPI_COMM_WORLD);
	}
	else{																			//SE IL PROCESSORE È IL MASTER
		printf("Matrix Multiplication MPI From process 0: Num processes: %d\n",p);
		/*COSTRUZIONE MATRICI*/
		createMatrix(matrixA, SIZE);
		createMatrix(matrixB, SIZE);

		printf("First Matrix \n");
		printMatrix(matrixA, SIZE);

		printf("Second Matrix \n");
		printMatrix(matrixB, SIZE);

		if(p==1){												//SE C'È UN UNICO PROCESSORE
			for(i=0; i<SIZE; i++){
				for(j=0; j<SIZE; j++){
					for(k=0; k<SIZE; k++){
						sum = sum + matrixA[i][k]*matrixB[k][j];
					}
					matrixC[i][j] = sum;
					sum = 0;
				}
			}
			printf("Multiplication of the 2 matrix is:\n");
			printMatrix(matrixC, SIZE);
		}

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

/*FUNZIONE PER L'ALLOCAZIONE DI MATRICI*/
void allocateMatrix(int **matrix, int size){
	int i;

	for(i=0;i<SIZE;i++)
		matrix[i]=(int*)malloc(SIZE*sizeof(int));			//ALLOCAZIONE COLONNE
}

/*FUNZIONE PER LA CREAZIONE DELLE MATRICI A E B*/
void createMatrix(int **matrix, int size){
	int i,j;
	for(i = 0; i<size; i++){
		for(j=0; j<size; j++){
			matrix[i][j] = rand() % 10;						//Valori tra 0 e 9 esclusi
		}
	}
}
/*FUNZIONE PER LA STAMPA DELLE MATRICI*/
void printMatrix(int **matrix, int size){

	int i,j;
	for(i = 0; i<size; i++){
		for(j=0; j<size; j++){
			printf("%d\t",matrix[i][j]);
		}
		printf("\n");
	}
}
