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
	if((num_blocks) % 8 == 0) {
		bitmap_entries = (num_blocks) / 8; 
	}else{
		bitmap_entries = ((num_blocks) / 8) + 1;
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

		// alloco la memoria necessaria al file per evitare "bus error"
		int ret = posix_fallocate(file, 0, sizeof(DiskHeader) + bitmap_entries + num_blocks*BLOCK_SIZE);
		disk->fd = file;
		disk->header = (DiskHeader*) mmap(0, sizeof(DiskHeader) + bitmap_entries + ((num_blocks*BLOCK_SIZE) / 8), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
		
	}else{ // se il file è stato appena creato
	
		file = open(filename, O_CREAT | O_RDWR, 0666);

		if(!file) {
			printf("C'è stato un errore nell'apertura del file. Il programma è stato bloccato.");
			return;
		}

		disk->fd=file;

		// alloco la memoria necessaria al file per evitare "bus error"
		int ret = posix_fallocate(file, 0, sizeof(DiskHeader) + bitmap_entries + num_blocks*BLOCK_SIZE);

		// Creiamo un DiskHeader che andrà inserito nel DiskDriver
		disk->header = (DiskHeader*) mmap(0, sizeof(DiskHeader) + bitmap_entries + ((num_blocks*BLOCK_SIZE) / 8), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
		disk->header->num_blocks = num_blocks;
		disk->header->bitmap_blocks = num_blocks;
		disk->header->bitmap_entries = bitmap_entries;
		disk->header->free_blocks = num_blocks;

		lseek(file, 0, SEEK_SET);

	}

/*
	// Mi calcolo le dimensioni del file aperto
	struct stat file_stat;
	fstat(file, &file_stat);
	int file_size = file_stat.st_size-1;

	// Ottengo il contenuto del file
	char * file_content = malloc(file_size);
	read(file, file_content, file_size);
*/

	// Creiamo un DiskHeader che andrà inserito nel DiskDriver
	/*disk->header = (DiskHeader*) mmap(0, sizeof(DiskHeader) + bitmap_entries + ((num_blocks*BLOCK_SIZE) / 8), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	disk->header->num_blocks = num_blocks;
	disk->header->bitmap_blocks = num_blocks;
	disk->header->bitmap_entries = bitmap_entries;
	disk->header->free_blocks = file_size % BLOCK_SIZE == 0 ? num_blocks - (file_size / BLOCK_SIZE) : num_blocks - (file_size / BLOCK_SIZE) + 1; */

	// Memorizzo in bitmap_data il puntatore alla mmap saltando lo spazio dedicato a DiskHeader
	disk->bitmap_data = (char *) disk->header + sizeof(DiskHeader);
	
/*
	// Nel caso in cui il file è vuoto imposto tutti i bit della BitMap a zero
	int i;
	if(file_size == 0) {
		for(i = 0; i <= num_blocks; i++) { 
			DiskDriver_freeBlock(disk, i);
		}
	}
	// Sovrascrivo i dati del file già presenti
	int start= disk->header->bitmap_entries;
	for(i = start; i < start + file_size; i++){
		disk->bitmap_data[i] = file_content[i-start];
	}
*/	
	// Calcolo il primo blocco libero dopo aver assegnato il valore alle entries
	disk->header->first_free_block = DiskDriver_getFreeBlock(disk, 0);

	return;
}


// reads the block in position block_num, returns -1 if the block is free accrding to the bitmap 0 otherwise 
int DiskDriver_readBlock(DiskDriver* disk, void * dest, int block_num){

	BitMap bitmap;
	bitmap.num_bits = disk->header->bitmap_blocks;
	bitmap.entries = disk->bitmap_data;

	// Se il blocco che si vuole leggere è vuoto, restituiamo un errore
	if(BitMap_get(&bitmap, block_num, 0) == block_num) return -1;

	// Sposto il puntatore nel blocco "block_num". Se non posso, restituisco -1
	int start = sizeof(DiskHeader) + disk->header->bitmap_entries + block_num * BLOCK_SIZE;
	if(lseek(disk->fd, start , SEEK_SET) == -1) return -1;

	// Leggiamo il contenuto delle entries e lo memorizziamo in dest
	read(disk->fd, dest, BLOCK_SIZE);

	return 0;
}


// writes a block in position block_num, and alters the bitmap accordingly, returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver * disk, void * src, int block_num) {

	BitMap bitmap;
	bitmap.num_bits = disk->header->bitmap_blocks;
	bitmap.entries = disk->bitmap_data;

	// Sposto il puntatore nel blocco "block_num". Se non posso, restituisco -1
	int start = sizeof(DiskHeader) + disk->header->bitmap_entries + block_num * BLOCK_SIZE;
	if(lseek(disk->fd, start , SEEK_SET) == -1) return -1;

	if(strlen(src) * 8 > BLOCK_SIZE) return -1;

	// Scrivo che il blocco è occupato
	BitMap_set(&bitmap, block_num, 1);

	// Scrivo "src" nel file. Se non riesco, restituisco -1
	int write_size = strlen(src) > BLOCK_SIZE / 8 ? BLOCK_SIZE / 8 : strlen(src);
  return write(disk->fd, src, write_size);
}


// frees a block in position block_num, and alters the bitmap accordingly, returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num) {
	if(block_num > disk->header->num_blocks) return -1;

	BitMap bitmap;
	bitmap.num_bits = disk->header->bitmap_blocks;
	bitmap.entries = disk->bitmap_data;

	// Imposto il blocco come libero nella BitMap
	BitMap_set(&bitmap, block_num, 0);

	// Incremento il numero di blocchi liberi nel DiskHeader
	disk->header->free_blocks++;

	// Nel caso in cui il blocco è precedente a quello salvato in DiskHeader lo cambio
	if(block_num < disk->header->first_free_block) disk->header->first_free_block = block_num;

	return 0;
}


// returns the first free block in the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start) {
	
	BitMap bitmap;
	bitmap.num_bits = disk->header->bitmap_blocks;
	bitmap.entries = disk->bitmap_data;
	
	// Controllo che l'indice start non sia maggiore dei blocchi disponibili
	if(start > disk->header->num_blocks) return -1;

	//Controllo che DiskHeader sia inizzializzato
	if(disk->header->num_blocks <= 0 ) return -1;

	// Controlliamo nella BitMap quale è il primo blocco libero
	return BitMap_get(&bitmap, start, 0);
	
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk) {

}
