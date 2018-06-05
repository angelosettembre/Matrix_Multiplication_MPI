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
int matrixSend[SIZE][SIZE];

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
	int **matrixA, **matrixB, **matrixC; 					/*MATRICI*/
	int i,j,k;
	int fromProcess, toProcess;
	int sum = 0;
	int *arraySend;
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
			if(my_rank == 0){
				printf("Matrix is not divisible by number of processors \n");
				printf("----BYE----");
			}
			MPI_Finalize();
			return 0;
		}
	} else {
		if(SIZE % p != 0){
			if(my_rank == 0){
				printf("Matrix is not divisible by number of processors \n");
				printf("----BYE----");
			}
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

	//ACCEDERE SIZE*i+j

	fromProcess = my_rank * SIZE/p;
	toProcess = (my_rank+1) * SIZE/p;

	if (my_rank ==0){																			//SE IL PROCESSORE È IL MASTER
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
	}

	if(p != 1){																			//SE IL NUMERO DI PROCESSI NON E' 1
		MPI_Bcast(&matrixB[0][0], SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD);				//INVIO MATRICE B TRAMITE BROADCAST A TUTTI I PROCESSORI
		//printf("matrix B rank:%d \n", my_rank);
		//printMatrix(matrixB);

		if(p == 2){																													//SE CI SONO 2 PROCESSORI
			MPI_Scatter(*matrixA, SIZE*SIZE/p, MPI_INT, matrixSend[fromProcess], SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);			//INVIO RIGHE MATRICE A AD OGNI PROCESSO
			printf("MATRIX Temp rank:%d \n", my_rank);
			for(i=0; i<SIZE; i++){
				for(j=0; j<SIZE; j++){
					printf("%d\t",matrixSend[i][j]);
				}
				printf("\n");
			}

			/*CALCOLO MOLTIPLICAZIONE TRA MATRICE RICEVUTA E MATRICE B*/
			for(i=fromProcess; i<toProcess; i++){
				for(j=0; j<SIZE; j++){
					for(k=0; k<SIZE; k++){
						sum = sum + matrixSend[i][k]*matrixB[k][j];
					}
					matrixC[i][j] = sum;
					sum = 0;
				}
			}
			/*-------*/

			MPI_Gather(&matrixC[fromProcess][0], SIZE*SIZE/p, MPI_INT, &matrixC[0][0], SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);

		} else {
			arraySend = (int*) malloc(SIZE*SIZE*sizeof(int));				//ALLOCAZIONE ARRAY

			MPI_Scatter(*matrixA, SIZE*SIZE/p, MPI_INT, arraySend, SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);			//INVIO RIGHE MATRICE A AD OGNI PROCESSO

			printf("My rank (%d) ", my_rank);
			for(i = 0; i<SIZE; i++){
				printf(" %d ",arraySend[i]);
			}
			printf("\n");

			/*CALCOLO MOLTIPLICAZIONE TRA ARRAY E MATRICE B*/
			for(i=fromProcess; i<toProcess; i++){
				for(j=0; j<SIZE; j++){
					for(k=0; k<SIZE; k++){
						sum = sum + arraySend[k]*matrixB[k][j];
					}
					matrixC[i][j] = sum;
					sum = 0;
				}
			}
			/*-------*/

			MPI_Gather(&matrixC[fromProcess][0], SIZE*SIZE/p, MPI_INT, &matrixC[0][0], SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);
		}
	} else {																		//SE C'È UN UNICO PROCESSORE
		/*CALCOLO MOLTIPLICAZIONE TRA MATRICE A E MATRICE B*/
		for(i=0; i<SIZE; i++){
			for(j=0; j<SIZE; j++){
				for(k=0; k<SIZE; k++){
					sum = sum + matrixA[i][k]*matrixB[k][j];
				}
				matrixC[i][j] = sum;
				sum = 0;
			}
		}
		/*-------*/
	}

	if (my_rank == 0) {
		printf("The multiplication between the two matrix is:\n");
		printMatrix(matrixC);
		printf("\n\n");
	}

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
