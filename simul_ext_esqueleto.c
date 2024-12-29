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
void Directorio(EXT_ENTRADA_DIR (*directorio)[MAX_FICHEROS], EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
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
	 char comando[LONGITUD_COMANDO];
	 char orden[LONGITUD_COMANDO];
	 char argumento1[LONGITUD_COMANDO];
	 char argumento2[LONGITUD_COMANDO];
	 
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
         if (strcmp(orden,"bytemaps")==0) {
            Printbytemaps(&ext_bytemaps);
            continue;
         }
         if (strcmp(orden, "rename")==0) {
            if (argumento1[0] == '\0' || argumento2[0] == '\0') {
                printf("ERROR: Sintaxis incorrecta. Uso: rename <nombre_antiguo> <nombre_nuevo>\n");
                continue;
            }
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
            // Guardar cambios
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            continue;
        }
        if (strcmp(orden, "imprimir")==0) {
            if (argumento1[0] == '\0') {
                printf("ERROR: Sintaxis incorrecta. Uso: imprimir <nombre_fichero>\n");
                continue;
            }
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
            continue;
        }
        if (strcmp(orden, "remove")==0) {
            if (argumento1[0] == '\0') {
                printf("ERROR: Sintaxis incorrecta. Uso: remove <nombre_fichero>\n");
                continue;
            }
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == 0) {
                // Guardar cambios
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
            continue;
        }
        if (strcmp(orden, "copy")==0) {
            if (argumento1[0] == '\0' || argumento2[0] == '\0') {
                printf("ERROR: Sintaxis incorrecta. Uso: copy <nombre_origen> <nombre_destino>\n");
                continue;
            }
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == 0) {
                // Guardar cambios
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
                grabardatos = 1;  // Indicar que hay que guardar los datos
            }
            continue;
        }
        // ...
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(memdatos,fent);
            fclose(fent);
            return 0;
         }
        printf("%s: comando no encontrado\n", orden);
        continue;
     }
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
    for(int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
    
    printf("Bloques [0-25]: ");
    for(int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

void Directorio(EXT_ENTRADA_DIR (*directorio)[MAX_FICHEROS], EXT_BLQ_INODOS *inodos) {
    for(int i = 1; i < MAX_FICHEROS; i++) {
        if((*directorio)[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[(*directorio)[i].dir_inodo];
            
            printf("%s\ttamano:%d\tinodo:%d bloques:", 
                   (*directorio)[i].dir_nfich,
                   inodo->size_fichero,
                   (*directorio)[i].dir_inodo);
            
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
            strcpy(directorio[i].dir_nfich, nombrenuevo);
            return 0;
        }
    }
    
    printf("ERROR: Fichero %s no encontrado\n", nombreantiguo);
    return -1;
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

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int i;
    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO && 
           strcmp(directorio[i].dir_nfich, nombre) == 0) {
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
            
            for(int j = 0; j < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[j] != NULL_BLOQUE; j++) {
                int indice_bloque = inodo->i_nbloque[j] - PRIM_BLOQUE_DATOS;
                if(indice_bloque >= 0 && indice_bloque < MAX_BLOQUES_DATOS) {
                    int bytes_restantes = inodo->size_fichero - (j * SIZE_BLOQUE);
                    int bytes_a_imprimir = (bytes_restantes < SIZE_BLOQUE) ? bytes_restantes : SIZE_BLOQUE;
                    
                    fwrite(memdatos[indice_bloque].dato, 1, bytes_a_imprimir, stdout);
                }
            }
            printf("\n");
            return 0;
        }
    }
    
    printf("ERROR: Fichero %s no encontrado\n", nombre);
    return -1;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    int i;
    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO && 
           strcmp(directorio[i].dir_nfich, nombre) == 0) {
            
            int num_inodo = directorio[i].dir_inodo;
            EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[num_inodo];
            
            for(int j = 0; j < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[j] != NULL_BLOQUE; j++) {
                ext_bytemaps->bmap_bloques[inodo->i_nbloque[j]] = 0;
                ext_superblock->s_free_blocks_count++;
            }
            
            ext_bytemaps->bmap_inodos[num_inodo] = 0;
            ext_superblock->s_free_inodes_count++;
            
            inodo->size_fichero = 0;
            for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                inodo->i_nbloque[j] = NULL_BLOQUE;
            }
            
            memset(directorio[i].dir_nfich, 0, LEN_NFICH);
            directorio[i].dir_inodo = NULL_INODO;
            
            return 0;
        }
    }
    
    printf("ERROR: Fichero %s no encontrado\n", nombre);
    return -1;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    int i, encontrado = 0;
    int inodo_origen = -1;
    EXT_SIMPLE_INODE *inodo_orig = NULL;

    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo != NULL_INODO) {
            if(strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
                inodo_origen = directorio[i].dir_inodo;
                inodo_orig = &inodos->blq_inodos[inodo_origen];
                encontrado = 1;
            } else if(strcmp(directorio[i].dir_nfich, nombredestino) == 0) {
                printf("ERROR: El fichero %s ya existe\n", nombredestino);
                return -1;
            }
        }
    }

    if(!encontrado) {
        printf("ERROR: Fichero origen %s no encontrado\n", nombreorigen);
        return -1;
    }

    int nuevo_inodo = -1;
    for(i = 0; i < MAX_INODOS; i++) {
        if(ext_bytemaps->bmap_inodos[i] == 0) {
            nuevo_inodo = i;
            break;
        }
    }

    if(nuevo_inodo == -1) {
        printf("ERROR: No hay inodos libres\n");
        return -1;
    }

    int entrada_libre = -1;
    for(i = 0; i < MAX_FICHEROS; i++) {
        if(directorio[i].dir_inodo == NULL_INODO) {
            entrada_libre = i;
            break;
        }
    }

    if(entrada_libre == -1) {
        printf("ERROR: No hay entradas libres en el directorio\n");
        return -1;
    }

    EXT_SIMPLE_INODE *nuevo_inodo_struct = &inodos->blq_inodos[nuevo_inodo];
    nuevo_inodo_struct->size_fichero = inodo_orig->size_fichero;

    for(i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo_orig->i_nbloque[i] != NULL_BLOQUE; i++) {
        int nuevo_bloque = -1;
        for(int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
            if(ext_bytemaps->bmap_bloques[j] == 0) {
                nuevo_bloque = j;
                break;
            }
        }

        if(nuevo_bloque == -1) {
            printf("ERROR: No hay bloques libres\n");
            return -1;
        }

        int indice_origen = inodo_orig->i_nbloque[i] - PRIM_BLOQUE_DATOS;
        int indice_destino = nuevo_bloque - PRIM_BLOQUE_DATOS;
        memcpy(memdatos[indice_destino].dato, memdatos[indice_origen].dato, SIZE_BLOQUE);

        nuevo_inodo_struct->i_nbloque[i] = nuevo_bloque;
        ext_bytemaps->bmap_bloques[nuevo_bloque] = 1;
        ext_superblock->s_free_blocks_count--;
    }
    for(; i < MAX_NUMS_BLOQUE_INODO; i++) {
        nuevo_inodo_struct->i_nbloque[i] = NULL_BLOQUE;
    }
    ext_bytemaps->bmap_inodos[nuevo_inodo] = 1;
    ext_superblock->s_free_inodes_count--;
    strcpy(directorio[entrada_libre].dir_nfich, nombredestino);
    directorio[entrada_libre].dir_inodo = nuevo_inodo;

    return 0;
}