#include "tData.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// ingresa el tipo STR, SET, LIST
tData nvo_nodo(int t){
	tData nvo = (tData)malloc(sizeof(struct dataType));
	nvo->nodeType = t;
	nvo->data = NULL;
	nvo->next = NULL;
	nvo->elem = NULL;
	nvo->num = 0;
	nvo->numf = 0;
	nvo->boolean = false;
	return nvo;
}
	
void carga_elem(tData nodo, str elem){
	if(nodo!=NULL&&returnType(nodo)==ELEM){
		nodo->elem = elem;
	}
}
void carga_bool(tData nodo, bool boolean){
	if(nodo!=NULL&&returnType(nodo)==BOOL){
		nodo->boolean = boolean;
	}
}
void carga_num(tData nodo, int num){
	if(nodo!=NULL&&returnType(nodo)==NUM){
		nodo->num = num ;
	}
}	
	/// retorna un padre del tipo especificado y lo aniade a lo recibido como parametro como su hijo
tData newNestedData(tData d, int t){
	tData nvo = NULL;
	if(d!=NULL){
		if(t!=ELEM&&t!=BOOL&&t!=NUM){
			nvo = nvo_nodo(t);
			nvo->data = d;
		}
	}
	return nvo;
}
		
int typeOf(char *s){
	int t = ELEM;
	if(s[0] == '{' && s[tam_cad(s)-1]=='}'){
		t = SET;
	}else if(s[0] == '[' && s[tam_cad(s)-1]==']'){
		t = LIST;
	}else if(s[0] != '"'){
		t = NUM;
	}else if (compara_cad(s,"true")==0 || compara_cad(s,"false")==0)
		t = BOOL;
	return t;
}
	
	/// falta que no deje entrar como conj vacio ''	
	
tData newData(char *s){
	int t;
	tData cab = NULL;
	t = typeOf(s);
	cab = nvo_nodo(t);
	if(t!=ELEM && t!=NUM && t!=BOOL){
		cab->data = newData(obtenerElemento(s));
		if(tam_cad(s)>0)
			cab->next = newData(s);
	}else{
		if(tam_cad(s)>0) {//no carga una cadena vacia en su lugar se representa como vacio si no hay datos
			if(t==ELEM)
			carga_elem(cab, s);
			if(t==NUM)
			carga_num(cab, atoi(s));
			if(t==BOOL){
				compara_cad(s,"false") == 0 ? carga_bool(cab,false) : carga_bool(cab,true);
			}
		}
		else return NULL;
	}
	return cab;
}
	
int returnType(tData d){
	if(d!=NULL){
		return d->nodeType;
	}
	return -1;
}

void dataFree(tData *d){
	if(returnType(*d) == ELEM||returnType(*d) == NUM || returnType(*d) == BOOL || returnType(*d) == FLOAT){
		free(*d);
		*d = NULL;
	}else if((*d)!=NULL){
		dataFree(&((*d)->data));
		free((*d)->data);
		dataFree(&((*d)->next));
		free((*d)->next);
		free(*d);
		*d = NULL;
	}
}

int cant_nodo_rec(tData cab, int i){
	if(cab==NULL){
		return 1;
	}else if(returnType(cab)==ELEM || returnType(cab) == NUM || returnType(cab) == BOOL || returnType(cab) == FLOAT){
		return i+1;
	}
	return cant_nodo_rec(cab->next, i)+cant_nodo_rec(cab->data, i);
}

int cant_nodo(tData cab){
	return cant_nodo_rec(cab, 0);
}

