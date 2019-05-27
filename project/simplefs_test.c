#include "simplefs.h"
#include "simplefs.c"
#include "disk_driver.h"
#include "disk_driver.c"
#include "bitmap.h"
#include "bitmap.c"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int agc, char** argv) {
	// printf("char size %ld\n", sizeof(char));
	// printf("int size %ld\n", sizeof(int));
	// printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
	// printf("DataBlock size %ld\n", sizeof(FileBlock));
	// printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
	// printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));

	// Test BitMap_create
	printf("\n+++ Test BitMap_create()");
	DiskDriver disk;
	char* filename = "test.txt";
	int num_blocks = 100;
	DiskDriver_init(&disk, filename, num_blocks);
	BitMap bitmap=*(disk.bitmap);

	// Test BitMap_blockToIndex
	printf("\n+++ Test BitMap_blockToIndex()");
	int num = 4 * BLOCK_SIZE;
	BitMapEntryKey block = BitMap_blockToIndex(num);
	printf("\n\nLa posizione del blocco è %d, ovvero entry %d con sfasamento %d", num, block.entry_num, block.bit_num);

	// Test BitMap_indexToBlock
	printf("\n+++ Test BitMap_indexToBlock()");
	int posizione = BitMap_indexToBlock(block);
	printf("\n\nAbbiamo la entry %d e lo sfasamento %d, ovvero la posizione %d", block.entry_num, block.bit_num, posizione);

	// Test BitMap_set
	printf("\n+++ Test BitMap_set()");
	BitMap* bimap = (BitMap*)malloc(sizeof(BitMap));
  bimap ->num_bits = 32;
  bimap ->entries = (char*)malloc(sizeof(char) * 4);
  bimap ->entries[0] = 'c';
  bimap ->entries[1] = 'i';
  bimap ->entries[2] = 'a';
  bimap ->entries[3] = 'o';
	char a;
  int res;
  int i;
	printf("\n");
	for(i=0; i<= (bimap ->num_bits)/8; i++){
		printf("%c",bimap ->entries[i]);
	}

  a = bimap ->entries[2];
	printf("\n%c\n",a);
  printf("%d come entry, setto a 1 il primo bit\n");
  for (i = 7; i >= 0; i--) {
      printf("%d", !!((a >> i) & 0x01));
  }
  printf("\n");

  res = BitMap_set(bimap, 17, 1);
  printf("res: %d\n", res);

  a = bimap ->entries[2];
	printf("%c\n",a);
  for (i = 7; i >= 0; i--) {
      printf("%d", !!((a >> i) & 0x01));
  }
	for(i=0; i<= (bimap ->num_bits)/8; i++){
		printf("%c",bimap ->entries[i]);
	}
  printf("\n\n");


	// Test BitMap_get
	printf("\n+++ Test BitMap_get()");
	for(i=0; i<= (bimap ->num_bits)/8; i++){
		printf("%c",bimap ->entries[i]);
	}
	printf("\n");
	int start = 6;
	int status = 0;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(bimap, start, status));
	start = 3;
	status = 1;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(bimap, start, status));
	start = 12;
	status = 0;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(bimap, start, status));
	start = 13;
	status = 1;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(bimap, start, status));


	// Test DiskDriver_writeBlock
	printf("\n+++ Test DiskDriver_writeBlock()");
	int block_num=0;
	void* src="hello geek";
	printf("Il risultato della write con block_num=%d è: %d\n", block_num, DiskDriver_writeBlock(&disk,src,block_num));


	// Test DiskDriver_readBlock
	printf("\n+++ Test DiskDriver_readBlock()");
	void* dest;
	printf("Il risultato della read con block_num=%d è: %d\n", block_num, DiskDriver_readBlock(&disk,dest,block_num));


	// Test DiskDriver_freeBlock
	printf("\n+++ Test DiskDriver_freeBlock()");
	//printf("\n Il risultato della FreeBlock è: %d \n",DiskDriver_freeBlock(&disk,0));


	// Test DiskDriver_getFreeBlock
	printf("\n+++ Test DiskDriver_getFreeBlock()");
	printf("\n Il risultato della getFreeBlock è: %d \n",DiskDriver_getFreeBlock(&disk,0));
}
