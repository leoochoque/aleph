#ifndef TDATA_H
#define TDATA_H

#define ELEM 1
#define SET 2
#define LIST 3
#define NUM 4
#define BOOL 5
#define FLOAT 6
#define FUN 7
#define STRCTDEF 8
#define STRCINS 9
#define TNULL 10

#include "Auxiliar.h"

typedef enum{
	false,true
}bool;

typedef struct dataType* tData;

struct syml;
struct ast;
struct Env;

struct dataType{
	int nodeType;
	union{
		str elem;
		long num;
		bool boolean;
		double numf;
		struct{
			struct syml *params;
			struct ast *body;
			struct Env *context;
		}function;
		struct {
			char * name;
			struct syml * params;
			struct ast * body;
		}structDef;
		struct{
			struct dataType *definition;
			struct Env *context;
		}structIns;
		struct{
			struct dataType* data;
			struct dataType* next;
		};
	};
};

/*operaciones generales*/

//a partir de una cadena dada por el usuario, crea un nuevo dato (ATM, SET o LIST)
/// newData("{a,b,c,[1,2,3]}");

tData nvo_nodo(int t);
tData newData(char *s);
//a partir de un tData crea uno nuevo anidando (nest) el tData recibido como hijo derecho
tData newNestedData(tData d, int t); /// implementado pero no utilizado
//elimina un dataSL
void dataFree(tData* d); /// falta implementar reemplazar en la fn de faker
//imprime un dato por pantalla
void printData(tData d);
//retorna tipo de dato
int returnType(tData d);
//convierte tData a cadena
char* toStr(tData d);
//copia un tData en otro tData
//tData copyData(tData d);

tData copyData(tData d);
tData ToNewType(tData d,int tipe);

void Depurar(tData L);
/*operaciones con SET y LIST*/
//Es vac�o
int isEmpty(tData d);
//Son iguales
int isEqual(tData d1, tData d2);

/*operaciones con LIST*/
/*Retorna elemento en posici�n pos dentro del SLData*/
tData returnElemData(tData d, tData pos); /// retorna elemtno
tData returnElem(tData d, int pos);
//agrega un elemento a la lista por el final 
void PUSH(tData L, tData elem);
void ADDOP(tData L, tData dato);
//elimina el �ltimo elemento de la lista y lo devuelve como salida
tData POPA(tData L); 
//retorna el tama�o de la lista
tData SIZEDATA(tData L); 
tData RANGELIST2(tData INI,tData FIN, tData PASO);
tData RANGEPERUANO(tData, tData, tData);
//Se pueden agregar operaciones de ordenamiento y b�squeda
//Se puede agregar operaci�n de inserci�n y eliminaci�n

/*operaciones con SET*/
//calcula la cantidad de elementos de un conjunto
int CARDINAL(tData S);
//determina si un elemento pertenece a un conjunto
tData INDATA(tData S, tData elem);
//genera un nuevo conjunto que resulta de la uni�n de dos conjuntos
tData UNIONA(tData A,tData B);
//genera un nuevo conjunto que resulta de la intersecci�n de dos conjuntos
tData INTERA(tData A,tData B);
//genera un nuevo conjunto que resulta de la diferencia de dos conjuntos
tData DIFFA(tData A,tData B);

tData NUMOP(char OP,tData A, tData B);

tData SQRTOP(tData A);

tData COMPDATA(int type, tData A, tData B);

tData INCLUDE(tData d1,tData d2);

tData CONCATA(tData A,tData B);

tData TAILA(tData A);

int isTrue(tData A);

tData REPLACE(tData A, tData pos, tData B);

tData newNull();

#endif
