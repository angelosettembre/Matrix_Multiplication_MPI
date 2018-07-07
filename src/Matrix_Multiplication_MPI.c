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
void printMatrix(int **matrix, int size);

int main(int argc, char* argv[]){
	int SIZE;									/*Dimensione righe e colonne di ogni matrice*/
	int  my_rank; /* rank of process */
	int  p;       /* number of processes */
	int **matrixA, **matrixB, **matrixC; 					/*MATRICI*/
	int i,j,k;
	int fromProcess, toProcess;
	int sum = 0;
	double startTime, endTime;								/*Tempo inizio-fine computazione totale*/
	double starTimeProcess, endTimeProcess;					/*Tempo inizio-fine computazione di ogni processo*/
	MPI_Datatype matrixType;								/*Tipo derivato per le matrici*/

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

	if (my_rank == 0){																			//SE IL PROCESSORE È IL MASTER
		printf("Matrix Multiplication MPI From process 0, Num processes: %d, Matrix size: %d\n", p, SIZE);
		/*COSTRUZIONE MATRICI*/
		createMatrix(matrixA, SIZE);
		createMatrix(matrixB, SIZE);

		printf("Matrix A \n");
		printMatrix(matrixA, SIZE);														//Stampa Matrice A
		printf("\n");

		printf("Matrix B \n");
		printMatrix(matrixB, SIZE);														//Stampa Matrice B
		printf("\n");
	}

	startTime = MPI_Wtime();														//Acquisizione del tempo di inizio della computazione

	/*Calcolo porzione di matrice di assegnata ad ogni processore*/
	fromProcess = my_rank * SIZE/p;
	toProcess = (my_rank+1) * SIZE/p;
	/*------*/

	if(p != 1){																			//SE IL NUMERO DI PROCESSI NON E' 1
		MPI_Bcast(&matrixB[0][0], SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD);				//PROCESSORE MASTER INVIA MATRICE B TRAMITE UNA BROADCAST A TUTTI I PROCESSORI

		/*SUDDIVISIONE: ad ogni processo viene assegnato un certo numero di righe, (anche il master partecipa alla computazione)*/
		MPI_Type_contiguous(SIZE*SIZE/p, MPI_INT, &matrixType);							/*Replicazione del tipo di dato (matrixType) in posizioni contigue della matrice*/
		MPI_Type_commit(&matrixType);

		if(my_rank == 0){
			printf("Portion assigned to each process: %d\n\n", SIZE/p);
		}

		/*PROCESSORE MASTER INVIA LE PORZIONI DELLA MATRICE (A) AD OGNI PROCESSO. OGNI PROCESSO AVRA' UN CERTO NUMERO DI RIGHE DA COMPUTARE*/
		MPI_Scatter(*matrixA, 1, matrixType, matrixA[fromProcess], 1, matrixType, 0, MPI_COMM_WORLD);

		/*STAMPA DELLA MATRICE TEMPORANEA DI OGNI PROCESSO*/
		/*printf("MATRIX Temp rank:%d \n", my_rank);
		for(i=0; i<SIZE; i++){
			for(j=0; j<SIZE; j++){
				printf("%d\t",matrixA[i][j]);
			}
			printf("\n");
		}*/
		/*-----*/
	}

	starTimeProcess = MPI_Wtime();															/*Acquisizione tempo di inizio del calcolo del prodotto*/
	/*CALCOLO MOLTIPLICAZIONE TRA MATRICE RICEVUTA E MATRICE B*/
	for(i=fromProcess; i<toProcess; i++){
		for(j=0; j<SIZE; j++){
			for(k=0; k<SIZE; k++){
				sum = sum + matrixA[i][k]*matrixB[k][j];
			}
			matrixC[i][j] = sum;
			sum = 0;
		}
	}
	/*-------*/
	endTimeProcess = MPI_Wtime();															/*Acquisizione tempo di fine del calcolo del prodotto*/
	printf("Elapsed time of computation for process %d is %f\n", my_rank, endTimeProcess - starTimeProcess);
	printf("\n");

	if(p != 1){
		/*Ogni processo dopo aver effettuato la moltiplicazione con la propria porzione di matrice, invia le righe che si è calcolato alla matrice risultante (C)*/
		MPI_Gather(&matrixC[fromProcess][0], 1, matrixType, &matrixC[0][0], 1, matrixType, 0, MPI_COMM_WORLD);
	}

	endTime = MPI_Wtime();													//Acquisizione tempo di fine della computazione

	/*IL MASTER MOSTRERA' LA MATRICE FINALE (C)*/
	if (my_rank == 0) {
		printf("\nThe multiplication between the two matrix is:\n");
		printMatrix(matrixC, SIZE);
		printf("\n\n");
		printf( "Elapsed time is %f\n", endTime - startTime);
	}

	/* shut down MPI */
	if(p != 1){
		MPI_Type_free(&matrixType);						//Viene liberata la memoria allocata per il datatype creato
	}

	/*DEALLOCAZIONE PUNTATORI*/
	free(matrixA);
	free(matrixB);
	free(matrixC);
	/*-------*/

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
			matrix[i][j] = rand() % 10;						//Valori tra 0 e 10 escluso
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
