#include "simplefs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <stdlib.h>

// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk) {

	// Interpreto il disco passato in parametro come disco principale del FileSystem
	fs->disk = disk;

	// Mi formatto il FileSystem ricevuto come parametro
	SimpleFS_format(fs);

	// Mi credo un DirectoryHandle e ci collego il FileSystem attuale
	DirectoryHandle * dh = malloc(sizeof(DirectoryHandle));
	dh->sfs = fs;

	// Recupero la FirstDirectoryBlock memorizzata su disco e memorizzo le sue informazioni
	FirstDirectoryblock * fdb = malloc(sizeof(FirstDirectoryBlock));
	DiskDriver_readBlock(fs->disk, fdb, 0);
	dh->dcb = fdb;
	dh->directory = NULL;
  dh->current_block = dh->dcb->header;
  dh->pos_in_dir = 0;
  dh->pos_in_block = 0;

	// Restituisco il DirectoryHandle popolato
	return dh;
}

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs) {

	// Azzero la BitMap di tutto il disco
	BitMap bitmap;
	bitmap.num_bits = fs->disk->header->bitmap_blocks;
	bitmap.entries = fs->disk->bitmap_data;
	for(i = 0; i < fs->disk->header->num_blocks * BLOCK_SIZE; i++) {
		BitMap_set(&bitmap, i, 0);
	}

	// Creo il primo blocco della cartella "base"
	FirstDirectoryBlock * fdb = malloc(sizeof(FirstDirectoryBlock));

	// Inserisco le informazioni relative all'header
	fdb->header.previous_block = -1;
	fdb->header.next_block = 1;
	fdb->header.block_in_file = 0;

	// Inserisco le informazioni relative al FileControlBlock
	fdb->fcb.directory_block = -1;
  fdb->fcb.block_in_disk = fs->disk->header->num_blocks;   // repeated position of the block on the disk
  fdb->fcb.name = "/";
  fdb->fcb.size_in_bytes = (BLOCK_SIZE / 8) - sizeof(FirstDirectoryBlock) - sizeof(BlockHeader) - sizeof(FileControlBlock);
  fdb->fcb.size_in_blocks = 1;
  fdb->fcb.is_dir = 1;

	// Memorizziamo la FirstDirectoryBlock nel disco
	DiskDriver_writeBlock(fs->disk, fdb, 0);

	return;
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
