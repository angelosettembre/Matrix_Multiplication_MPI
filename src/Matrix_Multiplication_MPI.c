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


/*PROTOTIPI FUNZIONI*/
void allocateMatrix(int **matrix, int size);
void createMatrix(int **matrix, int size);
void initMatrixSend(int **matrix, int size);
void printMatrix(int **matrix, int size);

int main(int argc, char* argv[]){
	int SIZE;									/*Dimensione righe e colonne di ogni matrice*/
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int **matrixA, **matrixB, **matrixC; 					/*MATRICI*/
	int **matrixSend;
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

	SIZE = atoi(argv[1]);					//Dimensione matrice da riga di comando


	/*CONTROLLO SE LA DIMENSIONE DELLA MATRICE È DIVISIBILE PER IL NUM DI PROCESSORI*/
	if(p > SIZE){
		if(p % SIZE != 0){
			if(my_rank == 0){
				printf("Matrix is not divisible by number of processors \n\n");
				printf("----BYE----");
			}
			MPI_Finalize();
			return 0;
		}
	} else {
		if(SIZE % p != 0){
			if(my_rank == 0){
				printf("Matrix is not divisible by number of processors \n\n");
				printf("----BYE----");
			}
			MPI_Finalize();
			return 0;
		}
	}
	/*--------*/


	/*ALLOCAZIONE MATRICI (PUNTATORI DI PUNTATORI) NEL HEAP*/
	matrixA = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixA, SIZE);

	matrixB = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixB, SIZE);

	matrixC = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
	allocateMatrix(matrixC, SIZE);
	/*--------*/

	srand(time(NULL));										//SEME DELLA FUNZIONE rand()

	//ACCEDERE SIZE*i+j

	fromProcess = my_rank * SIZE/p;							//Porzione di matrice di ogni processore
	toProcess = (my_rank+1) * SIZE/p;

	if (my_rank ==0){																			//SE IL PROCESSORE È IL MASTER
		printf("Matrix Multiplication MPI From process 0: Num processes: %d\n",p);
		/*COSTRUZIONE MATRICI*/
		createMatrix(matrixA, SIZE);
		createMatrix(matrixB, SIZE);

		printf("Matrix A \n");
		printMatrix(matrixA, SIZE);
		printf("\n");

		printf("Matrix B \n");
		printMatrix(matrixB, SIZE);
		printf("\n");
	}

	if(p != 1){																			//SE IL NUMERO DI PROCESSI NON E' 1
		MPI_Bcast(&matrixB[0][0], SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD);				//INVIO MATRICE B TRAMITE UNA BROADCAST A TUTTI I PROCESSORI

		if(p != SIZE){
			matrixSend = (int **) malloc(SIZE*sizeof(int*));			//ALLOCAZIONE PER RIGHE DELLA MATRICE
			allocateMatrix(matrixSend, SIZE);
			initMatrixSend(matrixSend, SIZE);							//Inizializzazione matrice

			/*SUDDIVISIONE: ad ogni processo viene assegnato un certo numero di righe, (anche il master partecipa alla computazione)*/
			MPI_Scatter(*matrixA, SIZE*SIZE/p, MPI_INT, matrixSend[fromProcess], SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);			//INVIO RIGHE DELLA MATRICE (A) AD OGNI PROCESSO
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

			MPI_Gather(&matrixC[fromProcess][0], SIZE*SIZE/p, MPI_INT, &matrixC[0][0], SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);			//Ogni processo invia la propria porzione di matrice alla matrice C risultante

		} else {																											//SE IL NUMERO DI PROCESSORI È UGUALE ALLA DIMENSIONE DELLE MATRICI
			arraySend = (int*) malloc(SIZE*SIZE*sizeof(int));				//ALLOCAZIONE ARRAY DOVE OGNI PROCESSORE AVRA' UNA RIGA

			MPI_Scatter(*matrixA, SIZE*SIZE/p, MPI_INT, arraySend, SIZE*SIZE/p, MPI_INT, 0, MPI_COMM_WORLD);			//INVIO RIGHE MATRICE (A) AD OGNI PROCESSO

			printf("My rank (%d) ", my_rank);
			for(i = 0; i<SIZE*SIZE/p; i++){
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
		printf("\nThe multiplication between the two matrix is:\n");
		printMatrix(matrixC, SIZE);
		printf("\n\n");
	}

	/* shut down MPI */
	MPI_Finalize();

	return 0;
}

/*FUNZIONE PER L'ALLOCAZIONE DI MATRICI*/
void allocateMatrix(int **matrix, int size){
	int i;
	int *contiguousItems = (int *)malloc(size*size*sizeof(int));				//ALLOCAZIONE DI SIZE*SIZE ELEMENTI CONTIGUI

	for(i=0;i<size;i++)
		matrix[i]= &contiguousItems[i*size];			//SI RENDE LA MATRICE COME UN ARRAY
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

/*FUNZIONE PER LA INIZIALIZZAZIONE DI MATRIX SEND*/
void initMatrixSend(int **matrix, int size){
	int i,j;
	for(i = 0; i<size; i++){
		for(j=0; j<size; j++){
			matrix[i][j] = 0;						//Valori tra 0 e 9 esclusi
		}
	}
}

/*FUNZIONE PER LA STAMPA DELLE MATRICI*/
void printMatrix(int **matrix, int size){

	int i,j;
	for(i = 0; i<size; i++){
		printf("\n\t[");
		for(j=0; j<size; j++){
			printf(" %d ",matrix[i][j]);
		}
		printf("]");
	}
	printf("\n");
}
