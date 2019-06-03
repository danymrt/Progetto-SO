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
	DirectoryHandle * directory_handle = malloc(sizeof(DirectoryHandle));	
	directory_handle->sfs = fs;

	// Inserirò la radice sempre al primo posto della bitmap, nel caso già esiste la leggo solamente		
	if(fs->disk->header->first_free_block != 0){
		FirstDirectoryBlock * first_directory_block = malloc(sizeof(FirstDirectoryBlock));
		DiskDriver_readBlock(disk, first_directory_block, 0);
		directory_handle->dcb = first_directory_block;
		directory_handle->directory = NULL;
		directory_handle->current_block = &(directory_handle->dcb->header);
		directory_handle->pos_in_dir = 0;
		directory_handle->pos_in_block = first_directory_block->fcb.block_in_disk;
	}else{ // Nel caso in cui il filesystem non esiste, formatto il FileSystem
	//
		// Mi formatto il FileSystem ricevuto come parametro
		SimpleFS_format(fs);

		// Mi creo un DirectoryHandle e ci collego il FileSystem attuale
		directory_handle->sfs = fs;
	
		// Recupero la FirstDirectoryBlock memorizzata su disco e memorizzo le sue informazioni
		FirstDirectoryBlock * first_directory_block = malloc(sizeof(FirstDirectoryBlock));
		DiskDriver_readBlock(disk, first_directory_block, 0);
		directory_handle->dcb = first_directory_block;
		directory_handle->directory = NULL;
		directory_handle->current_block = &(directory_handle->dcb->header);
		directory_handle->pos_in_dir = 0;
		directory_handle->pos_in_block = first_directory_block->fcb.block_in_disk;
	}

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
	bitmap.num_bits = fs->disk->header->num_blocks;
	bitmap.entries = fs->disk->bitmap_data;
	for(i = 0; i < bitmap.num_bits; i++) {
		BitMap_set(&bitmap, i, 0);
	}
	fs->disk->bitmap_data = bitmap.entries;
	
	// Creo il primo blocco della cartella "base"
	FirstDirectoryBlock * first_directory_block = malloc(sizeof(FirstDirectoryBlock));

	// Inserisco le informazioni relative all'header
	first_directory_block->header.previous_block = -1;
	first_directory_block->header.next_block = -1;
	first_directory_block->header.block_in_file = 0; 

	// Inserisco le informazioni relative al FileControlBlock
	first_directory_block->fcb.directory_block = -1;
	first_directory_block->fcb.block_in_disk = fs->disk->header->first_free_block;
  strcpy(first_directory_block->fcb.name,"/");
  first_directory_block->fcb.size_in_bytes = sizeof(FirstDirectoryBlock);
  first_directory_block->fcb.size_in_blocks = count_blocks(first_directory_block->fcb.size_in_bytes);
  first_directory_block->fcb.is_dir = 1;
	first_directory_block->num_entries = 0;

	// Memorizziamo la FirstDirectoryBlock nel disco
	DiskDriver_writeBlock(fs->disk, first_directory_block, fs->disk->header->first_free_block);
	DiskDriver_flush(fs->disk);	

	return;
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename) {

	// TODO: Se esiste già un file con lo stesso nome, dentro la stessa cartella, restituisco -1

	// Se non ci sono blocchi liberi per creare il file, restituisco errore
	if(d->sfs->disk->header->free_blocks < 1){
		return NULL; 
	}
	
	if(!SimpleFS_openFile(d, filename)) {
		return NULL;
	}

	// Creo il FileHandle e inserisco le informazioni relative
	FileHandle * file_handle = malloc(sizeof(FileHandle));
	file_handle->sfs = d->sfs;
	FirstFileBlock * first_file_block = malloc(sizeof(FirstFileBlock)); 
	first_file_block->header.previous_block = -1;
	first_file_block->header.next_block = -1;
	first_file_block->header.block_in_file = 0;
	first_file_block->fcb.directory_block = d->dcb->fcb.block_in_disk;
	first_file_block->fcb.block_in_disk = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	strcpy(first_file_block->fcb.name, filename);
	first_file_block->fcb.size_in_bytes = sizeof(FirstFileBlock); 
	first_file_block->fcb.size_in_blocks = count_blocks(sizeof(FirstFileBlock));
  first_file_block->fcb.is_dir = 0;
	strcpy(first_file_block->data,"\0");
	file_handle->fcb = first_file_block;
	file_handle->directory = d->dcb;
	file_handle->current_block = &(first_file_block->header);
	file_handle->pos_in_file = 0;

	// Se c'è spazio nel blocco corrente della cartella attuale
	//    Aggiungo il primo blocco della nuova cartella
	// Altrimenti
	//    Se c'è un blocco successivo
	//       Accedo all'ultimo blocco disponibile della cartella attuale
	//    Altrimenti
  //       Se non c'è abbastanza spazio
	//          Mi creo un nuovo blocco della cartella attuale
  //          Aggiorno il blocco precedente in modo che punti a quello appena creato
	//    Assegno, all'interno dell'ultimo "file_blocks" della cartella corrente,
  //    l'indice del blocco in cui è memorizzata la cartella

	DiskDriver_writeBlock(d->sfs->disk, first_file_block, first_file_block->fcb.block_in_disk);
	if(space_in_dir(d->dcb->file_blocks, sizeof(d->dcb->file_blocks))) {
		d->dcb->file_blocks[d->dcb->num_entries] = first_file_block->fcb.block_in_disk;
		d->dcb->num_entries++;
		DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->dcb->fcb.block_in_disk);
	}else{
		int db_block = d->dcb->fcb.block_in_disk;
		DirectoryBlock * db;
		int new_db_block;	
		if(d->dcb->header.next_block != -1){
			db_block = d->dcb->header.next_block;
			db = malloc(sizeof(DirectoryBlock));
			DiskDriver_readBlock(d->sfs->disk, db, d->dcb->header.next_block);
			while(db->header.next_block != -1){
				db_block = db->header.next_block;
				DiskDriver_readBlock(d->sfs->disk, db, db_block);
			}
		}else if(!space_in_dir(db->file_blocks, sizeof(d->dcb->file_blocks))){ // In questo punto, dentro db abbiamo l'ultimo blocco esistente della cartella
			new_db_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
			DirectoryBlock * directory_block = malloc(sizeof(DirectoryBlock));
			directory_block->header.next_block = -1;
			directory_block->header.previous_block = db_block; 
			directory_block->header.block_in_file = db->header.block_in_file + 1;

			// Aggiorno il next_block del vecchio DirectoryBlock e lo sovrascrivo/aggiorno sul suo blocco
			db->header.next_block = new_db_block;
			DiskDriver_writeBlock(d->sfs->disk, db, db_block);

			// Adatto i puntatori e i valori in modo che possano funzionare all'esterno
			db = directory_block;
			db_block = new_db_block;
		}
		db->file_blocks[d->dcb->num_entries] = file_handle->fcb->fcb.block_in_disk;
		d->dcb->num_entries++;

		// Scrivo, su un nuovo blocco (libero) la DirectoryBlock appena creata
		DiskDriver_writeBlock(d->sfs->disk, db, db_block);
	}
	DiskDriver_flush(d->sfs->disk);
	return file_handle;
}

