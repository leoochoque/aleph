#include "aleph.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void yyerror(char *s, ...);
int contador = 0;

tData RETURNVAL = NULL;
int RETURNSTATE = 0;

env *global_env = NULL;
env *current_env = NULL;

/* Tabla de símbolos */

struct symbol symtab[NHASH];

static unsigned symhash(char *sym){
    unsigned int hash = 0;
    unsigned c;
    while(c = *sym++) hash = hash*9 ^ c;
    return hash;
}

void init_env(){
    global_env = calloc(1, sizeof(env));
    global_env->prev = NULL;
    current_env = global_env;
}

struct symbol *define_symbol(char *sym){
    unsigned hash = symhash(sym) % NHASH;
    struct symbol *sp = current_env->table[hash];

    while(sp != NULL) {
        if(compara_cad(sp->name, sym) == 0) {
            printf("Error: Symbol '%s' is already defined in the current scope.\n",sym);
            exit(1);
        };
        sp = sp->next;
    }

    sp = calloc(1, sizeof(struct symbol));
    sp->name = devuelve_cad(sym);
    sp->next = current_env->table[hash]; //why is that porque no NULL
    current_env->table[hash] = sp;
    return sp;
}

struct symbol *lookup(char *sym){
    env *e = current_env;
    unsigned hash = symhash(sym) % NHASH;

    while(e != NULL){
        struct symbol *sp = e->table[hash];
        while(sp != NULL) {
            if(compara_cad(sp->name, sym) == 0) return sp;
            sp = sp->next;
        }
        e = e->prev;
    }

    return NULL;
}

struct ast *newasgn(int nodetype, struct syml *li, struct expl *le){
    struct symasgn *sym = malloc(sizeof(struct symasgn));
    sym->s = li;
    sym->l = le;
    sym->nodetype = nodetype;
    return (struct ast *)sym;
}

