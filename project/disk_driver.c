#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

// Apre il file (creandolo, se necessario), allocando lo spazio necessario sul disco e calcolando quanto deve essere grane la mappa se il file è stato appena creato.
// Compila un Disk Header e riempie la Bitmap della dimensione appropriata con tutti 0 (per denotare lo spazio libero)
// opens the file (creating it if necessary) allocates the necessary space on the disk calculates how big the bitmap should be
// If the file was new compiles a disk header, and fills in the bitmap of appropriate size with all 0 (to denote the free space)
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks) {
	// Apriamo il file ricevuto come parametro
	int file = open(filename, O_CREAT | O_RDWR, 0666);
	// Se il file non esiste o non viene aperto, blocchiamo la funzione
	if(!file) return;
	disk->fd=file;

	// Creiamo un DiskHeader che andrà inserito nel DiskDriver
	DiskHeader header;
	// Impostiamo il numero dei blocchi all'interno del DiskHeader
	header.num_blocks = num_blocks;
	// Se il numero dei blocchi è multiplo di 8, impostiamo num_blocks/8, altrimenti aggiungiamo 1 (per arrotondare per eccesso)
	if(num_blocks % 8 == 0) {
		header.bitmap_blocks = num_blocks / 8; //TODO
	}else{
		header.bitmap_blocks = (num_blocks / 8) + 1;
	}
	//
	header.bitmap_entries = ((disk->bitmap->num_bits)/8)+1;

	header.free_blocks = 0; //TODO
	header.first_free_block = 0; //TODO
	disk->header = &header;
	BitMap bmap;
	BitMap_init(&bmap,720,"000000000000000000000000000000000000000000000000000000000000000000000000000000");
	disk->bitmap = &bmap;

}


// reads the block in position block_num
// returns -1 if the block is free accrding to the bitmap
// 0 otherwise

//sia off_set che ssize_t sono long ovvero 8 byte
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){
	//lseek is a system call that is used to change the location of the read/write pointer of a file descriptor
	off_t start=lseek(disk->fd,block_num,SEEK_SET); //block_num è the offset of the pointer (measured in bytes).
	if(start==-1) return -1;
	//printf("\n start=%ld\n", start);

	int rz= read(disk->fd,dest ,BLOCK_SIZE);

	if(rz!=0) return -1;
	else return 0;
}


// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num) {
	//sposto il puntatore nel blocco che voglio io
	off_t start=lseek(disk->fd,block_num,SEEK_SET);
	if(start==-1) return -1;

	int sz = write(disk->fd,src,BLOCK_SIZE);
  if(sz==-1) return -1;

	if(sz!=BLOCK_SIZE) return -1;
	else return 0;
}

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num) {
	BitMap* bmap= disk->bitmap;
	int i;
	int indice= block_num*BLOCK_SIZE;
	for(i=indice ; i< indice+BLOCK_SIZE; i++){
		int n=BitMap_set(bmap,i,0);
		if(n<0) return -1;
	}
	return 0;

}

// returns the first free blockin the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start) {
	BitMap* bmap= disk->bitmap;
	int num=disk->header->num_blocks;
	int i,j;
	int indice= start*BLOCK_SIZE;
	for(i=0; i< num * BLOCK_SIZE; i=i+BLOCK_SIZE){
		for(j=0; j<=BLOCK_SIZE; j++){
			if(j==BLOCK_SIZE){
				return i;
			}
			int n=BitMap_get(bmap,j+i,0);
			if(n!=j+i){
				break;
			}
		}
	}
	printf("\n i=%d j= %d \n",i,j);
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk) {
}
