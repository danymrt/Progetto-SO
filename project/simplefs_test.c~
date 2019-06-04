#include "bitmap.c" 
#include "disk_driver.c"
#include "simplefs.c"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0

// TEST
// 1 = BitMap
// 2 = DiskDriver
// 3 = SimpleFS
int test = 3;
int use_global_test = FALSE;

void stampa_in_binario(char* stringa) {
	int i, j;
	for(i = 0; i < strlen(stringa); i++) {
		char c = stringa[i];
		for (j = 7; j >= 0; j--) {
	      printf("%d", !!((c >> j) & 0x01)); 
	  }
	} 
}

int count_blocks(int num_bytes) {
	return num_bytes % BLOCK_SIZE == 0 ? num_bytes / BLOCK_SIZE : ( num_bytes / BLOCK_SIZE ) + 1;
}

int space_in_dir(int * file_blocks, int dim) {
	int i = 0;
	int free_spaces = 0;
	while(i < dim) {
		if(*file_blocks == 0) free_spaces++;
		file_blocks++;
		i++;
	}
	return free_spaces;
}

int main(int agc, char** argv) {

	if(!test) {
		printf("\nCosa vuoi testare?\n1 = BitMap\n2 = DiskDriver\n3 = SimpleFS\n\n >>> ");
	  scanf("%d", &test);
	}

	if(test == 1) {

		// Test BitMap_blockToIndex
		int num = 765;
		printf("\n+++ Test BitMap_blockToIndex(%d)", num);   
		BitMapEntryKey block = BitMap_blockToIndex(num);
		printf("\n    La posizione del blocco è %d, ovvero la entry %d al bit %d", num, block.entry_num, block.bit_num);
	 
		// Test BitMap_indexToBlock 
		printf("\n\n+++ Test BitMap_indexToBlock(block)");
		int posizione = BitMap_indexToBlock(block); 
		printf("\n    Abbiamo la entry %d e il bit %d, ovvero la posizione %d", block.entry_num, block.bit_num, posizione);

		// Test BitMap_set
		DiskDriver disk;
		BitMap bitmap;
		if(use_global_test) {
			DiskDriver_init(&disk, "test/test.txt", 15); 
		}else{
			char disk_filename[255];
			sprintf(disk_filename, "test/%d.txt", time(NULL));
			DiskDriver_init(&disk, disk_filename, 15); 
		}
		bitmap.num_bits = disk.header->bitmap_entries * 8;
		bitmap.entries = disk.bitmap_data;
		printf("\n\n+++ Test BitMap_set()");
		printf("\n+++ Test DiskDriver_init(disk, \"test.txt\", 15)");
		printf("\n    Prima => ");
		stampa_in_binario(bitmap.entries);
		printf("\n    Bitmap_set(6, 1) => %d", BitMap_set(&bitmap, 6, 1));
		printf("\n    Dopo  => ");
		stampa_in_binario(bitmap.entries);

		// Test BitMap_get
		printf("\n\n+++ Test BitMap_get()");
		int start = 6, status = 0;    
		printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(&bitmap, start, status));
		start = 2, status = 1;
		printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(&bitmap, start, status));
		start = 11, status = 0;
		printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(&bitmap, start, status));
		start = 13, status = 1;
		printf("\n    Partiamo dalla posizione %d e cerchiamo %d => %d", start, status, BitMap_get(&bitmap, start, status));

	}else if(test == 2) {

		// Test DiskDriver_init   
		printf("\n+++ Test DiskDriver_init()");
		printf("\n+++ Test DiskDriver_getFreeBlock()");
		printf("\n+++ Test BitMap_get()");
		DiskDriver disk;
		if(use_global_test) {
			DiskDriver_init(&disk, "test/test.txt", 15); 
		}else{
			char disk_filename[255];
			sprintf(disk_filename, "test/%d.txt", time(NULL));
			DiskDriver_init(&disk, disk_filename, 15); 
		}
		BitMap bitmap;
		bitmap.num_bits = disk.header->bitmap_entries * 8;
		bitmap.entries = disk.bitmap_data;
		printf("\n    BitMap creata e inizializzata correttamente");
		printf("\n    Primo blocco libero => %d", disk.header->first_free_block); 

		// Test DiskDriver_writeBlock  
		printf("\n\n+++ Test DiskDriver_writeBlock()");
		printf("\n+++ Test DiskDriver_flush()");
		printf("\n    Prima => ");
		stampa_in_binario(bitmap.entries);
		printf("\n    Il risultato della writeBlock(\"Ciao\", 4) è %d", DiskDriver_writeBlock(&disk, "Ciao", 4));
		printf("\n    Dopo  => ");
		stampa_in_binario(bitmap.entries);

		// Test DiskDriver_readBlock
		printf("\n\n+++ Test DiskDriver_readBlock()");
		void * dest = malloc(BLOCK_SIZE);
		printf("\n    Controlliamo tramite una readBlock(dest, 4)   => %d", DiskDriver_readBlock(&disk, dest, 4));
		printf("\n    Dopo la readBlock, la dest contiene            => %s", (char *) dest);

		// Test DiskDriver_freeBlock
		printf("\n\n+++ Test DiskDriver_freeBlock()");
		printf("\n    Prima => ");
		stampa_in_binario(bitmap.entries);
		printf("\n    Libero il blocco %d, la funzione ritorna: %d",6,DiskDriver_freeBlock(&disk,6));
		printf("\n    Dopo  => ");
		stampa_in_binario(bitmap.entries);

	}else if(test == 3) {

		// Test SimpleFS_init
		printf("\n+++ Test SimpleFS_init()");
		printf("\n+++ Test SimpleFS_format()");
		SimpleFS fs;
		DiskDriver disk;
		if(use_global_test) {
			DiskDriver_init(&disk, "test/test.txt", 15); 
		}else{
			char disk_filename[255];
			sprintf(disk_filename, "test/%d.txt", time(NULL));
			DiskDriver_init(&disk, disk_filename, 15); 
		}
		DirectoryHandle * dir_handle = SimpleFS_init(&fs, &disk);
		printf("\n    File System creato e inizializzato correttamente");
		printf("\n    BitMap attuale: ");
		stampa_in_binario(disk.bitmap_data);

		// Test SimpleFS_createFile
		printf("\n\n+++ Test SimpleFS_createFile()");
		int i, num_file = 4;
		for(i = 0; i < num_file; i++) {
			char filename[255];
			sprintf(filename, "prova_%d.txt", dir_handle->dcb->num_entries);
			if(SimpleFS_createFile(dir_handle,filename) != NULL)
				printf("\n    File %s creato correttamente", filename);
			else
				printf("\n    Errore nella creazione di %s", filename);
		}

	 	// Test SimpleFS_mkDir
		printf("\n\n+++ Test SimpleFS_mkDir()");
		printf("\n    Il risultato della SimpleFS_mkDir(dh, \"pluto\") è: %d", SimpleFS_mkDir(dir_handle, "pluto"));

		printf("\n\n+++ Test SimpleFS_readDir()");
		printf("\n    Nella cartella ci sono %d elementi:", dir_handle->dcb->num_entries);
		char ** elenco2 = malloc(dir_handle->dcb->num_entries * 255);
		SimpleFS_readDir(elenco2, dir_handle);
		for(i = 0; i < dir_handle->dcb->num_entries; i++) {
			printf("\n    > %s", elenco2[i]);
		}
		
		// Test SimpleFS_openFile
		printf("\n\n+++ Test SimpleFS_openFile()");
		FileHandle * fh = SimpleFS_openFile(dir_handle,"prova.txt");
		if(fh != NULL){
			printf("\n    Abbiamo aperto il file: %s ",fh->fcb->fcb.name);
		}else{
			printf("\n    Il file %s non esiste","prova.txt");		
		}
		fh = SimpleFS_openFile(dir_handle,"prova_0.txt");
		if(fh != NULL){
			printf("\n    Abbiamo aperto il file: %s ",fh->fcb->fcb.name);
		}else{
			printf("\n    Il file %s non esiste","prova.txt");		
		}

		// Test per vedere se il file è già presente
		if(SimpleFS_createFile(dir_handle,"prova_0.txt") != NULL)
			printf("\n    File %s creato correttamente", "prova_0.txt");
		else
			printf("\n    Il file %s già esiste", "prova_0.txt");

		// Test per vedere se la directory è gia presente
		printf("\n    Il risultato della SimpleFS_mkDir(dh, \"pluto\") è: %d", SimpleFS_mkDir(dir_handle, "pluto"));

		printf("\n\n+++ Test SimpleFS_write()");
		printf("\n\n+++ Test SimpleFS_read()");
	
		// Test SimpleFS_close
		printf("\n\n+++ Test SimpleFS_close()");
		printf("\n    La SimpleFS_close(fh) ha restituito %d", SimpleFS_close(fh));
		
		printf("\n\n+++ Test SimpleFS_seek()");
		printf("\n\n+++ Test SimpleFS_remove()");

	}
	printf("\n\n");
}
