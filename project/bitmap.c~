#include "bitmap.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

// Funzione che restituisce una BitMap contenente i valori passati come argomenti
void BitMap_init(BitMap* bitmap, int num_bits, uint8_t* entries){
	// Assegno il valore preso come argomento a num_bits
	bitmap->num_bits = num_bits;
	bitmap->entries = malloc(num_bits/8+1);
	int i;
	for(i = 0; i < num_bits; i++) {
		BitMap_set(bitmap, i, 0);
	}
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
 int BitMap_set(BitMap* bitmap, int pos, int status) {
    BitMapEntryKey bmek = BitMap_blockToIndex(pos);
		uint8_t mask = 1 << bmek.bit_num;
		if(status){
			bitmap->entries[bmek.entry_num] |= mask;
		}else{
    	bitmap->entries[bmek.entry_num] &= ~(mask);
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
		BitMapEntryKey bmek = BitMap_blockToIndex(i);
		result = (bmap->entries[bmek.entry_num] & (1 << bmek.bit_num)); //TODO modificarlo
		// Se dobbiamo verificare "status=1", il risultato deve essere ">0", altrimenti deve essere "=0"
		if(status == 1) {
			if(result > 0) return i;
		}else{
			if(result == 0) return i;
		}
	}
}