int isEqual(tData d1, tData d2){
	int b = -1;
	tData aux = d2;
	if(((returnType(d1) == returnType(aux)) || (returnType(d1) == NUM && returnType(aux) == FLOAT) || (returnType(d1) == FLOAT && returnType(aux) == NUM)) && cant_nodo(d1)==cant_nodo(aux)){ ///caso base
		if(returnType(d1)==SET){
			while(d1!=NULL && b!=0){
				b=-1; ///reinicia para hacer una nueva busqueda
				while(aux!=NULL && b!=1){
					b = isEqual(d1->data, aux->data);
					aux = aux->next;
				}
				aux = d2;
				d1 = d1->next;
			}
			return b;
		}else if(returnType(d1)==LIST){
			while(d1!=NULL && b!=0){
				b = isEqual(d1->data, aux->data);
				d1 = d1->next;
				aux = aux->next;
			}
			return b;
		}else if(returnType(d1)==ELEM){ ///caso base
			b = compara_cad(d1->elem, aux->elem) == 0 ? 1 : 0;
			return b;
		}else if(returnType(d1)==BOOL){
			b = d1->data == aux->data ? 1 : 0;
			return b;
		}else if (returnType(d1)==FLOAT || returnType(aux) == FLOAT) {
			double x,y;
			x = d1->numf;
			y = aux->numf;
			if(returnType(d1)==NUM){
				x = d1->num;
			}else if(returnType(aux)==NUM){
				y = aux->num;
			}
			b = x == y ? 1 : 0;
			return b;
		}else if(returnType(d1)==NUM && returnType(aux) == NUM){
			b = d1->num == aux->num ? 1 : 0;
			return b;
		}
		else{
			return 1;
		}
	}
	return 0;
}
		
tData INDATA(tData S, tData elem){
	tData r = nvo_nodo(BOOL);
	bool b=false;
	if(returnType(S) == SET || returnType(S) == LIST){ 
		while(S!=NULL && b!=true){
			if(isEqual(S->data, elem)==1)
				b=true;
			S = S->next;
		}
	}
	r->boolean=b;
	return r;
}

tData INCLUDE(tData d1,tData d2){
	tData r = nvo_nodo(BOOL);
	tData aux;
	tData runner = d1;
	bool b = true;
	if(!isEqual(d1,d2)){
		if((returnType(d1)==SET && returnType(d2) == SET || returnType(d1)==LIST && returnType(d2) == LIST) && !isEmpty(d2)){
			while(!isEmpty(runner) && b){
				aux=INDATA(d2,runner->data);
				b = aux->boolean;
				runner=runner->next;
				dataFree(&aux);
			}
		}
		else{
			b= false;
		}
	}
	r->boolean=b;
	return r;
}
	
void printData(tData d){
	tData sig = NULL;
	if(d!=NULL){
		int t = d->nodeType;
		if(t == SET){
			printf("{");
		}else if(t == LIST){
			printf("[");
		}else if(t == ELEM){
			printf("%s", d->elem);
		}else if(t == NUM){
			printf("%ld", d->num);
		}else if(t == FLOAT){
			printf("%.12lf", d->numf);
		}else if(t == BOOL){
			switch(d->boolean){
				case true: printf("true"); break;
				default: printf("false");
			}
		}
		while(d!=NULL && t!=ELEM && t!=NUM && t!=BOOL && t!=FLOAT){
			sig = d->next;
			if(sig!=NULL){
				if(d->data!=NULL){
					if(d->data->nodeType == ELEM){
						printf("%s,", d->data->elem);
					}else if(d->data->nodeType == NUM){
						printf("%ld,", d->data->num);
					}else if(d->data->nodeType == FLOAT) {
						printf("%.12lf,", d->data->numf);
					}else if(d->data->nodeType == BOOL){
						switch(d->data->boolean){
						case true: printf("true,"); break;
						default: printf("false,");
						}
					}
					else{
						printData(d->data);
						printf(",");
					}
				}
				
			}else{
				if(d->data!=NULL){
					if(d->data->nodeType == ELEM){
						printf("%s", d->data->elem);
					}else if(d->data->nodeType == NUM){
						printf("%ld", d->data->num);
					}else if(d->data->nodeType == FLOAT){
						printf("%.12lf", d->data->numf);
					}else if(d->data->nodeType == BOOL){
						switch(d->data->boolean){
						case true: printf("true"); break;
						default: printf("false");
						}
					}
					else{
						printData(d->data);
					}
				}
				
			}
			d = sig;
		}
		if(t == SET){
			printf("}");
		}else if(t == LIST){
			printf("]");
		}
	}else{
		printf("null");
	}
}

