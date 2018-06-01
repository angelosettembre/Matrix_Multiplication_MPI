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
void allocateMatrix(int **matrix);
void createMatrix(int **matrix);
void printMatrix(int **matrix);

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
	int *arraySend;
	int sendCount, receiveCount;
	MPI_Status status ;   /* return status for receive */

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	/*CONTROLLO SE LA DIMENSIONE DELLA MATRICE È DIVISIBILE PER IL NUM DI PROCESSORI*/
	if(p > SIZE){
		if(p % SIZE != 0){
			printf("Matrix is not divisible by number of processors \n");
			MPI_Finalize();
			return 0;
		}
	} else {
		if(SIZE % p != 0){
			printf("Matrix is not divisible by number of processors \n");
			MPI_Finalize();
			return 0;
		}
	}
	/*--------*/


	/*ALLOCAZIONE MATRICI (PUNTATORI DI PUNTATORI) NEL HEAP*/
	matrixA = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixA);

	matrixB = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixB);

	matrixC = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixC);
	/*--------*/

	srand(time(NULL));										//SEME DELLA FUNZIONE rand()

	arraySend = malloc(SIZE*sizeof(int));				//ALLOCAZIONE RIGHE

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
		createMatrix(matrixA);
		createMatrix(matrixB);

		printf("Matrix A \n");
		printMatrix(matrixA);
		printf("\n");

		printf("Matrix B \n");
		printMatrix(matrixB);
		printf("\n");

		/*
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
		 */
	}
	sendCount = SIZE;
	receiveCount = SIZE;

	MPI_Bcast(&matrixB[0][0], SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD);				//INVIO MATRICE B TRAMITE BROADCAST A TUTTI I PROCESSORI
	printf("matrix B rank:%d \n", my_rank);
	printMatrix(matrixB);

	/*MPI_Scatter(*matrixA, SIZE/p, MPI_INT, arraySend[my_rank*SIZE/p], SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);
	printf("MATRIX AAA rank:%d \n", my_rank);
	for (i=0; i<SIZE; i++) {
		printf("\n\t| ");
		for (j=0; j<SIZE; j++)
			printf("%2d ", matrixA[i][j]);
		printf("|");
	}
	printf("\n");
	 */
	/*
	MPI_Scatter(*matrixA, sendCount, MPI_INT, arraySend, receiveCount, MPI_INT, 0, MPI_COMM_WORLD);
	printf("Rank = %d \n", my_rank);
	for(i=0; i<SIZE; i++){
		printf(" %d ",arraySend[i]);
	}
	printf("\n");
	 */

	/* shut down MPI */
	MPI_Finalize();

	return 0;
}

/*FUNZIONE PER L'ALLOCAZIONE DI MATRICI*/
void allocateMatrix(int **matrix){
	int i;
	int *contiguousItems = (int *)malloc(SIZE*SIZE*sizeof(int));				//ALLOCAZIONE DI SIZE*SIZE ELEMENTI CONTIGUI

	for(i=0;i<SIZE;i++)
		matrix[i]= &contiguousItems[i*SIZE];			//SI RENDE LA MATRICE COME UN ARRAY
}

/*FUNZIONE PER LA CREAZIONE DELLE MATRICI A E B*/
void createMatrix(int **matrix){
	int i,j;
	for(i = 0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			matrix[i][j] = rand() % 10;						//Valori tra 0 e 9 esclusi
		}
	}
}

/*FUNZIONE PER LA STAMPA DELLE MATRICI*/
void printMatrix(int **matrix){

	int i,j;
	for(i = 0; i<SIZE; i++){
		printf("\n\t[");
		for(j=0; j<SIZE; j++){
			printf(" %d ",matrix[i][j]);
		}
		printf("]");
	}
	printf("\n");
}
