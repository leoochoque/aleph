#include "aleph.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void yyerror(char *s, ...);
int contador = 0;

tData RETURNVAL = NULL;
int RETURNSTATE = 0;

/* Tabla de símbolos */

struct symbol symtab[NHASH];

static unsigned symhash(char *sym){
    unsigned int hash = 0;
    unsigned c;
    
    while(c = *sym++) hash = hash*9 ^ c;
    
    return hash;
}

struct symbol *lookup(char *sym){
    struct symbol *sp = &symtab[symhash(sym)%NHASH];
    int scount = NHASH;
    
    while(--scount >= 0){
        if(sp->name && !compara_cad(sp->name, sym)) 
            return sp;
        
        if(!sp->name){
            sp->name = devuelve_cad(sym);
            sp->value = NULL;
            sp->bodyfn = NULL;
            sp->params = NULL;
            return sp;
        }
        
        if(++sp >= symtab+NHASH)
            sp = symtab;
    }
    yyerror("Out of memory!\n");
    abort();
}

struct ast *newasgn(struct syml *li, struct expl *le){
    struct symasgn *sym = malloc(sizeof(struct symasgn));
    sym->s = li;
    sym->l = le;
    sym->nodetype = ASGN;
    return (struct ast *)sym;
}

struct ast *newref(struct symbol *s){
    struct symref *sym = malloc(sizeof(struct symref));
    sym->nodetype = REF;
    sym->s = s;
    return (struct ast *)sym;
}

/* Tabla de símbolos */

struct ast *newast(int nodetype, struct ast *l, struct ast *r){
    struct ast *a = malloc(sizeof(struct ast));
    
