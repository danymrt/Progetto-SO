# Progetto-SO
Progetto per il corso di Sistemi Operativi, realizzato da Daniela Moretti e Francesco Schettino.

# Obiettivo del progetto  
## Implementare un file system  
Implementare l'interfaccia di un file system usando file binari.  
Il file system riserva la prima parte del file per memorizzare:  
* una lista collegata di blocchi liberi
* una lista collegata di blocchi di file
* una singola cartella globale
  
I blocchi saranno di due tipologie:  
* blocchi di dati, che contengono informazioni "casuali"
* blocchi di cartelle, che contengono strutture di tipo *directory_entry*, le quali definiscono se si tratta di directory o file (primi blocchi)
