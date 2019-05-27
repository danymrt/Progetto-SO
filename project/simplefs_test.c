#include "bitmap.c"
#include "disk_driver.c"
#include "simplefs.c"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int agc, char** argv) {

	// Test BitMap_create 
	printf("\n+++ Test DiskDriver_init(), BitMap_init(), DiskDriver_getFreeBlock(), BitMap_get()");
	DiskDriver disk;
	DiskDriver_init(&disk, "test.txt", 15);
	BitMap bitmap=*(disk.bitmap);
	printf("\n    BitMap creata e inizializzata correttamente");
	printf("\n    Primo blocco libero => %d", disk.header->first_free_block); // WORKING: correggere il risultato messo qui dentro dalla DiskDriver_getFreeBlock()

	// Test BitMap_blockToIndex
	int num = 4 * BLOCK_SIZE;
	printf("\n\n+++ Test BitMap_blockToIndex(%d)", num);
	BitMapEntryKey block = BitMap_blockToIndex(num);
	printf("\n    La posizione del blocco è %d, ovvero entry %d con sfasamento %d", num, block.entry_num, block.bit_num);
 
	// Test BitMap_indexToBlock
	printf("\n\n+++ Test BitMap_indexToBlock(block)");
	int posizione = BitMap_indexToBlock(block);
	printf("\n    Abbiamo la entry %d e lo sfasamento %d, ovvero la posizione %d", block.entry_num, block.bit_num, posizione);

	// Test BitMap_set
	printf("\n\n+++ Test BitMap_set()");
	printf("\n    La bitmap contiene: %s", disk.bitmap->entries);
	printf("\n    Eseguo BitMap_set(bitmap, 17, 1)...");
	BitMap_set(disk.bitmap, 17, 1);
	printf("\n    La bitmap contiene: %s", disk.bitmap->entries);


	// Test BitMap_get
	printf("\n\n+++ Test BitMap_get()");
	printf("\n    bitmap->entries contiene: %s", disk.bitmap->entries);
	printf("\n    bitmap->entries contiene, in binario: "); // TODO: Stampare la disk.bitmap->entries in binario
	int start = 6, status = 0; 
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 3, status = 1;
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 12, status = 0;
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 13, status = 1;
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));

	/*
	// Test DiskDriver_writeBlock
	printf("\n+++ Test DiskDriver_writeBlock()");
	int block_num=0;
	void* src = "ciao mondo";
	printf("\n    Il risultato della write con block_num=%d è: %d\n", block_num, DiskDriver_writeBlock(&disk,src,block_num));


	// Test DiskDriver_readBlock
	printf("\n+++ Test DiskDriver_readBlock()");
	void* dest;
	printf("\n    Il risultato della read con block_num=%d è: %d\n", block_num, DiskDriver_readBlock(&disk,dest,block_num));


	// Test DiskDriver_freeBlock
	printf("\n+++ Test DiskDriver_freeBlock()");
	printf("\n Il risultato della FreeBlock è: %d \n",DiskDriver_freeBlock(&disk,0));


	// Test DiskDriver_getFreeBlock
	printf("\n+++ Test DiskDriver_getFreeBlock()");
	printf("\n    Il risultato della getFreeBlock è: %d \n",DiskDriver_getFreeBlock(&disk,0));
	*/

	printf("\n\n");
}
