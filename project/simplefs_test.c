#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"
#include <stdio.h>
#include <string.h>

int main(int agc, char** argv) {
	// printf("char size %ld\n", sizeof(char));
	// printf("int size %ld\n", sizeof(int));
	// printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
	// printf("DataBlock size %ld\n", sizeof(FileBlock));
	// printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
	// printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));

	// Test BitMap_create
	BitMap bitmap;
	BitMap_init(&bitmap, 40, "pippo");
	printf("\nBitMap contiene: %d e %s", bitmap.num_bits, bitmap.entries);

	// Test BitMap_blockToIndex
	int num = 4 * BLOCK_SIZE;
	BitMapEntryKey block = BitMap_blockToIndex(num);
	printf("\n\nLa posizione del blocco è %d, ovvero entry %d con sfasamento %d", num, block.entry_num, block.bit_num);

	// Test BitMap_indexToBlock
	int posizione = BitMap_indexToBlock(block);
	printf("\n\nAbbiamo la entry %d e lo sfasamento %d, ovvero la posizione %d", block.entry_num, block.bit_num, posizione);

	// Test BitMap_set
	printf("\n\nIl vecchio valore di \"entries\" in \"bitmap\" è %s", bitmap.entries);
	int bitmap_set = BitMap_set(&bitmap, 2, 1);
	//printf("\nBitMap_set() ha restituito %d", bitmap_set);
	printf("\nIl nuovo valore di \"entries\" in \"bitmap\" è %s", bitmap.entries);

	// Test BitMap_get
	printf("\n\nLa BitMap contiene %s\n", bitmap.entries);
	int start = 1;
	int status = 0;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(&bitmap, start, status));
	start = 3;
	status = 1;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(&bitmap, start, status));
	start = 4;
	status = 0;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(&bitmap, start, status));
	start = 0;
	status = 1;
	printf("Verifichiamo con start=%d e status=%d ... %d\n", start, status, BitMap_get(&bitmap, start, status));

	// DiskDriver disk;
	// char* filename = "test.txt";
	// int num_blocks = 100;
	// DiskDriver_init(&disk, filename, num_blocks);

	printf("\n\n");
}

// Funzione che restituisce una BitMap contenente i valori passati come argomenti
void BitMap_init(BitMap* bitmap, int num_bits, uint8_t* entries){
	// Assegno il valore preso come argomento a num_bits
	bitmap->num_bits = num_bits;
	// Assegno il valore preso come argomento a entries
	bitmap->entries = (entries);
}

// Prendiamo in ingresso il parametro "num" che rappresenta la posizione di un blocco nella memoria, lo convertiamo in due valori che rappresentano rispettivamente l'indice dell'entry e lo spiazzamento all'interno di essa
// Converts a linear index to an index in the array, and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num) {
	BitMapEntryKey blocco;
	// Calcolo l'indice della entry
	blocco.entry_num = num / 8 ;
	// Calcolo lo spiazzamento della posizione reale all'interno della entry
	blocco.bit_num = num % 8;
	return blocco;
}

// Questa funzione converte l'indice dell'entry e lo spiazzamento nell'entry, in un intero che rappresenta la posizione del blocco nella memoria
// Converts a bit to a linear index
int BitMap_indexToBlock(BitMapEntryKey entry) {
	return (entry.entry_num*8) + entry.bit_num;
}

// Imposta il bit all'indice "pos" in bmap a "status"
// Sets the bit at index pos in bmap to status
 int BitMap_set(BitMap* bmap, int pos, int status) {
	BitMapEntryKey bmek = BitMap_blockToIndex(pos);
	uint8_t mask = ((uint8_t) 128) >> ( (uint8_t) bmek.bit_num );
 	if(status) {
 		bmap->entries[bmek.entry_num] |= mask;
 	}else{
		bmap->entries[bmek.entry_num] &= (~ mask );
 	}
 	return status;
 }

// Restituisce l'indice del primo bit avente status "status" nella bitmap bmap, iniziando a cercare dalla posizione "start"
// Returns the index of the first bit having status "status" in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status) {
	// Se si inizia a cercare da una posizione che esce dall'entry, si restituisce -1
	if(start > bmap->num_bits) return -1;
	// Definiamo le variabili che si useranno all'interno
	int posizione, i, result;
	uint8_t c, m;
	// Per ogni bit a partire da "start", verifichiamo
	for(i = start; i <= bmap->num_bits; i++) {
		// Se sforiamo le entries, restituisce -1 perché "status" non è stato trovato
		if(i == bmap->num_bits) return -1;
		BitMapEntryKey bmek = BitMap_blockToIndex(start);
		result = (bmap->entries[bmek.entry_num] & (128 >> bmek.bit_num));
		// Se dobbiamo verificare "status=1", il risultato deve essere ">0", altrimenti deve essere "=0"
		if(status == 1) {
			if(result > 0) return i;
		}else{
			if(result == 0) return i;
		}
	}
}

// Apre il file (creandolo, se necessario), allocando lo spazio necessario sul disco e calcolando quanto deve essere grane la mappa se il file è stato appena creato.
// Compila un Disk Header e riempie la Bitmap della dimensione appropriata con tutti 0 (per denotare lo spazio libero)
// opens the file (creating it if necessary) allocates the necessary space on the disk calculates how big the bitmap should be
// If the file was new compiles a disk header, and fills in the bitmap of appropriate size with all 0 (to denote the free space)
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks) {
	// Apriamo il file ricevuto come parametro
	FILE* file = fopen(filename, "rw+");
	// Se il file non esiste o non viene aperto, blocchiamo la funzione
	if(!file) return;

	// Creiamo un DiskHeader che andrà inserito nel DiskDriver
	DiskHeader header;
	// Impostiamo il numero dei blocchi all'interno del DiskHeader
	header.num_blocks = num_blocks;
	// Se il numero dei blocchi è multiplo di 8, impostiamo num_blocks/8, altrimenti aggiungiamo 1 (per arrotondare per eccesso)
	if(num_blocks % 8 == 0) {
		header.bitmap_blocks = num_blocks / 8;
	}else{
		header.bitmap_blocks = (num_blocks / 8) + 1;
	}
	//
	header.bitmap_entries = ???;
	header.free_blocks = ???;
	header.first_free_block = ???;
	disk->header = header;
	disk->bitmap_data = ???;


}

/*

// reads the block in position block_num
// returns -1 if the block is free accrding to the bitmap
// 0 otherwise
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num) {
}

// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num) {
}

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num) {
}

// returns the first free blockin the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start) {
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk) {
}

// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk) {

}

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs) {
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename) {
}

// reads in the (preallocated) blocks array, the name of all files in a directory
int SimpleFS_readDir(char** names, DirectoryHandle* d) {
}

// opens a file in the  directory d. The file should be exisiting
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename) {
}

// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f) {
}

// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size) {
}

// writes in the file, at current position size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size) {
}

// returns the number of bytes read (moving the current pointer to pos)
// returns pos on success
// -1 on error (file too short)
int SimpleFS_seek(FileHandle* f, int pos) {
}

// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
 int SimpleFS_changeDir(DirectoryHandle* d, char* dirname) {
}

// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname) {
}

// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(SimpleFS* fs, char* filename) {
}
*/
