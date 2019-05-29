#define _GNU_SOURCE
#include "disk_driver.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

// Apre il file (creandolo, se necessario), allocando lo spazio necessario sul disco e calcolando quanto deve essere grane la mappa se il file è stato appena creato.
// Compila un Disk Header e riempie la Bitmap della dimensione appropriata con tutti 0 (per denotare lo spazio libero)
// opens the file (creating it if necessary) allocates the necessary space on the disk calculates how big the bitmap should be
// If the file was new compiles a disk header, and fills in the bitmap of appropriate size with all 0 (to denote the free space)
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks) {

	// Calcoliamo quanti blocchi dovremo memorizzare nel disco
	// Se il numero dei blocchi è multiplo di 8, impostiamo num_blocks/8, altrimenti aggiungiamo 1 (per arrotondare per eccesso)
	int bitmap_entries;
	if((num_blocks*BLOCK_SIZE) % 8 == 0) {
		bitmap_entries = (num_blocks*BLOCK_SIZE) / 8; 
	}else{
		bitmap_entries = ((num_blocks*BLOCK_SIZE) / 8) + 1;
	}

	// Variabile in cui memorizzare il file descriptor che ci aiuterà ad utilizzare il file stesso
	int file;

	// Se il file esiste, calcoliamo alcuni dati, se non esiste, lo creiamo e inseriamo "Ciao mondo"
	if(!access(filename, F_OK)) { // se il file esiste
		file = open(filename, O_RDWR, 0666);
		if(!file) {
			printf("C'è stato un errore nell'apertura del file. Il programma è stato bloccato.");
			return;
		}
		disk->fd = file;
	}else{ // se il file è stato appena creato
		file = open(filename, O_CREAT | O_RDWR, 0666);
		if(!file) {
			printf("C'è stato un errore nell'apertura del file. Il programma è stato bloccato.");
			return;
		}
		disk->fd=file;

		// inizializzo il file per evitare "bus error"
		char * init = "\0";
		write(file, init, strlen(init)); 
		lseek(file, 0, SEEK_SET);
	}

	// Mi calcolo le dimensioni del file aperto
	struct stat file_stat;
	fstat(file, &file_stat);
	int file_size = file_stat.st_size-1;

	// Ottengo il contenuto del file
	char * file_content = malloc(file_size);
	read(file, file_content, file_size);

	// Creiamo un DiskHeader che andrà inserito nel DiskDriver
	//disk->header = (DiskHeader*) mmap(0, sizeof(DiskHeader) + bitmap_entries, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	disk->header = (DiskHeader*) malloc(sizeof(DiskHeader));
	disk->header->num_blocks = num_blocks;	// TODO: Se utilizzamo mmap, da questa riga in poi, il file viene sovrascritto, correggere. Con /0 non funziona
	disk->header->bitmap_blocks = num_blocks;
	disk->header->bitmap_entries = bitmap_entries;
	disk->header->free_blocks = num_blocks; // TODO: calcolare quanti sono i blocchi liberi

	// Creo una BitMap e memorizzo le informazioni e il contenuto del file aperto
	disk->bitmap = (BitMap*) malloc(sizeof(BitMap));
	disk->bitmap->num_bits = num_blocks * BLOCK_SIZE;
	disk->bitmap->entries = malloc(disk->bitmap->num_bits);
	
	// Imposto tutti i bit della BitMap a zero
	int i;
	for(i = 0; i <= num_blocks; i++) { 
		DiskDriver_freeBlock(disk, i);
	}

	// Sovrascrivo i dati del file nella BitMap
	for(i = 0; i < file_size; i++){
		disk->bitmap->entries[i] = file_content[i];
	}

	// Calcolo il primo blocco libero dopo aver assegnato il valore alle entries
	disk->header->first_free_block = DiskDriver_getFreeBlock(disk, 0);

	return;
}


// reads the block in position block_num, returns -1 if the block is free accrding to the bitmap 0 otherwise 
int DiskDriver_readBlock(DiskDriver* disk, void * dest, int block_num){

	// Se il blocco che si vuole leggere è vuoto, restituiamo un errore
	if(DiskDriver_getFreeBlock(disk,block_num) == block_num) return -3;

	// Se la funzione mi restituisce -1, restituisco un errore
	if(lseek(disk->fd, (block_num * BLOCK_SIZE) / 8, SEEK_SET) == -1) return -2;

	// Leggiamo il contenuto delle entries e lo memorizziamo in dest
	read(disk->fd, dest, strlen(disk->bitmap->entries));

	return 0;
}


// writes a block in position block_num, and alters the bitmap accordingly, returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver * disk, void * src, int block_num) {

	// Sposto il puntatore nel blocco "block_num". Se non posso, restituisco -1
	if(lseek(disk->fd, (block_num * BLOCK_SIZE) / 8, SEEK_SET) == -1) return -1;

	if(strlen(src) * 8 > (disk->header->num_blocks - block_num) * BLOCK_SIZE) return -1;

	// Scrivo src nell'entries
	int i;
	char * a = src;
	for(i = 0; i < strlen(src); i++) {
		disk->bitmap->entries[i] = a[i];
	}

	// Scrivo "src" nel file. Se non riesco, restituisco -1
  return write(disk->fd, src, strlen(src));
}


// frees a block in position block_num, and alters the bitmap accordingly, returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num) {
	if(block_num > disk->header->num_blocks) return -1;

	// Setto i bit del blocco "block_num" a 0
	int i;
	for(i = block_num * BLOCK_SIZE ; i < (block_num+1) * BLOCK_SIZE; i++){
		BitMap_set(disk->bitmap, i, 0);
	}

	// Incremento il numero di blocchi liberi nel DiskHeader
	disk->header->free_blocks++;

	// Nel caso in cui il blocco è precedente a quello salvato in DiskHeader lo cambio
	if(block_num < disk->header->first_free_block) disk->header->first_free_block = block_num;

	return 0;
}


// returns the first free block in the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start) {

	// Controlliamo ogni blocco di Disk e ogni bit della BitMap
	int i, j;
	for(i = start; i < disk->header->num_blocks; i++) {
		for(j = 0; j <= BLOCK_SIZE; j++){

			// Se arrivo alla fine del blocco e questo è libero ritorno i
			if(j == BLOCK_SIZE) return i;

			// Controllo ogni bit e se questo è 1 esco da questo blocco e passo al successivo
			if(BitMap_get(disk->bitmap, i*BLOCK_SIZE + j, 0) != i*BLOCK_SIZE + j) break;
		}
	}
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk) {

}
