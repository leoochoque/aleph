#include "Auxiliar.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

void copia_cad(str A, str B){
	strcpy(A, B);
}

int tam_cad(str A){
	return strlen(A);
}

str devuelve_cad(str A){
	return strdup(A);
}

str concat_cad(str A, str B){
	return strcat(A,B);
}

int compara_cad(str A, str B){
	if(strcmp(A,B)>0)
		return 1;
	else if(strcmp(A,B)<0)
		return -1;
	return 0;
}

char * obtenerElemento(char *A){
	int i = 1; // ignora la primera llave o corchete
	int c = 0; // bandera para determinar si hay anidamientos (Si c>0 hay anidamientos)
	int k; // contador para copiar cadenas
	char s = ','; // definimos un caracter para corte de ciclo
	char * element = NULL; //inicializamos una variable para guardar una cadena
	if(A[0]=='{'||A[0]=='['||A[0]=='('){ //verificamos que la cadena ingresada sea un conj.
		element = (char *)malloc(sizeof(char)); // creamos una cadena para guardar el elemento
		while(A[i]!='\0'&&A[i]!=s){ //se ejecuta mientras no se llegue al final y no sea un caracter de salida.
			if(A[i]=='{'||A[i]=='['||A[i]=='('){ //verifica si hay anidamientos
				c++;  //cuenta los anidamientos
				s='\0';  //modificamos el caracter de salida para que ignore las ',' dentro de los anidamientos
			}else if(A[i]=='}'||A[i]==']'||A[i]==')'){
				c--;
				if(c == 0) s=','; //se termino los anidamientos vuelve a salir con ','
			}
			element[i-1]=A[i];  //copia el elemento
			i++; //pasa al siguiente caracter
			element = realloc(element, i*sizeof(char)); //reserva mas memoria
		}
		if(A[i]!='\0'){ //si no se llego al final
			element[i-1]='\0'; // fin de la cadena
			//el siguiente bloque modifica la original elimando un elemento
			k=1; //inicia en uno dejando la '{' o '[' original para la nueva cadena
			i++; // salta la ',' que fue el corte del ciclo
			while(A[i]!='\0'){
				A[k]=A[i];
				k++;
				i++;
			}
			A[k]='\0'; //finaliza la copia en A
		}else{
			element[i-2]='\0'; // fin de la cadena
			A[0]='\0'; // queda sin elementos A
		}
	}
	return element;
}
		
char * leeCad(){
	char * aux = NULL;
	char * cad = NULL;
	size_t tam = 0;
	ssize_t tam_cad = getline(&aux, &tam, stdin);
	aux[tam_cad-1]='\0'; //porque guarda \n en la anteultima posicion
	cad = (char *) malloc(tam_cad * sizeof(char));
	strcpy(cad, aux);
	return cad;
}

int compara_conj(str A, str B){
	str auxB = (str)malloc(sizeof(char)*strlen(B));
	strcpy(auxB, B);
	int band;
	if(strlen(A)==strlen(auxB)){
		band = 1;
		str cad;
		while((cad=obtenerElemento(auxB))!=NULL){
			if(strstr(A,cad)==NULL){
				band=0;
			}
		}
	}else{
		band = 0;
	}
	return band;
}