// reads in the (preallocated) blocks array, the name of all files in a directory
int SimpleFS_readDir(char** names, DirectoryHandle* d) {
	FirstDirectoryBlock * db = malloc(sizeof(FirstDirectoryBlock));
	DiskDriver_readBlock(d->sfs->disk, db, d->dcb->fcb.block_in_disk);

	// Per ogni file
	//    Se è presente nel blocco corrente
	//       Memorizzo il nome nell'array
	//    Se non è presente nel blocco corrente
	//       Entro nel blocco successivo
	//       Memorizzo il nome nell'array
	int i, j;
	for(i = 0; i < d->dcb->num_entries; i++) {
		j = i;
		if(i > sizeof(db->file_blocks)) {
			DirectoryBlock * db;
			j = j - sizeof(db->file_blocks);
			DiskDriver_readBlock(d->sfs->disk, db, db->header.next_block);
		}
		FirstFileBlock * first_file_block = malloc(sizeof(FirstFileBlock));
		DiskDriver_readBlock(d->sfs->disk, first_file_block, db->file_blocks[j]);
		names[j] = first_file_block->fcb.name;
	}

	return 0;
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

	// TODO: Se non c'è spazio per creare la cartella, restituisco -1
	// TODO: Se esiste già una cartella con lo stesso nome, restituisco -1

	FirstDirectoryBlock * fdb = malloc(sizeof(FirstDirectoryBlock));
	fdb->header.previous_block = -1;
	fdb->header.next_block = -1;
	fdb->header.block_in_file = 0;
	fdb->fcb.directory_block = d->dcb->fcb.block_in_disk;
	fdb->fcb.block_in_disk = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	strcpy(fdb->fcb.name, dirname);
	fdb->fcb.size_in_bytes = 0;
	fdb->fcb.size_in_blocks = 0;
	fdb->fcb.is_dir = 1;
	fdb->num_entries = 0;
	DiskDriver_writeBlock(d->sfs->disk, fdb, fdb->fcb.block_in_disk);

	// Se c'è spazio nel blocco corrente della cartella attuale
	//    Aggiungo il primo blocco della nuova cartella
	// Altrimenti
	//    Se c'è un blocco successivo
	//       Accedo all'ultimo blocco disponibile della cartella attuale
	//    Altrimenti
  //       Se non c'è abbastanza spazio
	//          Mi creo un nuovo blocco della cartella attuale
  //          Aggiorno il blocco precedente in modo che punti a quello appena creato
	//    Assegno, all'interno dell'ultimo "file_blocks" della cartella corrente,
  //    l'indice del blocco in cui è memorizzata la cartella

	if(space_in_dir(d->dcb->file_blocks, sizeof(d->dcb->file_blocks))) {
		d->dcb->file_blocks[d->dcb->num_entries] = fdb->fcb.block_in_disk;
		d->dcb->num_entries++;
		DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->dcb->fcb.block_in_disk);
	}else{
		int db_block = d->dcb->fcb.block_in_disk;
		DirectoryBlock * db;
		int new_db_block;	
		if(d->dcb->header.next_block != -1){
			db_block = d->dcb->header.next_block;
			db = malloc(sizeof(DirectoryBlock));
			DiskDriver_readBlock(d->sfs->disk, db, d->dcb->header.next_block);
			while(db->header.next_block != -1){
				db_block = db->header.next_block;
				DiskDriver_readBlock(d->sfs->disk, db, db_block);
			}
		}else if(!space_in_dir(db->file_blocks, sizeof(d->dcb->file_blocks))){ // In questo punto, dentro db abbiamo l'ultimo blocco esistente della cartella
			new_db_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
			DirectoryBlock * directory_block = malloc(sizeof(DirectoryBlock));
			directory_block->header.next_block = -1;
			directory_block->header.previous_block = db_block; 
			directory_block->header.block_in_file = db->header.block_in_file + 1;

			// Aggiorno il next_block del vecchio DirectoryBlock e lo sovrascrivo/aggiorno sul suo blocco
			db->header.next_block = new_db_block;
			DiskDriver_writeBlock(d->sfs->disk, db, db_block);

			// Adatto i puntatori e i valori in modo che possano funzionare all'esterno
			db = directory_block;
			db_block = new_db_block;
		}
		db->file_blocks[d->dcb->num_entries] = fdb->fcb.block_in_disk;
		d->dcb->num_entries++;

		// Scrivo, su un nuovo blocco (libero) la DirectoryBlock appena creata
		DiskDriver_writeBlock(d->sfs->disk, db, db_block);
	}
	DiskDriver_flush(d->sfs->disk);

	return 0;
}

// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(SimpleFS* fs, char* filename) {
}
