#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"
#include <stdio.h>

int main(int agc, char** argv) {
	printf("char size %ld\n", sizeof(char));
	printf("int size %ld\n", sizeof(int));
	printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
	printf("DataBlock size %ld\n", sizeof(FileBlock));
	printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
	printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));

	// Test BitMap_create
	BitMap bitmap = BitMap_create(10,"ciao");
	printf("\nBitMap contiene: %d e %s \n", bitmap.num_bits, bitmap.entries);

	// Test BitMap_blockToIndex
	int num = 4 * BLOCK_SIZE;
	BitMapEntryKey block = BitMap_blockToIndex(num);
	printf("\nLa posizione del blocco è %d, ovvero entry %d con sfasamento %d\n", num, block.entry_num, block.bit_num);

	// Test BitMap_indexToBlock
	int posizione = BitMap_indexToBlock(block.entry_num, block.bit_num);
	printf("\nAbbiamo la entry %d e lo sfasamento %d, ovvero la posizione %d\n", block.entry_num, block.bit_num, posizione);
}

// Funzione che restituisce una BitMap contenente i valori passati come argomenti
BitMap BitMap_create(int num_bits, char* entries){
	// Definisco una BitMap
	BitMap bitmap;
	// Assegno il valore preso come argomento a num_bits
	bitmap.num_bits = num_bits;
	// Assegno il valore preso come argomento a entries
	bitmap.entries = entries;
	// Restituisco la BitMap
	return bitmap;
}

// Prendiamo in ingresso il parametro "num" che rappresenta la posizione di un blocco nella memoria, lo convertiamo in due valori che rappresentano rispettivamente l'indice dell'entry e lo spiazzamento all'interno di essa
// Converts a linear index to an index in the array, and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num) {
	// Dichiaro la BitMapEntryKey che conterrà le informazioni
	BitMapEntryKey blocco;
	// Calcolo l'indice della entry
	blocco.entry_num = num / ( sizeof(char) * 8 );
	// Calcolo lo spiazzamento della posizione reale all'interno della entry
	blocco.bit_num = num - ( blocco.entry_num * 8 );
	// Restituisco la struct che contiene tutte le informazioni
	return blocco;
}

// Questa funzione converte l'indice dell'entry e lo spiazzamento nell'entry, in un intero che rappresenta la posizione del blocco nella memoria
// Converts a bit to a linear index
int BitMap_indexToBlock(int entry, uint8_t bit_num) {
	// dichiaro la variabile da restituire
	int posizione;
	// ottengo la posizione di inizio di questa entry
	posizione = entry * ( sizeof(char) * 8 );
	// aggiungo lo spiazzamento per ottenere la posizione "precisa"
	posizione = posizione + bit_num;
	// restituisco la posizione precisa
	return posizione;
}

// Imposta il bit all'indice "pos" in bmap a "status"
// Sets the bit at index pos in bmap to status
int BitMap_set(BitMap* bmap, int pos, int status) {

}

/*
// Restituisce l'indice del primo bit avente status "status" nella bitmap bmap, iniziando a cercare dalla posizione "start"
// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status) {
	for(int i=start; i < bmap->num_bit ; i++){
		printf("entries: %c status: %d",bmap->entries,status);
	}
}

// The blocks indices seen by the read/write functions
// have to be calculated after the space occupied by the bitmap

// opens the file (creating it if necessary_
// allocates the necessary space on the disk
// calculates how big the bitmap should be
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks) {
}

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
