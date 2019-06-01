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

	// Mi creo un DirectoryHandle e ci collego il FileSystem attuale
	DirectoryHandle * directory_handle = malloc(sizeof(DirectoryHandle));
	directory_handle->sfs = fs;

	// Recupero la FirstDirectoryBlock memorizzata su disco e memorizzo le sue informazioni
	FirstDirectoryBlock * first_directory_block = malloc(sizeof(FirstDirectoryBlock));
	DiskDriver_readBlock(fs->disk, first_directory_block, 0);
	directory_handle->dcb = first_directory_block;
	directory_handle->directory = NULL;
  directory_handle->current_block = &(directory_handle->dcb->header);
  directory_handle->pos_in_dir = 0;
  directory_handle->pos_in_block = 0;

	// Restituisco il DirectoryHandle popolato
	return directory_handle;
}

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs) {

	// Azzero la BitMap di tutto il disco
	int i;
	BitMap bitmap;
	bitmap.num_bits = fs->disk->header->bitmap_blocks;
	bitmap.entries = fs->disk->bitmap_data;
	for(i = 0; i < fs->disk->header->num_blocks * BLOCK_SIZE; i++) {
		BitMap_set(&bitmap, i, 0);
	}

	// Creo il primo blocco della cartella "base"
	FirstDirectoryBlock * first_directory_block = malloc(sizeof(FirstDirectoryBlock));

	// Inserisco le informazioni relative all'header
	first_directory_block->header.previous_block = -1;
	first_directory_block->header.next_block = -1;
	first_directory_block->header.block_in_file = 0; 

	// Inserisco le informazioni relative al FileControlBlock
	first_directory_block->fcb.directory_block = -1;
	first_directory_block->fcb.block_in_disk = 0; //TODO
  strcpy(first_directory_block->fcb.name,"/"); //copia la stringa nell'array
  first_directory_block->fcb.size_in_bytes = (BLOCK_SIZE / 8) + sizeof(FirstDirectoryBlock) + sizeof(BlockHeader) + sizeof(FileControlBlock); //TODO:  
  first_directory_block->fcb.size_in_blocks = first_directory_block->fcb.size_in_bytes % BLOCK_SIZE == 0 ? first_directory_block->fcb.size_in_bytes / BLOCK_SIZE : first_directory_block->fcb.size_in_bytes / BLOCK_SIZE + 1;;
  first_directory_block->fcb.is_dir = 1;

	first_directory_block.num_entries = 0;

	// Memorizziamo la FirstDirectoryBlock nel disco
	DiskDriver_writeBlock(fs->disk, first_directory_block, 0);
	DiskDriver_flush(fs->disk);	

	return;
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename) {
	
	if(d->sfs->disk->header->free_blocks == 0){
		return NULL; 
	}
	
	//if()//TODO: caso in cui il file già esiste()

	// Mi creo il FileHandle e ci inserisco le informazioni relative
	FileHandle * file_handle = malloc(sizeof(FileHandle));
	file_handle->sfs = d->sfs;

	FirstFileBlock * first_file_block = malloc(sizeof(FirstFileBlock)); 
	first_file_block->header.previous_block = -1 ; //TODO	
	first_file_block->header.next_block = -1 ; //TODO
	first_file_block->header.block_in_file = 0;
	
	first_file_block->fcb.directory_block = d->dcb->fcb.block_in_disk; 
	first_file_block->fcb.block_in_disk = DiskDriver_getFreeBlock(d->sfs->disk, first_file_block->fcb.directory_block);
	strcpy(first_file_block->fcb.name,filename);
	first_file_block->fcb.size_in_bytes = sizeof(FirstFileBlock) + sizeof(BlockHeader); 
	first_file_block->fcb.size_in_blocks = first_file_block->fcb.size_in_bytes % BLOCK_SIZE == 0 ? first_file_block->fcb.size_in_bytes / BLOCK_SIZE : first_file_block->fcb.size_in_bytes / BLOCK_SIZE + 1;
  first_file_block->fcb.is_dir = 0;
	
	strcpy(first_file_block->data,"");

	file_handle->fcb = first_file_block;
	file_handle->directory = d->dcb;
	file_handle->current_block = &first_file_block->header; //TODO
	file_handle->pos_in_file = first_file_block->fcb.block_in_disk;//TODO

	//TODO: fare la write e la flush 
	
	
	// Aggiungiamo in file_blocks il blocco in cui è contenuto questo file
	DirectoryBlock * db = malloc(sizeof(FirstDirectoryBlock)); //TODO: db ha dimensioni diverse
	if(d->dcb.header.next_block!=-1){// ha un next_block
		int successivo;
		db = d->dcb;
		while(db.header.next_block != -1){
				successivo = db.header.next_block;
				DiskDriver_read(d->sfs->disk, db, successivo);
		}
	}else{
		db=d->dcb; // Dentro db abbiamo l'ultimo blocco esistente della cartella
	}

	// Controlliamo se c'è spazio nel file_blocks
	// se c'è spazio
		// ce l'aggiungiamo in coda (il blocco di questo file)
	// se non c'è spazio
		// creo un nuovo directoryblock
		// aggiorno l'header del firstdirectoryblock
	
	int i = 0;
	if(db.num_entries < sizeof(db.file_blocks)){
		d->dcb->file_blocks[db.num_entries] = file_handle->pos_in_file; //TODO
		d.num_entries++;
	}else{
		 DirectoryBlock * directory_block = malloc(sizeof(DirectoryBlock));
		 directory_block->header.next_block = -1;
		 directory_block->header.previous_block = db->header.block_in_file; 
		 directory_block->header.block_in_file = ??;
		 //directory d'origine num_entries++
	}
	
	return file_handle;

}

// reads in the (preallocated) blocks array, the name of all files in a directory
int SimpleFS_readDir(char** names, DirectoryHandle* d) {
	/*
	int i;
	for(i=0; i < d->dcb.header.next_block ; i++){
		for(j=0; j < sizeof(d->dcb->file_blocks); j++){
			names[i] = "mario"; //TODO: nome file
		}
	}
	d->dcb->file_blocks	
	*/
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






























































































