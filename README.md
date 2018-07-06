# Parallel Matrix Multiplication

### Programmazione Concorrente, Parallela e su Cloud

##### Università degli Studi di Salerno									Anno accademico *2017/2018*

**Prof. Vittorio Scarano**

**Dott. Carmine Spagnuolo**

**Studente: Angelo Settembre**

------

### Problem statement

------

In questo progetto si vuole risolvere il problema del calcolo della moltiplicazione tra matrici. Date due matrici A e B di dimensione *(n x n)*, viene effettuato il prodotto righe per colonne tra le due matrici. Il risultato verrà riportato nella matrice risultato C. 

![1024px-Matrix_multiplication_diagram.svg](/home/angelo/eclipse-workspace/Matrix_Multiplication_MPI/img/1024px-Matrix_multiplication_diagram.svg.png)

------

### Soluzione proposta

L'obiettivo era quello di parallelizzare la moltiplicazione tra matrici utilizzando MPI. La soluzione proposta considera solo matrici quadrate **N x N** dove la dimensione di ogni matrice deve essere divisibile per il numero di processori **p**. Il programma prende in input la taglia delle matrici, costruendo le matrici A e B in maniera randomica (range di valori tra 0 e 9). In outuput fornirà la matrice risultato C. La comunicazione per i processori è stata realizzata mediante l'utilizzo di operazioni collettive come **MPI_Bcast**, **MPI_Scatter**, **MPI_Gather**. Vista la natura del problema, la soluzione che è stata attuata prevede il partizionamento della matrice A in righe, tutte composte dallo stesso numero di colonne, distribuite tra i vari processori. 

![](/home/angelo/eclipse-workspace/Matrix_Multiplication_MPI/img/division.jpg)

Ogni processore quindi avrà (**numero di righe / numero di processori**) righe. Poiché la dimensione della matrice è divisibile per il numero di processori (**SIZE / p**), ogni processore avrà un numero di righe equo. La matrice B, invece, verrà ricevuta da tutti i processori, in questo modo ogni processore effettua il prodotto tra la porzione della matrice A e le colonne della matrice B.

## Implementazione
### Allocazione e costruzione matrici
Inizialmente, vengono allocate dinamicamente le matrici nel heap: ogni matrice verrà allocata come un array di puntatori con blocchi contigui di memoria.

![](/home/angelo/eclipse-workspace/Matrix_Multiplication_MPI/img/matrix_allocation.png)

```
/*ALLOCAZIONE MATRICI (PUNTATORI DI PUNTATORI) NEL HEAP*/
matrixA = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
allocateMatrix(matrixA, SIZE);

matrixB = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
allocateMatrix(matrixB, SIZE);

matrixC = (int **) malloc(SIZE*sizeof(int*));				//ALLOCAZIONE PER RIGHE
allocateMatrix(matrixC, SIZE);
/*--------*/
...
...
...
void allocateMatrix(int **matrix, int size){
	int i;
	int *contiguousItems = (int *)malloc(size*size*sizeof(int));	//ALLOCAZIONE DI SIZE*SIZE ELEMENTI CONTIGUI

	for(i=0;i<size;i++)
		matrix[i]= &contiguousItems[i*size];			//SI RENDE LA MATRICE COME UN ARRAY
}
```
Dopo la allocazione, il processore MASTER inizializza le matrici A e B con valori randomici (tra 0 e 9):

```
if (my_rank == 0){							
		printf("Matrix Multiplication MPI From process 0, Num processes: %d, Matrix size: %d\n", p, SIZE);
		/*COSTRUZIONE MATRICI*/
		createMatrix(matrixA, SIZE);
		createMatrix(matrixB, SIZE);

		printf("Matrix A \n");
		printMatrix(matrixA, SIZE);				 //Stampa Matrice A
		printf("\n");

		printf("Matrix B \n");
		printMatrix(matrixB, SIZE);			     //Stampa Matrice B
		printf("\n");
}
...
...
...
void createMatrix(int **matrix, int size){
	int i,j;
	for(i = 0; i<size; i++){
		for(j=0; j<size; j++){
			matrix[i][j] = rand() % 10;		   //Valori tra 0 e 10 escluso
		}
	}
}
```
### Calcolo indici matrice
Una volta allocate e costruite le matrici A e B, vengono calcolati gli indici di inizio e di fine di ogni porzione di matrice assegnata ad un processore.

```
fromProcess = my_rank * SIZE/p;
toProcess = (my_rank+1) * SIZE/p;
```
### Invio matrice B in broadcast
Il processore MASTER invia in broadcast la matrice B a tutti i processori. In questo modo ogni processore (compreso il MASTER) ha la matrice B con cui dopo può effettuare la moltiplicazione. Viene utilizzata la routine di comunicazione collettiva **MPI_Bcast**. In questo modo si invieranno **SIZE*SIZE** elementi contigui.

```
MPI_Bcast(&matrixB[0][0], SIZE*SIZE, MPI_INT, 0, MPI_COMM_WORLD);
```

### Costruzione e allocazione data type
Si definisce un tipo derivato per le matrici. Si utilizza la routine **MPI_****Data_Type**.

```
MPI_Datatype matrixType;
...
...
MPI_Type_contiguous(SIZE*SIZE/p, MPI_INT, &matrixType);
MPI_Type_commit(&matrixType);
```

Una volta costruito il data type, viene allocato utilizzando la routine **MPI_****Type_contiguous** dove viene replicato *matrixType* in **SIZE * SIZE / p** posizioni contigue.

![](/home/angelo/eclipse-workspace/Matrix_Multiplication_MPI/img/data-contiguous.jpg)

La scelta è ricaduta sulla dimensione **SIZE * SIZE / p** poiché ciascun processore deve avere la propria porzione di sottomatrice, in maniera tale che ognuno di essi può effettuare la moltiplicazione equamente.

![](/home/angelo/eclipse-workspace/Matrix_Multiplication_MPI/img/blasmatrix.png)

### Invio righe matrice A