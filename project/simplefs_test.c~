#include "bitmap.c"
#include "disk_driver.c"
#include "simplefs.c"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>     

void stampa_in_binario(char* stringa) {
	int i, j;
	for(i = 0; i < strlen(stringa); i++) {
		char c = stringa[i];
		for (j = 0; j <= 7; j++) {
	      printf("%d", !!((c >> j) & 0x01));
	  }
	}
}

int main(int agc, char** argv) {

	// Test BitMap_create   
	printf("\n+++ Test DiskDriver_init()");
	printf("\n+++ Test BitMap_init()");
	printf("\n+++ Test DiskDriver_getFreeBlock()");
	printf("\n+++ Test BitMap_get()");
	DiskDriver disk;
	DiskDriver_init(&disk, "test.txt", 3);
	BitMap bitmap=*(disk.bitmap); 
	printf("\n    BitMap creata e inizializzata correttamente");
	printf("\n    Primo blocco libero => %d", disk.header->first_free_block); 


	// Test BitMap_blockToIndex
	int num = 4 * BLOCK_SIZE;
	printf("\n\n+++ Test BitMap_blockToIndex(%d)", num);   
	BitMapEntryKey block = BitMap_blockToIndex(num);
	printf("\n    La posizione del blocco è %d, ovvero entry %d con sfasamento %d", num, block.entry_num, block.bit_num);
 

	// Test BitMap_indexToBlock 
	printf("\n\n+++ Test BitMap_indexToBlock(block)");
	int posizione = BitMap_indexToBlock(block); 
	printf("\n    Abbiamo la entry %d e lo sfasamento %d, ovvero la posizione %d", block.entry_num, block.bit_num, posizione);


	// Test DiskDriver_freeBlock
	printf("\n\n+++ Test DiskDriver_freeBlock()");
	printf("\n    Libero il blocco %d, la funzione ritorna: %d",0,DiskDriver_freeBlock(&disk,0));

 
	// Test DiskDriver_writeBlock  
	printf("\n\n+++ Test DiskDriver_writeBlock()");
	printf("\n    Il risultato della writeBlock(\"Ciao\", 0) è %d", DiskDriver_writeBlock(&disk, "Ciao", 0));

 
	// Test DiskDriver_readBlock
	printf("\n\n+++ Test DiskDriver_readBlock()");
	void * dest = malloc(BLOCK_SIZE);
	printf("\n    Controlliamo tramite una readBlock(dest, 0)   => %d", DiskDriver_readBlock(&disk, dest, 0));
	printf("\n    Dopo la readBlock, la dest contiene           => %s", (char *) dest);


	// Test BitMap_set
	printf("\n\n+++ Test BitMap_set()"); 
	printf("\n    Prima     =>   %s\n                   ", disk.bitmap->entries);
	stampa_in_binario(disk.bitmap->entries);
	BitMap_set(disk.bitmap, 17, 1);
	printf("\n    Dopo (17) =>   %s\n                   ", disk.bitmap->entries);
	stampa_in_binario(disk.bitmap->entries);

  
	// Test BitMap_get
	printf("\n\n+++ Test BitMap_get()");  
	printf("\n    bitmap->entries contiene: %s", disk.bitmap->entries);
	printf("\n    bitmap->entries contiene, in binario: "); 
	stampa_in_binario(disk.bitmap->entries);
	int start = 6, status = 0;    
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 3, status = 1; 
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 12, status = 0;
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(disk.bitmap, start, status));
	start = 13, status = 1;
	printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d\n", start, status, BitMap_get(disk.bitmap, start, status));


	printf("\n\n");
}