struct ast *newref(char *s){
    struct symref *sym = malloc(sizeof(struct symref));
    sym->nodetype = REF;
    sym->name = s;
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

struct syml *newsyml(char *s, struct syml *next){
    struct syml *l = malloc(sizeof(struct syml));
    l->name = s;
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

struct ast * newfunc(char *name, struct syml * symlist, struct ast *block){
    struct fncreate* f = malloc(sizeof(struct fncreate));
    f->nodetype = FUNCDEF;
    f->name = name;
    f->symlist = symlist;
    f->block = block;
    return (struct ast *)f;
};

struct ast *newcall(struct ast * func, struct expl *explist){
    struct fncall* fn = malloc(sizeof(struct fncall));
    fn->nodetype = FUNC;
    fn->func = func;
    fn->explist = explist;
    return (struct ast*)fn;
};

struct ast *newset(struct ast* sym, struct ast *pos, struct ast *exp){
    struct set* a = malloc(sizeof(struct set));
    a->nodetype = SETTER;
    a->name = sym;
    a->b = pos;
    a->c = exp;
    return (struct ast *)a;
};

struct ast *newlambda(struct syml *symlist, struct ast * exp){
    struct lambda* l = malloc(sizeof(struct lambda));
    l->nodetype = LAMBDA;
    l->symlist = symlist;
    l->expreturn = newast(RETURNKEYW, exp, NULL);
    return (struct ast *)l;
};

struct ast *newdotop(struct ast *obj, struct ast *atr, struct ast *val){
    struct dotop* d = malloc(sizeof(struct dotop));
    d->nodetype = DOT_OP;
    d->obj = obj;
    d->atr = atr;
    d->val = val;
    return (struct ast *)d;
};

void newstruct(char *name, struct syml * params, struct ast * body){
    tData s = nvo_nodo(STRCTDEF);
    s->structDef.name = devuelve_cad(name);
    s->structDef.params = params;
    s->structDef.body = body;
    struct symbol* sym = define_symbol(name);
    sym->value = s;
}

tData executeFunction(tData funcData, struct expl* explist){
    struct syml* params = NULL;
    struct ast *body = NULL;
    env *new_env = NULL;
    tData ret_val = NULL;

    if(returnType(funcData) == STRCTDEF){
        tData instance = nvo_nodo(STRCINS);
        instance->structIns.definition = funcData;

        new_env = calloc(1, sizeof(env));
        new_env->prev = global_env;
        instance->structIns.context = new_env;

        ret_val = instance;

        params = funcData->structDef.params;
        body = funcData->structDef.body;
    }else if(returnType(funcData) == FUN){
        new_env = calloc(1, sizeof(env));

        new_env->prev = funcData->function.context ? funcData->function.context : global_env;

        params = funcData->function.params;
        body = funcData->function.body;
    }else {
        printf("Error: Not a function nor struct.\n");
        exit(1);
    }

    env *saved_env = current_env;

    struct expl *args_node = explist;
    struct syml *params_node = params;

    current_env = new_env;

    while(args_node != NULL && params_node != NULL){
        current_env = saved_env;
        tData arg_value = eval(args_node->a);
        current_env = new_env;

        struct symbol *s = define_symbol(params_node->name);
        s->value = copyData(arg_value);

        params_node = params_node->next;
        args_node = args_node->next;
    }

    eval(body);

    current_env = saved_env;

    if(returnType(funcData) == STRCTDEF) {
        RETURNSTATE = 0;
        return ret_val;
    }

    ret_val = RETURNVAL;

    RETURNVAL = NULL;
    RETURNSTATE = 0;

    return ret_val;
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
                struct set *setter = (struct set *)a;
                struct symref *ref = (struct symref *)(setter->name);
                if(ref->nodetype != REF){
                    yyerror("Error: Assignment target must be a variable name.\n");
                    exit(1);
                }
                struct symbol *s = lookup(ref->name);
                if (!s) { printf("Error: Variable '%s' not found in set operation.\n", ref->name); exit(1); }
                Left = eval(setter->b);
                Right = eval(setter->c);
                if (returnType(s->value)==LIST){
                    if ((int)Left->num < (SIZEDATA(s->value))->num && (int)Left->num >=0)
                        s->value = REPLACE(s->value, Left, Right);
                    else{
                        yyerror("Error: Index out of range\n");
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
            case ASGN:
            case DECL:{
                struct syml *cabIdd = (((struct symasgn *)a)->s);
                struct expl *cabExp = (((struct symasgn *)a)->l);
                struct symbol *s = NULL;
                tData val = NULL;
                
                while(cabIdd!=NULL){

                    if(a->nodetype == DECL){
                        s = define_symbol(cabIdd->name);
                    }else if(a->nodetype == ASGN){
                        s = lookup(cabIdd->name);
                    }
                    
                    if(!s){
                        printf("Error: Variable '%s' not defined.\n", cabIdd->name);
                        exit(1);
                    }

                    if (cabExp!=NULL){
                        val = copyData(eval(cabExp->a)); 
                    } else {
                        val = copyData(Left); 
                    }

                    s->value = val;
            
                    Left = val; 
                    cabIdd = cabIdd->next;
                    if(cabExp) cabExp = cabExp->next;
                }
                ret = NULL;
            }
            break;
            case REF:{
                struct symref *ref = (struct symref *)a;
                struct symbol *active_sym = lookup(ref->name);

                if (active_sym == NULL) {
                    printf("Error: Variable '%s' is not defined in the current scope.\n", ref->name);
                    exit(1);
                }
                if (active_sym->value) {
                    ret = active_sym->value; 
                }
                else {
                    printf("Error: Variable '%s' has no value assigned (null).\n", ref->name);
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
            case IFTERN:{
                struct flow *fl = (struct flow *)a;
                if(isTrue(eval(fl->cond))){
                    ret = eval(fl->tl);
                }else if(fl->fl){
                    ret = eval(fl->fl);
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
                char *loopvar = ((struct symref*)fl->cond)->name;
                struct symbol* x = define_symbol(loopvar);
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
                char *compvar = ((struct symref*)comp->var)->name;
                struct symbol* x = define_symbol(compvar);
                tData copy = copyData(eval(comp->iterable));
                int c = 0;
                if(returnType(copy) != LIST && returnType(copy) != SET){
                    yyerror("Error: Expected type list in list comprehension\n");
                    exit(1);
                }
                else{
                    ret = nvo_nodo(LIST);
                    while (x->value = returnElem(copy , c)){
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
                }
            }
            break;
            case SETCOMP:{
                struct comprehension *comp = (struct comprehension *)a;
                char *compvar = ((struct symref*)comp->var)->name;
                struct symbol* x = define_symbol(compvar);
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
            case FUNCDEF:{
                struct fncreate *fnode = (struct fncreate*)a;
                struct symbol *s = define_symbol(fnode->name);
                tData funcData = nvo_nodo(FUN);
                funcData->function.params = fnode->symlist;
                funcData->function.body = fnode->block;
                funcData->function.context = current_env;
                s->value = funcData;
                ret = NULL;
            }
            break;
            case FUNC:{
                struct fncall *fnode = (struct fncall*)a;
                // 1. Evaluamos la expresión de la izquierda (el nombre, la lambda, o el array access)
                tData func = eval(fnode->func);
                // 2. Ejecutamos usando la función genérica
                if(func) {
                    ret = executeFunction(func, fnode->explist);
                } else {
                    yyerror("Error: Function evaluation returned NULL\n");
                    exit(1);
                }
            }
            break;
            case LAMBDA:{
                struct lambda *l = (struct lambda *)a;
                ret = nvo_nodo(FUN);
                ret->function.params = l->symlist;
                ret->function.body = l->expreturn;
                ret->function.context = current_env;
            }
            break;
            case DOT_OP: {
                struct dotop *dotop = (struct dotop *)a;
                tData obj = eval(dotop->obj); // Lado izquierdo (la instancia)
                char *name = ((struct symref*)dotop->atr)->name; // Lado derecho (el nombre)

                if(returnType(obj) == STRCINS) {
                    // Búsqueda Manual en el entorno del objeto
                    env *e = obj->structIns.context;
                    unsigned hash = symhash(name) % NHASH;
                    struct symbol *sp = e->table[hash];
                    
                    while(sp) {
                        if(compara_cad(sp->name, name) == 0){
                            if(dotop->val){
                                sp->value = eval(dotop->val);
                            }
                            return sp->value;
                        }
                        sp = sp->next;
                    }
                    // Si no está en la instancia, podrías buscar en la definición (métodos compartidos)
                    // Por ahora, error:
                    printf("Error: Member '%s' not found in structure.\n", name);
                    exit(1);
                }
                // Aquí puedes agregar lógica para .size, .length de listas nativas si quieres
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