    if(!a){
        yyerror("no hay espacio");
        exit(0);
    }
    
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newElem(char *cad){
    struct elem *e = malloc(sizeof(struct elem));
    e->nodetype = TELEM;
    e->elem = devuelve_cad(cad);
    return (struct ast *)e;
}

struct ast *newNumber(int num){
    struct num *n = malloc(sizeof(struct num));
    n->nodetype = TNUM;
    n->num = num;
    return (struct ast *)n;
}

struct ast *newNumberFloat(double numf){
    struct numfloat *f = malloc(sizeof(struct numfloat));
    f->nodetype = TFLOAT;
    f->numf = numf;
    return (struct ast *)f;
}

struct ast *newBoolean(bool boolean){
    struct boolean *b = malloc(sizeof(struct boolean));
    b->nodetype = TBOOL;
    b->boolean = boolean;
    return (struct ast *)b;
}

struct expl *newexpl(struct ast *a, struct expl *next){
    struct expl *l = malloc(sizeof(struct expl));
    l->a = a;
    l->next = next;
    return l;
}

struct syml *newsyml(struct symbol *s, struct syml *next){
    struct syml *l = malloc(sizeof(struct syml));
    l->s = s;
    l->next = next;
    return l;
}

struct ast *newflow(int nodetype, struct ast * cond, struct ast * tl, struct ast * fl)
{
    struct flow *a = malloc(sizeof(struct flow));
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->fl = fl;
    return (struct ast *)a;
};

struct ast * newcomprenshion(int nodetype, struct ast * op, struct ast * var, struct ast * iterable,struct ast * cond){
    struct comprehension *a = malloc(sizeof(struct comprehension));
    a->nodetype = nodetype;
    a->op = op;
    a->var = var;
    a->iterable = iterable;
    a->cond = cond;
    return (struct ast *)a;
};

void newfunc(struct symbol *name, struct syml * symlist, struct ast *block){
    //falta hacer free a bodyfn y params
    if(name->bodyfn) free(name->bodyfn);
    if(name->params) free(name->params);
    name->bodyfn = block;
    name->params = symlist;
};

struct ast *newcall(struct symbol *name, struct expl *explist){
    struct fncall* fn = malloc(sizeof(struct fncall));
    fn->nodetype = FUNC;
    fn->s = name;
    fn->explist = explist;
    return (struct ast*)fn;
};

struct ast *newset(struct symbol *sym, struct ast *pos, struct ast *expend){
    struct set* a = malloc(sizeof(struct set));
    a->nodetype = SETTER;
    a->s = sym;
    a->b = pos;
    a->c = expend;
    return (struct ast *)a;
};

struct ast *newlambda(struct syml *symlist, struct ast * exp){
    struct lambda* l = malloc(sizeof(struct lambda));
    l->nodetype = LAMBDA;
    l->symlist = symlist;
    l->expreturn = newast(RETURNKEYW, exp, NULL);
    return (struct ast *)l;
};

tData callfunc(struct fncall* a){
    struct symbol *sym = a->s;
    struct expl* explist = a->explist; //args actuales
    struct ast* body = sym->bodyfn;
    struct syml* params; //args formales
    tData *oldval, *newval, ret;
    int i;
    int nargs;

    
    if(!body){
        printf("Function %s is not defined", sym->name);
        exit(1);
    }
   
    params = sym->params;
    for(nargs = 0; params; params = params->next)
        nargs++;

    oldval = malloc(nargs*sizeof(tData));
    newval = malloc(nargs*sizeof(tData));
   
    if(!oldval || !newval){
        printf("Out of memory!");
    }

    i=0;
    while(explist){
        newval[i] = eval(explist->a);
        explist = explist->next;
        i++;
    }

    if(nargs != i){
      printf("Expected %d arguments for the function %s",nargs,sym->name);
      exit(1);
    }

    params = sym->params;
    i = 0;
    while(params){
        oldval[i] = params->s->value;
        params->s->value = newval[i];
        params = params->next;
        i++;
    }

    eval(body);
    RETURNSTATE = 0;

    ret = RETURNVAL;

    params = sym->params;
    i = 0;
    while(params){
        params->s->value = oldval[i];
        params = params->next;
        i++;
    }

    return ret;
}

tData eval(struct ast *a){
    tData ret = NULL;
    tData Right = NULL, Left = NULL;
    int typeL, typeR;
    if(a && !RETURNSTATE)
        switch(a->nodetype){
            /*
                ">" <-> 1
                "<" <-> 2
                "<>" <-> 3
                "==" <-> 4
                ">=" <-> 5
                "<=" <-> 6
            */
            case 1: case 2: case 3: case 4: case 5: case 6:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if(typeL==typeR || (typeL==FLOAT && typeR == NUM) || (typeL==NUM && typeR == FLOAT)){
                    ret = COMPDATA(a->nodetype, Left, Right);
                }else{
                    yyerror("Error: Invalid comparation different types\n");
                    exit(1);
                }
            }
            break;
            case TSET: case TLIST:{
                if(a->nodetype == TSET)
                    ret = nvo_nodo(SET);
                else{
                    ret = nvo_nodo(LIST);
                }
                if(a->l){
                    struct expl* list = (struct expl *)(a->l);
                    while (list!=NULL){
                        ADDOP(ret, eval(list->a));
                        list = list->next;
                    }
                    if(returnType(ret) == SET);
                        Depurar(ret);
                }
            }
            break;
            case USET:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if(typeL == SET && typeR == SET){
                    ret = UNIONA(Left, Right);
                    Depurar(ret);
                }
                else{
                    yyerror("Error: Expected type set\n");
                    exit(1);
                }
            }
            break;
            case DSET:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if(typeL == SET && typeR == SET){
                    ret = DIFFA(Left, Right);
                    Depurar(ret);
                }
                else{
                    yyerror("Error: Expected type set\n");
                    exit(1);
                }
            }
            break;
            case ISET:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if(typeL == SET && typeR == SET){
                    ret = INTERA(Left, Right);
                    Depurar(ret);
                }
                else{
                    yyerror("Error: Expected type set\n");
                    exit(1);
                }
            }
            break;
            case PLIST:{
                Left = eval(a->l);
                if(returnType(Left) == LIST|| returnType(Left) == SET){
                    if (!isEmpty(Left)) 
                        ret = POPA(Left);
                    else{
                        yyerror("Error: Pop from empty list\n");
                        exit(1);
                    }

                }
                else{
                    yyerror("Error: Expected type list\n");
                    exit(1);
                }
            }
            break;
            case ADDTO:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if(typeR == SET || typeR == LIST){
                    ret = Right;
                    ADDOP(ret,Left);
                    if (returnType(ret) == SET){
                        Depurar(ret);
                    }            
                }
            }
            break;
            case GETTER:{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                tData aux = eval(a->r);
                if((typeL == SET || typeL == LIST) && typeR == NUM){
                    if ((int)Right->num < (SIZEDATA(Left)->num) && (int)Right->num >=0) {
                        ret = returnElemData(Left, Right); //si te pasas (a->r)
                    }
                    else{
                        yyerror("Error: index out of range\n");
                        exit(1);
                    }
                }else{
                    yyerror("Error: Unexpected type in get operation");
                    exit(1);
                }
            }
            break;
            case SETTER:{
                // set x[0] as 4
                struct set *setter = (struct set *)a;
                struct symbol *s = setter->s;
                Left = eval(setter->b);
                Right = eval(setter->c);
                if (returnType(s->value)==LIST){
                    if ((int)Left->num < (SIZEDATA(s->value))->num && (int)Left->num >=0)
                        s->value = REPLACE(s->value, Left, Right);
                    else{
                        yyerror("Error: index out of range\n");
                        exit(1);
                    }
                }
                else{
                    yyerror("Error: Set is only valid for List\n");
                    exit(1);
                }     
            }   
            break;
            case TO_SET:{
                Left= eval(a->l);
                if(returnType(Left) == LIST|| returnType(Left) == SET){
                    ret = ToNewType(Left,SET);
                    Depurar(ret);
                }
            }
            break;
            case TO_LIST:{
                Left= eval(a->l);
                if(returnType(Left) == SET|| returnType(Left) == LIST){
                    ret = ToNewType(Left,LIST);
                }
            }
            break;
            
            case TELEM:{
                ret = nvo_nodo(ELEM);
                ret->elem = ((struct elem *)a)->elem;
            }
            break;
            case TNUM:{
                ret = nvo_nodo(NUM);
                ret->num = ((struct num *)a)->num;
            }
            break;
            case TFLOAT:{
                ret = nvo_nodo(FLOAT);
                ret->numf = ((struct numfloat *)a)->numf;
            }
            break;
            case TBOOL:{
                ret = nvo_nodo(BOOL);
                ret->boolean = ((struct boolean *)a)->boolean;
            }
            break;
            case '+':{
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if (typeL == ELEM || typeR == ELEM){
                    ret = nvo_nodo(ELEM);
                    ret->elem = concat_cad(toStr(Left),toStr(Right));
                }
                else{
                    if((typeL == NUM || typeL == FLOAT) && (typeR == NUM || typeR == FLOAT))
                        ret = NUMOP(a->nodetype,Left, Right);
                    else{
                        yyerror("Error: Unexpected type in arithmetic operation \n");
                        exit(1);
                    }
                }
            }
            break;
            case '-': case '*': case '/': case '%': case '^': case 'D':{
                //PROBAR
                Left = eval(a->l);
                Right = eval(a->r);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if((typeL == NUM || typeL == FLOAT) && (typeR == NUM || typeR == FLOAT)){
                    if(a->nodetype == '%' && typeL == FLOAT && typeR == FLOAT){
                        yyerror("Error: Invalid operands");
                        exit(1);    
                    }
                    else{
                        ret = NUMOP(a->nodetype,Left, Right);
                    }
                }
                else{
                    yyerror("Error: Unexpected type in aritmethic operation \n");
                    exit(1);  
                }
            }
            break;
            case 'S':{
                Left = eval(a->l);
                typeL = returnType(Left);
                if(typeL == NUM || typeL == FLOAT)
                    ret = SQRTOP(Left);
                else{
                    yyerror("Error: Square root only valid for numbers");
                    exit(1);
                }
            }
            break;
            case 'M':{
                Left = eval(a->l);
                typeL = returnType(Left);
                if (typeL == NUM){
                    ret = Left;
                    ret->num = -ret->num;
                }else if(typeL == FLOAT){
                    ret = Left;
                    ret->numf = -ret->numf;
                }else{
                    yyerror("Error: Unary minus only valid for numbers");
                    exit(1);
                }
            }
            break;
            case '|':{
                ret = eval(a->l);
                if (returnType(ret) == NUM){
                    if (ret->num<0){
                        ret->num = -ret->num;
                    }
                } else if (returnType(ret) == FLOAT){
                    if (ret->numf<0){
                        ret->numf = -ret->numf;
                    }
                }
                else {
                    yyerror("Error: operation only for numbers");
                    exit(1);
                }
            }
            break;
            case ASGN:{
                struct syml *cabIdd = (((struct symasgn *)a)->s);
                struct expl *cabExp = (((struct symasgn *)a)->l);
                while(cabIdd!=NULL){
                    if (cabExp!=NULL){
                        //cabIdd->s->value = copyData(eval(cabExp->a));
                        /*Lambda function*/
                        if(cabExp->a->nodetype == LAMBDA){
                            struct lambda *l = (struct lambda *)cabExp->a;
                            newfunc(cabIdd->s,l->symlist,l->expreturn);
                        }else{
                            cabIdd->s->value = copyData(eval(cabExp->a));
                        }
                        Left = cabIdd->s->value;
                        cabIdd = cabIdd->next;
                        cabExp = cabExp->next;
                    }
                    else{
                        cabIdd->s->value = copyData(Left);
                        cabIdd = cabIdd->next;
                    }
                }
                if (cabExp){
                    yyerror("Error: expected indentifier for the assign\n");
                    exit(1);
                }
                ret = NULL;
            }
            break;
            case REF:{
                ret = (((struct symref *)a)->s)->value;
                if (!ret) {
                    yyerror("Error: the variable is not defined\n");
                    exit(1);
                }
            }
            break;
            case ANDLOG:{
                ret = nvo_nodo(BOOL);
                ret->boolean = isTrue(eval(a->l)) && isTrue(eval(a->r));
            }
            break;
            case ORLOG:{
                ret = nvo_nodo(BOOL);
                ret->boolean = isTrue(eval(a->l)) || isTrue(eval(a->r));
            }
            break;
            case NOTLOG:{
                ret = nvo_nodo(BOOL);
                ret->boolean = !isTrue(eval(a->l));
            }
            break;
            case INOP:{
                Right = eval(a->r);
                Left = eval(a->l);
                typeR = returnType(Right);
                if(typeR == SET || typeR == LIST){
                    ret = INDATA(Right, Left);
                }
            }
            break;
            case CONTOP:{
                Right = eval(a->r);
                Left = eval(a->l);
                typeL = returnType(Left);
                typeR = returnType(Right);
                if((typeL == SET && typeR == SET) || (typeL == LIST && typeR == LIST)){
                    ret = INCLUDE(Right,Left);
                }else{
                    yyerror("Error: Unexpected type in contains operation \n");
                    exit(1);
                }
            }
            break;
            case SIZEOP:{
                Left = eval(a->l);
                typeL = returnType(Left);
                if(typeL == SET || typeL == LIST){
                    ret = SIZEDATA(Left);
                }
                else{
                    yyerror("Error: Unexpected type in size operation \n");
                    exit(1);
                }
            }
            break;
            case IFSTMT:{
                struct flow *fl = (struct flow *)a;
                if(isTrue(eval(fl->cond))){
                    eval(fl->tl);
                }else if(fl->fl){
                    eval(fl->fl);
                }
            }
            break;
            case WHSTMT:{
                struct flow *fl = (struct flow *)a;
                while (isTrue(eval(fl->cond)))
                {        
                    eval(fl->tl);
                };
            }
            break;
            case FORSTMT:{
                int c = 0;
                struct flow *fl = (struct flow *)a;
                struct symbol* x = ((struct symref*)fl->cond)->s;
                tData copy = copyData(eval(fl->tl));
                if(returnType(copy) != LIST && returnType(copy) != SET){
                    yyerror("Error: Expected type list or set in for statement\n");
                    exit(1);
                }
                else{
                    while (x->value = returnElem(copy, c)){
                        c++;
                        eval(fl->fl);
                    }
                    x->value = returnElem(copy,c-1);
                }
            }
            break;
            case LISTCOMP:{
                struct comprehension *comp = (struct comprehension *)a;
                struct symbol* x = ((struct symref*)comp->var)->s;
                Left = x->value;
                tData copy = copyData(eval(comp->iterable));
                int c = 0;
                if(returnType(copy) != LIST && returnType(copy) != SET){
                    yyerror("Error: Expected type list in list comprehension\n");
                    exit(1);
                }
                else{
                    ret = nvo_nodo(LIST);
                    while (x->value =returnElem(copy , c)){
                        if(comp->cond){
                            if(isTrue(eval(comp->cond))){
                                ADDOP(ret, eval(comp->op));
                            }
                        }
                        else{
                            ADDOP(ret, eval(comp->op));
                            }
                        c++;
                    }
                    x->value = Left;
                }
            }
            break;
            case SETCOMP:{
                struct comprehension *comp = (struct comprehension *)a;
                struct symbol* x = ((struct symref*)comp->var)->s;
                Left = x->value;
                tData copy = copyData(eval(comp->iterable));
                int c = 0;
                if(returnType(copy) != LIST && returnType(copy) != SET){
                    yyerror("Error: Expected type list in set comprehension\n");
                    exit(1);
                }
                else{
                    ret = nvo_nodo(SET);
                    while (x->value =returnElem(copy , c)){
                        if(comp->cond){
                            if(isTrue(eval(comp->cond))){
                                ADDOP(ret, eval(comp->op));
                            }
                        }
                        else{
                            ADDOP(ret, eval(comp->op));
                            }
                        c++;
                    }
                    Depurar(ret);
                    x->value = Left;
                }
            }
            break;
            case IFCOMPRENSHION:{
                ret = nvo_nodo(BOOL);
                ret->boolean = isTrue(eval(a->l));
            }
            break;
            case RANGEOP: {
                tData ini = NULL, fin = NULL, paso = NULL;
                struct expl *cabExp = (struct expl *)(((struct ast *)a)->l);
                int i = 0;
                int b = 0;
                while(cabExp && i<3 && !b){
                    switch (i) {
                        case 0: ini = eval(cabExp->a);
                            if(returnType(ini)!=NUM) b = 1;
                            break;
                        case 1: fin = eval(cabExp->a);
                            if(returnType(fin)!=NUM) b = 1;
                            break;
                        case 2: paso = eval(cabExp->a);
                            if(returnType(paso)!=NUM) b = 1;
                            break;
                    }
                    i++;
                    cabExp = cabExp->next;
                }
                if(b){
                    yyerror("Error: Unexpected type for the paramaters in range function");
                    exit(1);
                }
                if (cabExp){
                    yyerror("Error: too many arguments for the range function");
                    exit(1);
                }
                ret = RANGELIST2(ini,fin,paso);
            }
            break;
            case BLOCK:{
                if(a->l)
                    eval(a->l);
                if(a->r)
                    eval(a->r);
            }
            break;
            case PRINTSTMT:{
                if(a->l){
                    if(!mode){
                        printData(eval(a->l));
                        if(a->r)
                            printf("\n");
                    }
                    else{
                        printf("=> ");
                        printData(eval(a->l));
                        printf("\n");
                    }
                }
            }
            break;
            case FUNC:{
                ret = callfunc((struct fncall*)a);
            }
            break;
            case RETURNKEYW:{
                ret = eval(a->l);
                RETURNVAL = ret;
                RETURNSTATE = 1;
            }
            break;
            default: printf("Error al analizar ast");
        }
    return ret;
}