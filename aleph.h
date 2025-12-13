#include "tpt/tData.h"

extern int mode;
/* Tabla de símbolos */

#define NHASH 101

struct symbol{
    char *name;
    tData value;
    struct symbol *next; // Para colisiones en la hash
};

typedef struct Env{
    struct symbol * table[NHASH];
    struct Env * prev;
}env;

extern env *global_env;
extern env *current_env;

struct symasgn{
    int nodetype;
    struct syml *s;
    struct expl *l;
};

struct symref{
    int nodetype;
    char *name;
};

struct flow{
    int nodetype;
    struct ast* cond;
    struct ast *tl;
    struct ast *fl;
};

struct comprehension{
    int nodetype;
    struct ast* op;
    struct ast* var;
    struct ast* iterable;
    struct ast* cond;
};

void init_env();
struct symbol *lookup(char *sym);        // Busca recursivamente hacia arriba
struct symbol *define_symbol(char *sym); // Crea variable en el entorno ACTUAL

struct ast *newref(char *s);
struct ast *newasgn(struct syml *li, struct expl *le);

//Declaracion de tipos de AST
#define TSET 1000
#define TELEM 1001
#define USET 1002
#define DSET 1003
#define ISET 1004
#define TLIST 1005 //{{1,2}, [1+2*3, 5/2], {1} union {"1"}} union {1,2}
#define ADDTO 1006
#define GETTER 1007
#define PLIST 1008
#define TNUM 1009
#define TBOOL 1010
#define ASGN 1011
#define REF 1012
#define ANDLOG 1013
#define ORLOG 1014
#define NOTLOG 1015
#define INOP 1016
#define CONTOP 1017
#define SIZEOP 1018
#define BLOCK 1019
#define IFSTMT 1020
#define WHSTMT 1021
#define FORSTMT 1022
#define PRINTSTMT 1023
#define TFLOAT 1024
#define FUNC 1025
#define RETURNKEYW 1026
#define RANGEOP 1027
#define SETTER 1028
#define TO_SET 1029
#define TO_LIST 1030
#define LISTCOMP 1031
#define SETCOMP 1032
#define IFCOMPRENSHION 1033
#define LAMBDA 1034
#define DOT_OP 1035
#define FUNCDEF 1036

struct ast{
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct fncall{
  int nodetype;
  struct ast *func;
  struct expl* explist;
};

struct fncreate{
  int nodetype;
  char *name;
  struct syml * symlist;
  struct ast *block;
};

struct set{
    int nodetype;
    char * name;
    struct ast *b;
    struct ast *c;
};

struct elem{
    int nodetype;
    char * elem;
};

struct num{
    int nodetype;
    int num;
};

struct boolean{
    int nodetype;
    bool boolean;
};
struct numfloat{
    int nodetype;
    double numf;
};

struct expl{
    struct ast *a;
    struct expl *next;
};

struct syml{
    char * name;
    struct syml *next;
};

struct cmp{
    int nodetype;
    int type;
};

struct lambda{
    int nodetype;
    struct syml *symlist;
    struct ast *expreturn;
};


struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newElem(char *);
struct ast *newNumber(int);
struct ast *newBoolean(bool);
struct ast *newNumberFloat(double num);
struct expl *newexpl(struct ast *a, struct expl *next);
struct syml *newsyml(char *s, struct syml *next);
struct ast * newflow(int nodetype, struct ast * cond, struct ast * tl, struct ast * fl);
struct ast * newcomprenshion(int nodetype, struct ast * op, struct ast * var, struct ast * iterable,struct ast * cond);
struct ast *newset(char *s, struct ast *pos, struct ast *expend);
struct ast *newcall(struct ast *func, struct expl *explist);
struct ast *newlambda(struct syml *symlist, struct ast * exp);
struct ast *newfunc(char *name, struct syml * symlist, struct ast *block);
tData callfunc(struct fncall* a);
tData eval(struct ast *);