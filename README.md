# Simulador de Sistema de Archivos EXT Simplificado

Este programa simula un sistema de archivos EXT simplificado, utilizando un archivo binario llamado `particion.bin` como partición virtual. Permite realizar operaciones básicas como listar archivos, renombrarlos, copiarlos, eliminarlos y mostrar contenido, todo a través de comandos en un entorno interactivo.

## Autores:
- Daniel Dolz
- Nicolas Sesmero

## Requisitos del Sistema:

- Compilador C, recomendamos GCC

## Instrucciones de Compilación:
gcc simul_ext_esqueleto.c -o simul_ext_esqueleto

## Modo de Ejecución:
./simul_ext_esqueleto

## Comandos Disponibles:
- info: Muestra información del superbloque.
- bytemaps: Muestra el estado de los bytemaps de bloques e inodos.
- dir: Lista los archivos en la partición con sus detalles.
- rename <nombre_antiguo> <nombre_nuevo>: Cambia el nombre de un archivo.
- imprimir <nombre_archivo>: Muestra el contenido de un archivo.
- remove <nombre_archivo>: Elimina un archivo.
- copy <nombre_origen> <nombre_destino>: Copia un archivo.
- salir: Guarda los cambios y finaliza el programa.

## Video Explicativo:
Link al video: [\[URL del video explicativo\]](https://www.youtube.com/watch?v=2lvXVvYShiM)