char* intToString(int num) {
   	char *str = (char *)malloc(12 * sizeof(char)); 
    if (str == NULL) {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

char* floatToString(double num) {
   	char *str = (char *)malloc(50 * sizeof(char)); 
    if (str == NULL) {
        return NULL;
    }
    sprintf(str, "%.12lf", num);
    return str;
}

char* toStr(tData d){
	tData sig = NULL;
	if(d!=NULL){
		str aux = NULL;
		str cad = (str) malloc(sizeof(char)*500);
		copia_cad(cad, "");
		int t = d->nodeType;
		if(t == SET){
			concat_cad(cad, "{");
		}else if(t == LIST){
			concat_cad(cad, "[");
		}else if(t == ELEM){
			concat_cad(cad, d->elem);
		}else if(t == NUM){
			concat_cad(cad, intToString(d->num));
		}else if(t == FLOAT){
			concat_cad(cad, floatToString(d->numf));
		}
		while(d!=NULL && t!=ELEM && t!=NUM && t!=FLOAT){
			sig = d->next;
			if(sig!=NULL){
				if(d->data!=NULL){
					if(d->data->nodeType == ELEM){
						concat_cad(cad, d->data->elem);
						concat_cad(cad, ",");
					}else if(d->data->nodeType == NUM){
						concat_cad(cad, intToString(d->data->num));
						concat_cad(cad, ",");
					}else if(d->data->nodeType == FLOAT){
						concat_cad(cad, floatToString(d->data->numf));
						concat_cad(cad, ",");
					}else{
						aux = toStr(d->data);
						concat_cad(cad, aux);
						concat_cad(cad, ",");
					}
				}
			}else{
				if(d->data!=NULL){
					if(d->data->nodeType == ELEM){
						concat_cad(cad, d->data->elem);
					}else if(d->data->nodeType == NUM){
						concat_cad(cad, intToString(d->data->num));
					}else if(d->data->nodeType == FLOAT){
						concat_cad(cad, floatToString(d->data->numf));
					}else{
						aux = toStr(d->data);
						concat_cad(cad, aux);
					}
				}
			}
			d = sig;
		}
		if(t == SET){
			concat_cad(cad, devuelve_cad("}"));
		}else if(t == LIST){
			concat_cad(cad, devuelve_cad("]"));
		}
		return cad;
	}
	return devuelve_cad("null");
}

void copyDataAux(tData O,tData C){
	if(!isEmpty(O)){
		if(returnType(O)==ELEM){
			C->elem = devuelve_cad(O->elem);
		}
		else if(returnType(O)==NUM || returnType(O)==BOOL || returnType(O) == FLOAT){
			C->data = O->data;
		}
		else{
			C->data=nvo_nodo(returnType(O->data));
			copyDataAux(O->data,C->data);
			if(O->next!=NULL){
				C->next=nvo_nodo(returnType(O->next));
				copyDataAux(O->next,C->next);
			}
		}
	}
}
tData ToNewType(tData d,int tipe){
	tData new = nvo_nodo(tipe);
	copyDataAux(d,new);
	return new;
}
tData copyData(tData d){
	tData new = nvo_nodo(returnType(d));
	copyDataAux(d,new);
	return new;
}

int isEmpty(tData A){
	if (A == NULL || A->data == NULL){
		return 1;
	}else{
		return 0;
	}
}

///LISTAS FUNCIONES
		
tData SIZEDATA(tData l){
	tData ret = nvo_nodo(NUM);
	long* c = &(ret->num);
	if(returnType(l)==LIST || returnType(l)==SET){
		*c=0;
		while(!isEmpty(l)){
			(*c)++;
			l=l->next;
		}
	}
	return ret;
}
	
tData returnElemData(tData d, tData pos){
	tData ret = NULL;
	if((returnType(d)==LIST || returnType(d)==SET) && returnType(pos)==NUM){
		if(pos->num>=0&&pos->num<SIZEDATA(d)->num){
			int c = -1;
			while(d!=NULL && c!=pos->num){
				ret = copyData(d->data);
				d = d->next;
				c++;
			}
		}
	}
	return ret;
}

tData returnElem(tData d, int pos){
	tData ret = NULL;
	if(returnType(d)==LIST || returnType(d)==SET){
		if(pos>=0&&pos<SIZEDATA(d)->num){
			int c = 0;
			while(d!=NULL && c!=pos){
				d = d->next;
				c++;
			}
			ret = copyData(d->data);
		}
	}
	return ret;
}
	
void PUSH(tData L, tData dato){
	tData last = nvo_nodo(SET);
	if(returnType(L) == SET){ 
		if (!isEmpty(L)){
			while(L->next!=NULL){
				L = L->next;
			}
			L->next = last;
			last->data = dato;
		}
		else{
			L->data = dato;
		}
		
	}
}

void ADDOP(tData L, tData dato){
	if(returnType(L) == SET || returnType(L) == LIST){
		tData last = nvo_nodo(returnType(L)); 
		if (!isEmpty(L)){
			while(L->next!=NULL){
				L = L->next;
			}
			L->next = last;
			last->data = dato;
		}
		else{
			L->data = dato;
		}
		
	}
}
	
tData POPA(tData L){
	tData cab = L;
	tData aux = NULL;
	tData elem = NULL;
	if ((L)->data!=NULL){
		if ((L)->next!=NULL){
			if(returnType(cab) == LIST || returnType(cab) == SET){ 
				while(cab->next!=NULL){
					if ((cab->next)->next==NULL){
						aux = cab;
					}
					cab = cab->next;
				}
				elem = copyData(cab->data);
				free(cab);
				aux->next = NULL;
			}
		}
		else{
			elem = copyData((L)->data);
			free((L)->data);
			(L)->data = NULL;
		}
	}
	return elem;
}
	
	///CONJUNTOS FUNCIONES
	
	/// cardinal no devuelve 0
int CARDINAL(tData s){
	int c=0;
	if(returnType(s)==SET){
		//printData(s);
		while(!isEmpty(s)){
			c++;
			s=s->next;
		}
	}
	return c;
}

tData UNIONA(tData A,tData B){
	tData C = NULL;
	tData aux = NULL;
	if(returnType(A)==returnType(B)&&returnType(A)==SET){
		C = copyData(A);
		aux = C;
		
		while(C->next!=NULL){
			C = C->next;
		}
		if(!isEmpty(B)){
			while(!isEmpty(B)){
				if (isEmpty(C)){
					C->data = copyData(B->data);
					if (B->next!=NULL){
						C->next = newData(devuelve_cad("{}"));
					}
					C=C->next;
				}
				else{
					C->next = newData(devuelve_cad("{}"));
					(C->next)->data	= copyData(B->data);
					C=C->next;
				}
				B=B->next;
			}
		}
	}
	
	return aux;
}

tData INTERA(tData A,tData B){
	tData C = NULL, aux = NULL;
	C=nvo_nodo(SET);
	aux=C;
	if(returnType(A)==returnType(B)&&returnType(A)==SET){
		if(!isEmpty(A)&&!isEmpty(B)){
			while(!isEmpty(A)){
				if (INDATA(B,A->data)->boolean){

				
					if (isEmpty(aux)){
						C->data = copyData(A->data);
					}
					else{
						if (C->next!=NULL){
							C=C->next;
						}
						C->next = newNestedData(A->data,2);
					}
					/*if (A->next!=NULL){
					C->next = newData(devuelve_cad("{}"));
					}
				
					C=C->next;*/
				}
				A=A->next;
			}
		}
	}
	return aux;
}

tData DIFFA(tData A,tData B){
tData C = NULL, aux = NULL;
	C=nvo_nodo(SET);
	aux=C;
	if(returnType(A)==returnType(B)&&returnType(A)==SET){
		if(!isEmpty(A)&&!isEmpty(B)){
			while(!isEmpty(A)){
					if (!INDATA(B,A->data)->boolean){
					if (isEmpty(aux)){
						C->data = copyData(A->data);
					}
					else{
						if (C->next!=NULL){
							C=C->next;
						}
						C->next = newNestedData(A->data,2);
					}
				}	
				A=A->next;
			}
		}
		else{
			if(isEmpty(B)){
				aux=copyData(A);
			}
		}
	}
	return aux;
}
void Depurar(tData L) {
	tData cab = L;  // Cabeza de la lista/conjunto
	tData aux = NULL; // Para almacenar el nodo previo
	tData tmp = NULL; // Para eliminar nodos duplicados
	
	if (cab == NULL || cab->data == NULL) {
		return; // Nada que depurar si la lista/conjunto esta vacio
	}
	/*if (returnType(cab) == LIST) {
	// Eliminar duplicados consecutivos en una lista
	while (cab->next != NULL) {
	if (isEqual(cab->data, cab->next->data)) { 
	// Si el siguiente elemento es duplicado, eliminarlo
	tmp = cab->next;
	cab->next = cab->next->next;
	free(tmp);
	} else {
	cab = cab->next; // Avanzar si no hay duplicado
	}
	}
	} else*/
	if (returnType(cab) == SET) {
		// Eliminar todos los duplicados en un conjunto
		tData current = L; 
		while (current != NULL && current->next != NULL) {
			aux = current;
			tData runner = current->next;
			while (runner != NULL) {
				if (isEqual(current->data, runner->data)) {
					// Encontrado duplicado en el conjunto
					tmp = runner;
					aux->next = runner->next; // Saltar duplicado
					runner = runner->next;
					free(tmp);
				} else {
					aux = runner; // Avanzar
					runner = runner->next;
				}
			}
			current = current->next;
		}
	}
}

tData SQRTOP(tData A){
	tData res;
	res = nvo_nodo(FLOAT);
	if(returnType(A)==FLOAT){
		res->numf = sqrt(A->numf);
	}else if(returnType(A)==NUM){
		res->numf = sqrt((double)A->num);
	}
	return res;
}

tData NUMOP(char OP, tData A, tData B){
	tData res;
	if(returnType(A)==FLOAT || returnType(B)==FLOAT){
		res = nvo_nodo(FLOAT);
		double a,b;
		a = A->numf;
		b = B->numf;
		if(returnType(A)==NUM){
			a = A->num;
		}else if(returnType(B)==NUM){
			b = B->num;
		}
		switch (OP){
			case '+': res->numf = a+b; break;
			case '-': res->numf = a-b; break;
			case '*': res->numf = a*b; break;
			case '/': case 'D': res->numf = a/b; break;
			case '^': res->numf = pow(a,b); break;
			default: res->numf = 0;
		}
	}
	else{
		if(OP!='D'){
			res = nvo_nodo(NUM);
			switch (OP){
				case '+': res->num = A->num + B->num; break;
				case '-': res->num = A->num - B->num; break;
				case '*': res->num = A->num * B->num; break;
				case '/': res->num = A->num / B->num; break;
				case '%': res->num = A->num % B->num; break;
				case '^': res->num =(int)pow(A->num,B->num); break;
				default: res->num = 0;
			}
		}else{
			res = nvo_nodo(FLOAT);
			res->numf = (double)A->num/(double)B->num;
		}
	}
	return res;
}

int comparation(tData A, tData B){
	int res = 0;
	double a,b;
	if(returnType(A) == SET || returnType(A) == LIST){
		res = SIZEDATA(A)->num > SIZEDATA(B)->num;
	}
	if(returnType(A) == FLOAT || returnType(B) == FLOAT){
		a = A->numf;
		b = B->numf;
		if(returnType(A)==NUM){
			a = A->num;
		}else if(returnType(B)==NUM){
			b = B->num;
		}
		res= a > b;
	} else if (returnType(A) == NUM){
		res= A->num > B->num;
	}
	if(returnType(A) == ELEM){
		res = compara_cad(A->elem, B->elem) > 0 ? 1 : 0;
	}
	if(returnType(A) == BOOL){
		res = A-> boolean > B->boolean;
	}
	return res;
}

tData COMPDATA(int type, tData A, tData B){
	tData res = nvo_nodo(BOOL);
	switch (type)
	{
		case 1:{  // >
			res->boolean = comparation(A,B);
		}
		break;
		case 2:{ // <
			res->boolean = comparation(B,A);
		}
		break;
		case 3:{ // <>
			res->boolean = !isEqual(A,B);
		}
		break;
		case 4:{ // ==

			res->boolean = isEqual(A,B);
		}
		break;
		case 5:{ // >=
			if(returnType(A) == SET || returnType(A) == LIST){
				res->boolean = comparation(A,B) || isEqual(SIZEDATA(A),SIZEDATA(B));
			}
			else{
				res->boolean = comparation(A,B) || isEqual(A,B);
			}
		}
		break;
		case 6:{ // <=
			res->boolean = comparation(B,A) || isEqual(A,B);
		}
		break;
	}
	return res;
}

int isTrue(tData A){
	int ret = 0;
	switch (returnType(A))
	{
		case SET: case LIST:{
			ret = !isEmpty(A);
		}
		break;
		case NUM:{
			ret = A->num == 0 ? 0 : 1;
		}
		break;
		case FLOAT:{
			ret = A->numf == 0 ? 0 : 1;
		}
		break;
		case ELEM:{
			ret = compara_cad(A->elem,"") == 0 ? 0 : 1;
		}
		break;
		case BOOL:{
			ret = A->boolean;
		}
		break;
		default:
			ret = false;
		break;
	}
}

tData RANGEPERUANO(tData INI,tData FIN, tData PASO){
	tData list = nvo_nodo(LIST);
	tData data;
	int ini = 0;
	int fin = 0;
	int paso = 1;

	if(INI){
		if(FIN){
				if(returnType(INI) == NUM && returnType(FIN) == NUM){
					ini = INI->num;
					fin = FIN->num;
				}
			if(PASO){
				if(returnType(PASO) == NUM)
				paso = PASO->num;
			}
		}else{
			if(returnType(INI) == NUM)
			fin = INI->num;
		}
	}
	
	for (int i = ini; (paso>0 && i<fin) || (paso<0 && i>fin); i+=paso){
		data = nvo_nodo(NUM);
		data->num = i;
		ADDOP(list, data);
	}

	return list;
}

tData RANGELIST2(tData INI,tData FIN, tData PASO){
	tData list;
	list = nvo_nodo(LIST);
	tData elem;
	int ini,fin,paso;
	if (INI && returnType(INI) == NUM){
		ini = 0;
		fin = INI->num;
		paso = 1;
		if (PASO && returnType(PASO) == NUM){
			ini = INI->num;
			fin = FIN->num;
			paso = PASO->num;	
		}			
		else if (FIN && returnType(FIN) == NUM){
			ini = INI->num;
			fin = FIN->num;
			if (ini>fin)
				paso = -1;
			else
				paso = 1;
		}
		if (paso!=0){
			if (paso>0){
				for (int i = ini; i < fin; i+=paso){
				elem = nvo_nodo(NUM);
				elem->num = i;
				ADDOP(list,elem);
				}
			}
			else {
				for (int i = ini; i > fin; i+=paso){
				elem = nvo_nodo(NUM);
				elem->num = i;
				ADDOP(list,elem);
				}
			}
			
		}
	}	
	return list;	
}

tData REPLACE(tData A, tData pos, tData B){
	tData ret = copyData(A);
	tData runner = ret;
	int i = 0;
	while(runner && i<(pos->num)){
		runner = runner->next;
		i++;
	}
	if(runner){
		runner->data = copyData(B);
	}
	return ret;
}