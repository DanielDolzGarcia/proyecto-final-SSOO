

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre)
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     //...
     
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
	     if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
            }
        if (strcmp(orden,"info")==0) {
          LeeSuperBloque(&ext_superblock);
          continue;
        }
        if (strcmp(orden, "rename")==0) {
            if (argumento1[0] == '\0' || argumento2[0] == '\0') {
                printf("ERROR: Sintaxis incorrecta. Uso: rename <nombre_antiguo> <nombre_nuevo>\n");
                continue;
            }
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
            Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
            continue;
        }
         //...
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }

     }
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    char *token;
    char string[LONGITUD_COMANDO];
    
    strcpy(string, strcomando);
    token = strtok(string, " \n");
    if (token == NULL) return 1;
    
    strcpy(orden, token);
    token = strtok(NULL, " \n");
    if (token != NULL) strcpy(argumento1, token);
    else argumento1[0] = '\0';
    
    token = strtok(NULL, " \n");
    if (token != NULL) strcpy(argumento2, token);
    else argumento2[0] = '\0';
    
    return 0;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    fseek(fich, 4 * SIZE_BLOQUE, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Bloque %d Bytes\n", psup->s_block_size);
    printf("Inodos particion = %d\n", psup->s_inodes_count);
    printf("Inodos libres = %d\n", psup->s_free_inodes_count);
    printf("Bloques particion = %d\n", psup->s_blocks_count);
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");
    // Muestra bytemap de inodos
    for(int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
    
    printf("Bloques [0-25]: ");
    // Muestra los primeros 25 bloques
    for(int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    // Empezamos en 1 para saltar la entrada "."
    for(int i = 1; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO) {
            // Obtener el inodo correspondiente
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
            
            // Imprimir nombre y tamaño
            printf("%s\ttamaño:%d\tinodo:%d bloques:", 
                   directorio[i].dir_nfich,
                   inodo->size_fichero,
                   directorio[i].dir_inodo);
            
            // Imprimir lista de bloques
            for(int j = 0; j < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[j] != NULL_BLOQUE; j++) {
                printf(" %d", inodo->i_nbloque[j]);
            }
            printf("\n");
        }
    }
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    int i;
    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO && 
           strcmp(directorio[i].dir_nfich, nombrenuevo) == 0) {
            printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
            return -1;
        }
    }
    
    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO && 
           strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) {
            // Esto cambia el nombre
            strcpy(directorio[i].dir_nfich, nombrenuevo);
            return 0;
        }
    }
    
    printf("ERROR: Fichero %s no encontrado\n", nombreantiguo);
    return -1;
}